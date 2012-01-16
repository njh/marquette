/*
Copyright (c) 2010,2011 Roger Light <roger@atchoo.org>
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

#ifndef _MOSQUITTO_INTERNAL_H_
#define _MOSQUITTO_INTERNAL_H_

#include "mosquitto_config.h"

#ifdef WITH_SSL
#include <openssl/ssl.h>
#endif
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
#include <winsock2.h>
#endif

#include "mosquitto.h"
#ifdef WITH_BROKER
struct _mosquitto_client_msg;
#endif

enum mosquitto_msg_direction {
	mosq_md_in = 0,
	mosq_md_out = 1
};

enum mosquitto_msg_state {
	mosq_ms_invalid = 0,
	mosq_ms_wait_puback = 1,
	mosq_ms_wait_pubrec = 2,
	mosq_ms_wait_pubrel = 3,
	mosq_ms_wait_pubcomp = 4
};

enum mosquitto_client_state {
	mosq_cs_new = 0,
	mosq_cs_connected = 1,
	mosq_cs_disconnecting = 2
};

struct _mosquitto_packet{
	uint8_t command;
	uint8_t have_remaining;
	uint8_t remaining_count;
	uint16_t mid;
	uint32_t remaining_mult;
	uint32_t remaining_length;
	uint32_t packet_length;
	uint32_t to_process;
	uint32_t pos;
	uint8_t *payload;
	struct _mosquitto_packet *next;
};

struct mosquitto_message_all{
	struct mosquitto_message_all *next;
	time_t timestamp;
	enum mosquitto_msg_direction direction;
	enum mosquitto_msg_state state;
	bool dup;
	struct mosquitto_message msg;
};

#ifdef WITH_SSL
struct _mosquitto_ssl{
	SSL_CTX *ssl_ctx;
	SSL *ssl;
	BIO *bio;
	bool want_read;
	bool want_write;
};
#endif

struct mosquitto {
#ifndef WIN32
	int sock;
#else
	SOCKET sock;
#endif
	char *address;
	char *id;
	char *username;
	char *password;
	uint16_t keepalive;
	bool clean_session;
	enum mosquitto_client_state state;
	time_t last_msg_in;
	time_t last_msg_out;
	uint16_t last_mid;
	struct _mosquitto_packet in_packet;
	struct _mosquitto_packet *out_packet;
	struct mosquitto_message *will;
#ifdef WITH_SSL
	struct _mosquitto_ssl *ssl;
#endif
#ifdef WITH_BROKER
	struct _mqtt3_bridge *bridge;
	struct _mosquitto_client_msg *msgs;
	struct _mosquitto_acl_user *acl_list;
	struct _mqtt3_listener *listener;
#else
	void *obj;
	bool in_callback;
	unsigned int message_retry;
	time_t last_retry_check;
	struct mosquitto_message_all *messages;
	int log_priorities;
	int log_destinations;
	void (*on_connect)(void *obj, int rc);
	void (*on_disconnect)(void *obj);
	void (*on_publish)(void *obj, uint16_t mid);
	void (*on_message)(void *obj, const struct mosquitto_message *message);
	void (*on_subscribe)(void *obj, uint16_t mid, int qos_count, const uint8_t *granted_qos);
	void (*on_unsubscribe)(void *obj, uint16_t mid);
	//void (*on_error)();
	char *host;
	int port;
#endif
};

#endif
