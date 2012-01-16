/*
Copyright (c) 2010, Roger Light <roger@atchoo.org>
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
#include <stdlib.h>
#include <string.h>

#include "mosquitto_internal.h"
#include "mosquitto.h"
#include "memory_mosq.h"
#include "messages_mosq.h"
#include "send_mosq.h"

void _mosquitto_message_cleanup(struct mosquitto_message_all **message)
{
	struct mosquitto_message_all *msg;

	if(!message || !*message) return;

	msg = *message;

	if(msg->msg.topic) _mosquitto_free(msg->msg.topic);
	if(msg->msg.payload) _mosquitto_free(msg->msg.payload);
	_mosquitto_free(msg);
}

void _mosquitto_message_cleanup_all(struct mosquitto *mosq)
{
	struct mosquitto_message_all *tmp;

	assert(mosq);

	while(mosq->messages){
		tmp = mosq->messages->next;
		_mosquitto_message_cleanup(&mosq->messages);
		mosq->messages = tmp;
	}
}

int mosquitto_message_copy(struct mosquitto_message *dst, const struct mosquitto_message *src)
{
	if(!dst || !src) return MOSQ_ERR_INVAL;

	dst->mid = src->mid;
	dst->topic = _mosquitto_strdup(src->topic);
	if(!dst->topic) return MOSQ_ERR_NOMEM;
	dst->qos = src->qos;
	dst->retain = src->retain;
	if(src->payloadlen){
		dst->payload = _mosquitto_malloc(src->payloadlen);
		if(!dst->payload){
			_mosquitto_free(dst->topic);
			return MOSQ_ERR_NOMEM;
		}
		memcpy(dst->payload, src->payload, src->payloadlen);
		dst->payloadlen = src->payloadlen;
	}else{
		dst->payloadlen = 0;
		dst->payload = NULL;
	}
	return MOSQ_ERR_SUCCESS;
}

int _mosquitto_message_delete(struct mosquitto *mosq, uint16_t mid, enum mosquitto_msg_direction dir)
{
	struct mosquitto_message_all *message;
	int rc;
	assert(mosq);

	rc = _mosquitto_message_remove(mosq, mid, dir, &message);
	if(rc == MOSQ_ERR_SUCCESS){
		_mosquitto_message_cleanup(&message);
	}
	return rc;
}

void mosquitto_message_free(struct mosquitto_message **message)
{
	struct mosquitto_message *msg;

	if(!message || !*message) return;

	msg = *message;

	if(msg->topic) _mosquitto_free(msg->topic);
	if(msg->payload) _mosquitto_free(msg->payload);
	_mosquitto_free(msg);
}

void _mosquitto_message_queue(struct mosquitto *mosq, struct mosquitto_message_all *message)
{
	struct mosquitto_message_all *tail;

	assert(mosq);
	assert(message);

	message->next = NULL;
	if(mosq->messages){
		tail = mosq->messages;
		while(tail->next){
			tail = tail->next;
		}
		tail->next = message;
	}else{
		mosq->messages = message;
	}
}

int _mosquitto_message_remove(struct mosquitto *mosq, uint16_t mid, enum mosquitto_msg_direction dir, struct mosquitto_message_all **message)
{
	struct mosquitto_message_all *cur, *prev = NULL;
	assert(mosq);
	assert(message);

	cur = mosq->messages;
	while(cur){
		if(cur->msg.mid == mid && cur->direction == dir){
			if(prev){
				prev->next = cur->next;
			}else{
				mosq->messages = cur->next;
			}
			*message = cur;
			return MOSQ_ERR_SUCCESS;
		}
		prev = cur;
		cur = cur->next;
	}
	return MOSQ_ERR_NOT_FOUND;
}

void _mosquitto_message_retry_check(struct mosquitto *mosq)
{
	struct mosquitto_message_all *message;
	time_t now = time(NULL);
	assert(mosq);

	message = mosq->messages;
	while(message){
		if(message->timestamp + mosq->message_retry < now){
			switch(message->state){
				case mosq_ms_wait_puback:
				case mosq_ms_wait_pubrec:
					message->timestamp = now;
					message->dup = true;
					_mosquitto_send_publish(mosq, message->msg.mid, message->msg.topic, message->msg.payloadlen, message->msg.payload, message->msg.qos, message->msg.retain, message->dup);
					break;
				case mosq_ms_wait_pubrel:
					message->timestamp = now;
					_mosquitto_send_pubrec(mosq, message->msg.mid);
					break;
				case mosq_ms_wait_pubcomp:
					message->timestamp = now;
					_mosquitto_send_pubrel(mosq, message->msg.mid, true);
					break;
				default:
					break;
			}
		}
		message = message->next;
	}
}

void mosquitto_message_retry_set(struct mosquitto *mosq, unsigned int message_retry)
{
	assert(mosq);
	if(mosq) mosq->message_retry = message_retry;
}

int _mosquitto_message_update(struct mosquitto *mosq, uint16_t mid, enum mosquitto_msg_direction dir, enum mosquitto_msg_state state)
{
	struct mosquitto_message_all *message;
	assert(mosq);

	message = mosq->messages;
	while(message){
		if(message->msg.mid == mid && message->direction == dir){
			message->state = state;
			message->timestamp = time(NULL);
			return MOSQ_ERR_SUCCESS;
		}
		message = message->next;
	}
	return MOSQ_ERR_NOT_FOUND;
}

