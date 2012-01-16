/*
Copyright (c) 2009-2011 Roger Light <roger@atchoo.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of mosquitto nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __SYMBIAN32__
#include <netinet.in.h>
#endif

#ifdef __QNX__
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif
#include <net/netbyte.h>
#include <netinet/in.h>
#endif

#ifdef WITH_BROKER
#  include <mqtt3.h>
   extern uint64_t bytes_received;
   extern uint64_t bytes_sent;
   extern unsigned long msgs_received;
   extern unsigned long msgs_sent;
#else
#  include <read_handle.h>
#endif

#ifndef WIN32
#  define COMPAT_ECONNRESET ECONNRESET
#else
#  define COMPAT_ECONNRESET WSAECONNRESET
#endif

#include <memory_mosq.h>
#include <mqtt3_protocol.h>
#include <net_mosq.h>

void _mosquitto_net_init(void)
{
#ifdef WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

#ifdef WITH_SSL
	SSL_library_init();
	OpenSSL_add_all_algorithms();
#endif
}

void _mosquitto_net_cleanup(void)
{
#ifdef WIN32
	WSACleanup();
#endif
}

void _mosquitto_packet_cleanup(struct _mosquitto_packet *packet)
{
	if(!packet) return;

	/* Free data and reset values */
	packet->command = 0;
	packet->have_remaining = 0;
	packet->remaining_count = 0;
	packet->remaining_mult = 1;
	packet->remaining_length = 0;
	if(packet->payload) _mosquitto_free(packet->payload);
	packet->payload = NULL;
	packet->to_process = 0;
	packet->pos = 0;
}

int _mosquitto_packet_queue(struct mosquitto *mosq, struct _mosquitto_packet *packet)
{
	struct _mosquitto_packet *tail;

	assert(mosq);
	assert(packet);

	packet->pos = 0;
	packet->to_process = packet->packet_length;

	packet->next = NULL;
	if(mosq->out_packet){
		tail = mosq->out_packet;
		while(tail->next){
			tail = tail->next;
		}
		tail->next = packet;
	}else{
		mosq->out_packet = packet;
	}
#ifdef WITH_BROKER
	return _mosquitto_packet_write(mosq);
#else
	if(mosq->in_callback == false){
		return _mosquitto_packet_write(mosq);
	}else{
		return MOSQ_ERR_SUCCESS;
	}
#endif
}

/* Close a socket associated with a context and set it to -1.
 * Returns 1 on failure (context is NULL)
 * Returns 0 on success.
 */
int _mosquitto_socket_close(struct mosquitto *mosq)
{
	int rc = 0;

	assert(mosq);
	/* FIXME - need to shutdown SSL here. */
	if(mosq->sock != INVALID_SOCKET){
		rc = COMPAT_CLOSE(mosq->sock);
		mosq->sock = INVALID_SOCKET;
	}

	return rc;
}

/* Create a socket and connect it to 'ip' on port 'port'.
 * Returns -1 on failure (ip is NULL, socket creation/connection error)
 * Returns sock number on success.
 */
int _mosquitto_socket_connect(struct mosquitto *mosq, const char *host, uint16_t port)
{
	int sock = INVALID_SOCKET;
	int opt;
	struct addrinfo hints;
	struct addrinfo *ainfo, *rp;
	int s;
	char err[1024];
#ifdef WIN32
	uint32_t val = 1;
#endif
#ifdef WITH_SSL
	int ret;
#endif

	if(!mosq || !host || !port) return MOSQ_ERR_INVAL;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	s = getaddrinfo(host, NULL, &hints, &ainfo);
	if(s) return MOSQ_ERR_UNKNOWN;

	for(rp = ainfo; rp != NULL; rp = rp->ai_next){
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sock == INVALID_SOCKET) continue;
		
		if(rp->ai_family == PF_INET){
			((struct sockaddr_in *)rp->ai_addr)->sin_port = htons(port);
		}else if(rp->ai_family == PF_INET6){
			((struct sockaddr_in6 *)rp->ai_addr)->sin6_port = htons(port);
		}else{
			continue;
		}
		if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1){
			break;
		}

		COMPAT_CLOSE(sock);
	}
	if(!rp){
		strerror_r(errno, err, 1024);
		fprintf(stderr, "Error: %s\n", err);
		COMPAT_CLOSE(sock);
		return MOSQ_ERR_UNKNOWN;
	}
	freeaddrinfo(ainfo);

