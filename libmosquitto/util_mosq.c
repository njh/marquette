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
#include <string.h>
#include <time.h>

#include <mosquitto.h>
#include <memory_mosq.h>
#include <net_mosq.h>
#include <send_mosq.h>
#include <util_mosq.h>

#ifdef WITH_BROKER
#include <mqtt3.h>
#endif

int _mosquitto_packet_alloc(struct _mosquitto_packet *packet)
{
	uint8_t remaining_bytes[5], byte;
	uint32_t remaining_length;
	int i;

	assert(packet);

	remaining_length = packet->remaining_length;
	packet->payload = NULL;
	packet->remaining_count = 0;
	do{
		byte = remaining_length % 128;
		remaining_length = remaining_length / 128;
		/* If there are more digits to encode, set the top bit of this digit */
		if(remaining_length > 0){
			byte = byte | 0x80;
		}
		remaining_bytes[packet->remaining_count] = byte;
		packet->remaining_count++;
	}while(remaining_length > 0 && packet->remaining_count < 5);
	if(packet->remaining_count == 5) return MOSQ_ERR_PAYLOAD_SIZE;
	packet->packet_length = packet->remaining_length + 1 + packet->remaining_count;
	packet->payload = _mosquitto_malloc(sizeof(uint8_t)*packet->packet_length);
	if(!packet->payload) return MOSQ_ERR_NOMEM;

	packet->payload[0] = packet->command;
	for(i=0; i<packet->remaining_count; i++){
		packet->payload[i+1] = remaining_bytes[i];
	}
	packet->pos = 1 + packet->remaining_count;

	return MOSQ_ERR_SUCCESS;
}

void _mosquitto_check_keepalive(struct mosquitto *mosq)
{
	assert(mosq);
	if(mosq->sock != INVALID_SOCKET && time(NULL) - mosq->last_msg_out >= mosq->keepalive){
		if(mosq->state == mosq_cs_connected){
			_mosquitto_send_pingreq(mosq);
		}else{
#ifdef WITH_BROKER
			if(mosq->listener){
				mosq->listener->client_count--;
				assert(mosq->listener->client_count >= 0);
			}
			mosq->listener = NULL;
#endif
			_mosquitto_socket_close(mosq);
		}
	}
}

/* Convert ////some////over/slashed///topic/etc/etc//
 * into some/over/slashed/topic/etc/etc
 */
int _mosquitto_fix_sub_topic(char **subtopic)
{
	char *fixed = NULL;
	char *token;
	char *saveptr = NULL;

	assert(subtopic);
	assert(*subtopic);

	/* size of fixed here is +1 for the terminating 0 and +1 for the spurious /
	 * that gets appended. */
	fixed = _mosquitto_calloc(strlen(*subtopic)+2, 1);
	if(!fixed) return MOSQ_ERR_NOMEM;

	if((*subtopic)[0] == '/'){
		fixed[0] = '/';
	}
	token = strtok_r(*subtopic, "/", &saveptr);
	while(token){
		strcat(fixed, token);
		strcat(fixed, "/");
		token = strtok_r(NULL, "/", &saveptr);
	}

	fixed[strlen(fixed)-1] = '\0';
	_mosquitto_free(*subtopic);
	*subtopic = fixed;
	return MOSQ_ERR_SUCCESS;
}

uint16_t _mosquitto_mid_generate(struct mosquitto *mosq)
{
	assert(mosq);

	mosq->last_mid++;
	if(mosq->last_mid == 0) mosq->last_mid++;
	
	return mosq->last_mid;
}

/* Search for + or # in a string. Return true if found. */
bool _mosquitto_wildcard_check(const char *str)
{
	while(str && str[0]){
		if(str[0] == '+' || str[0] == '#'){
			return true;
		}
		str = &str[1];
	}
	return false;
}