#ifdef WITH_SSL
	if(mosq->ssl){
		mosq->ssl->bio = BIO_new_socket(sock, BIO_NOCLOSE);
		if(!mosq->ssl->bio){
			COMPAT_CLOSE(sock);
			return MOSQ_ERR_SSL;
		}
		SSL_set_bio(mosq->ssl->ssl, mosq->ssl->bio, mosq->ssl->bio);

		ret = SSL_connect(mosq->ssl->ssl);
		if(ret != 1){
			COMPAT_CLOSE(sock);
			return MOSQ_ERR_SSL;
		}
	}
#endif

	/* Set non-blocking */
#ifndef WIN32
	opt = fcntl(sock, F_GETFL, 0);
	if(opt == -1 || fcntl(sock, F_SETFL, opt | O_NONBLOCK) == -1){
#ifdef WITH_SSL
		if(mosq->ssl){
			_mosquitto_free(mosq->ssl);
			mosq->ssl = NULL;
		}
#endif
		COMPAT_CLOSE(sock);
		return MOSQ_ERR_UNKNOWN;
	}
#else
	if(ioctlsocket(sock, FIONBIO, &val)){
#ifdef WITH_SSL
		if(mosq->ssl){
			_mosquitto_free(mosq->ssl);
			mosq->ssl = NULL;
		}
#endif
		COMPAT_CLOSE(sock);
		return MOSQ_ERR_UNKNOWN;
	}
#endif

	mosq->sock = sock;

	return MOSQ_ERR_SUCCESS;
}

int _mosquitto_read_byte(struct _mosquitto_packet *packet, uint8_t *byte)
{
	assert(packet);
	if(packet->pos+1 > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

	*byte = packet->payload[packet->pos];
	packet->pos++;

	return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_byte(struct _mosquitto_packet *packet, uint8_t byte)
{
	assert(packet);
	assert(packet->pos+1 <= packet->packet_length);

	packet->payload[packet->pos] = byte;
	packet->pos++;
}

int _mosquitto_read_bytes(struct _mosquitto_packet *packet, uint8_t *bytes, uint32_t count)
{
	assert(packet);
	if(packet->pos+count > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

	memcpy(bytes, &(packet->payload[packet->pos]), count);
	packet->pos += count;

	return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_bytes(struct _mosquitto_packet *packet, const uint8_t *bytes, uint32_t count)
{
	assert(packet);
	assert(packet->pos+count <= packet->packet_length);

	memcpy(&(packet->payload[packet->pos]), bytes, count);
	packet->pos += count;
}

int _mosquitto_read_string(struct _mosquitto_packet *packet, char **str)
{
	uint16_t len;
	int rc;

	assert(packet);
	rc = _mosquitto_read_uint16(packet, &len);
	if(rc) return rc;

	if(packet->pos+len > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

	*str = _mosquitto_calloc(len+1, sizeof(char));
	if(*str){
		memcpy(*str, &(packet->payload[packet->pos]), len);
		packet->pos += len;
	}else{
		return MOSQ_ERR_NOMEM;
	}

	return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_string(struct _mosquitto_packet *packet, const char *str, uint16_t length)
{
	assert(packet);
	_mosquitto_write_uint16(packet, length);
	_mosquitto_write_bytes(packet, (uint8_t *)str, length);
}

int _mosquitto_read_uint16(struct _mosquitto_packet *packet, uint16_t *word)
{
	uint8_t msb, lsb;

	assert(packet);
	if(packet->pos+2 > packet->remaining_length) return MOSQ_ERR_PROTOCOL;

	msb = packet->payload[packet->pos];
	packet->pos++;
	lsb = packet->payload[packet->pos];
	packet->pos++;

	*word = (msb<<8) + lsb;

	return MOSQ_ERR_SUCCESS;
}

void _mosquitto_write_uint16(struct _mosquitto_packet *packet, uint16_t word)
{
	_mosquitto_write_byte(packet, MOSQ_MSB(word));
	_mosquitto_write_byte(packet, MOSQ_LSB(word));
}

ssize_t _mosquitto_net_read(struct mosquitto *mosq, void *buf, size_t count)
{
#ifdef WITH_SSL
	int ret;
	int err;
#endif
	assert(mosq);
#ifdef WITH_SSL
	if(mosq->ssl){
		ret = SSL_read(mosq->ssl->ssl, buf, count);
		if(ret < 0){
			err = SSL_get_error(mosq->ssl->ssl, ret);
			if(err == SSL_ERROR_WANT_READ){
				ret = -1;
				mosq->ssl->want_read = true;
				errno = EAGAIN;
			}else if(err == SSL_ERROR_WANT_WRITE){
				ret = -1;
				mosq->ssl->want_write = true;
				errno = EAGAIN;
			}
		}
		return (ssize_t )ret;
	}else{
		/* Call normal read/recv */

#endif

#ifndef WIN32
	return read(mosq->sock, buf, count);
#else
	return recv(mosq->sock, buf, count, 0);
#endif

#ifdef WITH_SSL
	}
#endif
}

ssize_t _mosquitto_net_write(struct mosquitto *mosq, void *buf, size_t count)
{
#ifdef WITH_SSL
	int ret;
	int err;
#endif
	assert(mosq);

#ifdef WITH_SSL
	if(mosq->ssl){
		ret = SSL_write(mosq->ssl->ssl, buf, count);
		if(ret < 0){
			err = SSL_get_error(mosq->ssl->ssl, ret);
			if(err == SSL_ERROR_WANT_READ){
				ret = -1;
				mosq->ssl->want_read = true;
			}else if(err == SSL_ERROR_WANT_WRITE){
				ret = -1;
				mosq->ssl->want_write = true;
			}
		}
		return (ssize_t )ret;
	}else{
		/* Call normal write/send */
#endif

#ifndef WIN32
	return write(mosq->sock, buf, count);
#else
	return send(mosq->sock, buf, count, 0);
#endif

#ifdef WITH_SSL
	}
#endif
}

int _mosquitto_packet_write(struct mosquitto *mosq)
{
	ssize_t write_length;
	struct _mosquitto_packet *packet;

	if(!mosq) return MOSQ_ERR_INVAL;
	if(mosq->sock == INVALID_SOCKET) return MOSQ_ERR_NO_CONN;

	while(mosq->out_packet){
		packet = mosq->out_packet;

		while(packet->to_process > 0){
			write_length = _mosquitto_net_write(mosq, &(packet->payload[packet->pos]), packet->to_process);
			if(write_length > 0){
#ifdef WITH_BROKER
				bytes_sent += write_length;
#endif
				packet->to_process -= write_length;
				packet->pos += write_length;
			}else{
#ifndef WIN32
				if(errno == EAGAIN || errno == EWOULDBLOCK){
#else
				if(WSAGetLastError() == WSAEWOULDBLOCK){
#endif
					return MOSQ_ERR_SUCCESS;
				}else{
					switch(errno){
						case COMPAT_ECONNRESET:
							return MOSQ_ERR_CONN_LOST;
						default:
							return MOSQ_ERR_UNKNOWN;
					}
				}
			}
		}

#ifdef WITH_BROKER
		msgs_sent++;
#else
		if(((packet->command)&0xF6) == PUBLISH && mosq->on_publish){
			/* This is a QoS=0 message */
			mosq->in_callback = true;
			mosq->on_publish(mosq->obj, packet->mid);
			mosq->in_callback = false;
		}
#endif

		/* Free data and reset values */
		mosq->out_packet = packet->next;
		_mosquitto_packet_cleanup(packet);
		_mosquitto_free(packet);

		mosq->last_msg_out = time(NULL);
	}
	return MOSQ_ERR_SUCCESS;
}

#ifdef WITH_BROKER
int _mosquitto_packet_read(mosquitto_db *db, int context_index)
#else
int _mosquitto_packet_read(struct mosquitto *mosq)
#endif
{
	uint8_t byte;
	ssize_t read_length;
	int rc = 0;
#ifdef WITH_BROKER
	struct mosquitto *mosq;

	if(context_index < 0 || context_index >= db->context_count) return MOSQ_ERR_INVAL;
	mosq = db->contexts[context_index];
#endif

	if(!mosq) return MOSQ_ERR_INVAL;
	if(mosq->sock == INVALID_SOCKET) return MOSQ_ERR_NO_CONN;
	/* This gets called if pselect() indicates that there is network data
	 * available - ie. at least one byte.  What we do depends on what data we
	 * already have.
	 * If we've not got a command, attempt to read one and save it. This should
	 * always work because it's only a single byte.
	 * Then try to read the remaining length. This may fail because it is may
	 * be more than one byte - will need to save data pending next read if it
	 * does fail.
	 * Then try to read the remaining payload, where 'payload' here means the
	 * combined variable header and actual payload. This is the most likely to
	 * fail due to longer length, so save current data and current position.
	 * After all data is read, send to _mosquitto_handle_packet() to deal with.
	 * Finally, free the memory and reset everything to starting conditions.
	 */
	if(!mosq->in_packet.command){
		read_length = _mosquitto_net_read(mosq, &byte, 1);
		if(read_length == 1){
			mosq->in_packet.command = byte;
#ifdef WITH_BROKER
			bytes_received++;
			/* Clients must send CONNECT as their first command. */
			if(!(mosq->bridge) && mosq->state == mosq_cs_new && (byte&0xF0) != CONNECT) return MOSQ_ERR_PROTOCOL;
#endif
		}else{
			if(read_length == 0) return MOSQ_ERR_CONN_LOST; /* EOF */
#ifndef WIN32
			if(errno == EAGAIN || errno == EWOULDBLOCK){
#else
			if(WSAGetLastError() == WSAEWOULDBLOCK){
#endif
				return MOSQ_ERR_SUCCESS;
			}else{
				switch(errno){
					case COMPAT_ECONNRESET:
						return MOSQ_ERR_CONN_LOST;
					default:
						return MOSQ_ERR_UNKNOWN;
				}
			}
		}
	}
	if(!mosq->in_packet.have_remaining){
		/* Read remaining
		 * Algorithm for decoding taken from pseudo code at
		 * http://publib.boulder.ibm.com/infocenter/wmbhelp/v6r0m0/topic/com.ibm.etools.mft.doc/ac10870_.htm
		 */
		do{
			read_length = _mosquitto_net_read(mosq, &byte, 1);
			if(read_length == 1){
				mosq->in_packet.remaining_count++;
				/* Max 4 bytes length for remaining length as defined by protocol.
				 * Anything more likely means a broken/malicious client.
				 */
				if(mosq->in_packet.remaining_count > 4) return MOSQ_ERR_PROTOCOL;

#ifdef WITH_BROKER
				bytes_received++;
#endif
				mosq->in_packet.remaining_length += (byte & 127) * mosq->in_packet.remaining_mult;
				mosq->in_packet.remaining_mult *= 128;
			}else{
				if(read_length == 0) return MOSQ_ERR_CONN_LOST; /* EOF */
#ifndef WIN32
				if(errno == EAGAIN || errno == EWOULDBLOCK){
#else
				if(WSAGetLastError() == WSAEWOULDBLOCK){
#endif
					return MOSQ_ERR_SUCCESS;
				}else{
					switch(errno){
						case COMPAT_ECONNRESET:
							return MOSQ_ERR_CONN_LOST;
						default:
							return MOSQ_ERR_UNKNOWN;
					}
				}
			}
		}while((byte & 128) != 0);

		if(mosq->in_packet.remaining_length > 0){
			mosq->in_packet.payload = _mosquitto_malloc(mosq->in_packet.remaining_length*sizeof(uint8_t));
			if(!mosq->in_packet.payload) return MOSQ_ERR_NOMEM;
			mosq->in_packet.to_process = mosq->in_packet.remaining_length;
		}
		mosq->in_packet.have_remaining = 1;
	}
	while(mosq->in_packet.to_process>0){
		read_length = _mosquitto_net_read(mosq, &(mosq->in_packet.payload[mosq->in_packet.pos]), mosq->in_packet.to_process);
		if(read_length > 0){
#ifdef WITH_BROKER
			bytes_received += read_length;
#endif
			mosq->in_packet.to_process -= read_length;
			mosq->in_packet.pos += read_length;
		}else{
#ifndef WIN32
			if(errno == EAGAIN || errno == EWOULDBLOCK){
#else
			if(WSAGetLastError() == WSAEWOULDBLOCK){
#endif
				return MOSQ_ERR_SUCCESS;
			}else{
				switch(errno){
					case COMPAT_ECONNRESET:
						return MOSQ_ERR_CONN_LOST;
					default:
						return MOSQ_ERR_UNKNOWN;
				}
			}
		}
	}

	/* All data for this packet is read. */
	mosq->in_packet.pos = 0;
#ifdef WITH_BROKER
	msgs_received++;
	rc = mqtt3_packet_handle(db, context_index);
#else
	rc = _mosquitto_packet_handle(mosq);
#endif

	/* Free data and reset values */
	_mosquitto_packet_cleanup(&mosq->in_packet);

	mosq->last_msg_in = time(NULL);
	return rc;
}

