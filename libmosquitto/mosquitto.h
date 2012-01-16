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

#ifndef _MOSQUITTO_H_
#define _MOSQUITTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#	ifdef libmosquitto_EXPORTS
#		define libmosq_EXPORT  __declspec(dllexport)
#	else
#		define libmosq_EXPORT  __declspec(dllimport)
#	endif
#else
#	define libmosq_EXPORT
#endif

#ifdef WIN32
#	if _MSC_VER < 1600
		typedef unsigned char uint8_t;
		typedef unsigned short uint16_t;
		typedef unsigned int uint32_t;
		typedef unsigned long long uint64_t;
#	else
#		include <stdint.h>
#	endif
#	ifndef __cplusplus
#		define bool char
#		define true 1
#		define false 0
#	endif
#else
#	include <stdint.h>
#	include <stdbool.h>
#endif

#define LIBMOSQUITTO_MAJOR 0
#define LIBMOSQUITTO_MINOR 14
#define LIBMOSQUITTO_REVISION 4
#define LIBMOSQUITTO_VERSION_NUMBER (LIBMOSQUITTO_MAJOR*1000000+LIBMOSQUITTO_MINOR*1000+LIBMOSQUITTO_REVISION)

/* Log destinations */
#define MOSQ_LOG_NONE 0x00
#define MOSQ_LOG_STDOUT 0x04
#define MOSQ_LOG_STDERR 0x08

/* Log types */
#define MOSQ_LOG_INFO 0x01
#define MOSQ_LOG_NOTICE 0x02
#define MOSQ_LOG_WARNING 0x04
#define MOSQ_LOG_ERR 0x08
#define MOSQ_LOG_DEBUG 0x10
#define MOSQ_LOG_ALL 0xFF

/* Error values */
#define MOSQ_ERR_SUCCESS 0
#define MOSQ_ERR_NOMEM 1
#define MOSQ_ERR_PROTOCOL 2
#define MOSQ_ERR_INVAL 3
#define MOSQ_ERR_NO_CONN 4
#define MOSQ_ERR_CONN_REFUSED 5
#define MOSQ_ERR_NOT_FOUND 6
#define MOSQ_ERR_CONN_LOST 7
#define MOSQ_ERR_SSL 8
#define MOSQ_ERR_PAYLOAD_SIZE 9
#define MOSQ_ERR_NOT_SUPPORTED 10
#define MOSQ_ERR_AUTH 11
#define MOSQ_ERR_ACL_DENIED 12
#define MOSQ_ERR_UNKNOWN 13

struct mosquitto_message{
	uint16_t mid;
	char *topic;
	uint8_t *payload;
	uint32_t payloadlen;
	int qos;
	bool retain;
};

struct mosquitto;

/***************************************************
 * Important note
 * 
 * The following functions that deal with network operations will return
 * MOSQ_ERR_SUCCESS on success, but this does not mean that the operation has
 * taken place. Rather, the appropriate messages will have been queued and will
 * be completed when calling mosquitto_loop(). To be sure of the event having
 * taken place, use the callbacks. This is especially important when
 * disconnecting a client that has a will. If the broker does not receive the
 * DISCONNECT command, it will assume that the client has disconnected
 * unexpectedly and send the will.
 *
 * mosquitto_connect()
 * mosquitto_disconnect()
 * mosquitto_subscribe()
 * mosquitto_unsubscribe()
 * mosquitto_publish()
 ***************************************************/

/*
 * Function: mosquitto_lib_version
 *
 * Can be used to obtain version information for the phasecapture library.
 * This allows the application to compare the library version against the
 * version it was compiled against by using the LIBMOSQUITTO_MAJOR,
 * LIBMOSQUITTO_MINOR and LIBMOSQUITTO_REVISION defines.
 *
 * Parameters:
 *  major -    an integer pointer. If not NULL, the major version of the
 *             library will be returned in this variable.
 *  minor -    an integer pointer. If not NULL, the minor version of the
 *             library will be returned in this variable.
 *  revision - an integer pointer. If not NULL, the revision of the library will
 *             be returned in this variable.
 *
 * See Also:
 * 	<mosquitto_lib_cleanup>, <mosquitto_lib_init>
 */
libmosq_EXPORT void mosquitto_lib_version(int *major, int *minor, int *revision);

/*
 * Function: mosquitto_lib_init
 *
 * Must be called before any other mosquitto functions.
 *
 * Returns:
 * 	MOSQ_ERR_SUCCESS - always
 *
 * See Also:
 * 	<mosquitto_lib_cleanup>, <mosquitto_lib_version>
 */
libmosq_EXPORT int mosquitto_lib_init(void);

/*
 * Function: mosquitto_lib_cleanup
 *
 * Call to free resources associated with the library.
 *
 * Returns:
 * 	MOSQ_ERR_SUCCESS - always
 *
 * See Also:
 * 	<mosquitto_lib_init>, <mosquitto_lib_version>
 */
libmosq_EXPORT int mosquitto_lib_cleanup(void);

/*
 * Function: mosquitto_new
 *
 * Create a new mosquitto client instance.
 *
 * Parameters:
 * 	id -  String to use as the client id. Must not be NULL or zero length.
 * 	obj - A user pointer that will be passed as an argument to any callbacks
 *        that are specified.
 *
 * Returns:
 * 	Pointer to a struct mosquitto on success.
 * 	NULL on failure.
 *
 * See Also:
 * 	<mosquitto_destroy>
 */
libmosq_EXPORT struct mosquitto *mosquitto_new(const char *id, void *obj);

/* 
 * Function: mosquitto_destroy
 *
 * Use to free memory associated with a mosquitto client instance.
 *
 * Parameters:
 * 	mosq - a struct mosquitto pointer to free.
 *
 * See Also:
 * 	<mosquitto_new>
 */
libmosq_EXPORT void mosquitto_destroy(struct mosquitto *mosq);

/*
 * Function: mosquitto_log_init
 *
 * Configure logging options for a client instance. May be called at any point.
 *
 * Log priorities controls which types of messages are output. OR together
 * values from:
 *
 *	* MOSQ_LOG_INFO
 *	* MOSQ_LOG_NOTICE
 *	* MOSQ_LOG_WARNING
 *	* MOSQ_LOG_ERR
 *	* MOSQ_LOG_DEBUG
 *	* MOSQ_LOG_ALL
 *
 * Log destinations controls where the log messages are sent. OR together
 * values from:
 *
 *	* MOSQ_LOG_NONE
 *	* MOSQ_LOG_STDOUT
 *	* MOSQ_LOG_STDERR
 *
 * Parameters:
 *	mosq -         a valid mosquitto instance.
 *	priorities -   an integer bit mask of the log types to output.
 *	destinations - an integer bit mask of log destinations.
 *
 * Returns:
 * 	MOSQ_ERR_SUCCESS - always
 */
libmosq_EXPORT int mosquitto_log_init(struct mosquitto *mosq, int priorities, int destinations);

/* 
 * Function: mosquitto_will_set
 *
 * Configure will information for a mosquitto instance. By default, clients do
 * not have a will.  This must be called before calling <mosquitto_connect>.
 *
 * Parameters:
 * 	mosq -       a valid mosquitto instance.
 * 	will -       set to true to enable a will, false to disable. If set to true,
 *               at least "topic" but also be valid.
 * 	topic -      the topic on which to publish the will.
 * 	payloadlen - the size of the payload (bytes). Valid values are between 0 and
 *               268,435,455.
 * 	payload -    pointer to the data to send. If payloadlen > 0 this must be a
 *               valid memory location.
 * 	qos -        integer value 0, 1 or 2 indicating the Quality of Service to be
 *               used for the will.
 * 	retain -     set to true to make the will a retained message.
 *
 * Returns:
 * 	MOSQ_ERR_SUCCESS -      on success.
 * 	MOSQ_ERR_INVAL -        if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -        if an out of memory condition occurred.
 * 	MOSQ_ERR_PAYLOAD_SIZE - if payloadlen is too large.
 */
libmosq_EXPORT int mosquitto_will_set(struct mosquitto *mosq, bool will, const char *topic, uint32_t payloadlen, const uint8_t *payload, int qos, bool retain);

/*
 * Function: mosquitto_username_pw_set
 *
 * Configure username and password for a mosquitton instance. This is only
 * supported by brokers that implement the MQTT spec v3.1. By default, no
 * username or password will be sent.
 * If username is NULL, the password argument is ignored.
 * This must be called before calling mosquitto_connect().
 *
 * This is must be called before calling <mosquitto_connect>.
 *
 * Parameters:
 * 	mosq -     a valid mosquitto instance.
 * 	username - the username to send as a string, or NULL to disable
 *             authentication.
 * 	password - the password to send as a string. Set to NULL when username is
 * 	           valid in order to send just a username.
 *
 * Returns:
 * 	MOSQ_ERR_SUCCESS - on success.
 * 	MOSQ_ERR_INVAL -   if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -   if an out of memory condition occurred.
 */
libmosq_EXPORT int mosquitto_username_pw_set(struct mosquitto *mosq, const char *username, const char *password);

/*
 * Function: mosquitto_connect
 *
 * Connect to an MQTT broker.
 *
 * Parameters:
 * 	mosq -          a valid mosquitto instance.
 * 	host -          the hostname or ip address of the broker to connect to.
 * 	port -          the network port to connect to. Usually 1883.
 * 	keepalive -     the number of seconds after which the broker should send a
 * 	                PING message to the client if no other messages have been
 * 	                exchanged in that time.
 * 	clean_session - set to true to instruct the broker to clean all messages
 *                  and subscriptions on disconnect, false to instruct it to
 *                  keep them. See the man page mqtt(7) for more details.
 *
 * Returns:
 * 	MOSQ_ERR_SUCCESS - on success.
 * 	MOSQ_ERR_INVAL -   if the input parameters were invalid.
 *
 * See Also:
 * 	<mosquitto_reconnect>, <mosquitto_disconnect>
 */
libmosq_EXPORT int mosquitto_connect(struct mosquitto *mosq, const char *host, int port, int keepalive, bool clean_session);

/*
 * Function: mosquitto_reconnect
 *
 * Reconnect to a broker.
 *
 * This function provides an easy way of reconnecting to a broker after a
 * connection has been lost. It uses the values provided in the
 * <mosquitto_connect> call and so must not be called before
 * <mosquitto_connect>.
 *
 * See Also:
 * 	<mosquitto_connect>, <mosquitto_disconnect>
 */
libmosq_EXPORT int mosquitto_reconnect(struct mosquitto *mosq);

/*
 * Function: mosquitto_disconnect
 *
 * Disconnect from the broker.
 *
 * Parameters:
 *	mosq - a valid mosquitto instance.
 *
 * Returns:
 *	MOSQ_ERR_SUCCESS - on success.
 * 	MOSQ_ERR_INVAL -   if the input parameters were invalid.
 * 	MOSQ_ERR_NO_CONN -  if the client isn't connected to a broker.
 */
libmosq_EXPORT int mosquitto_disconnect(struct mosquitto *mosq);

/* 
 * Function: mosquitto_publish
 *
 * Publish a message on a given topic.
 * 
 * Parameters:
 * 	mosq -       a valid mosquitto instance.
 * 	mid -        pointer to a uint16_t. If not NULL, the function will set this
 *               to the message id of this particular message. This can be then
 *               used with the publish callback to determine when the message
 *               has been sent.
 *               Note that although the MQTT protocol doesn't use message ids
 *               for messages with QoS=0, libmosquitto assigns them message ids
 *               so they can be tracked with this parameter.
 * 	topic -      the topic to publish the message on. Must not contain the
 *               wildcard characters + or #
 * 	payloadlen - the size of the payload (bytes). Valid values are between 0 and
 *               268,435,455.
 * 	payload -    pointer to the data to send. If payloadlen > 0 this must be a
 *               valid memory location.
 * 	qos -        integer value 0, 1 or 2 indicating the Quality of Service to be
 *               used for the message.
 * 	retain -     set to true to make the message retained.
 *
 * Returns:
 * 	MOSQ_ERR_SUCCESS -      on success.
 * 	MOSQ_ERR_INVAL -        if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -        if an out of memory condition occurred.
 * 	MOSQ_ERR_NO_CONN -      if the client isn't connected to a broker.
 *	MOSQ_ERR_PROTOCOL -     if there is a protocol error communicating with the
 *                          broker.
 * 	MOSQ_ERR_PAYLOAD_SIZE - if payloadlen is too large.
 */
libmosq_EXPORT int mosquitto_publish(struct mosquitto *mosq, uint16_t *mid, const char *topic, uint32_t payloadlen, const uint8_t *payload, int qos, bool retain);

/*
 * Function: mosquitto_subscribe
 *
 * Subscribe to a topic.
 *
 * Parameters:
 *	mosq - a valid mosquitto instance.
 *	mid -  a pointer to a uint16_t. If not NULL, the function will set this to
 *	       the message id of this particular message. This can be then used
 *	       with the subscribe callback to determine when the message has been
 *	       sent.
 *	sub -  the subscription pattern.
 *	qos -  the requested Quality of Service for this subscription.
 *
 * Returns:
 *	MOSQ_ERR_SUCCESS - on success.
 * 	MOSQ_ERR_INVAL -   if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -   if an out of memory condition occurred.
 * 	MOSQ_ERR_NO_CONN - if the client isn't connected to a broker.
 */
libmosq_EXPORT int mosquitto_subscribe(struct mosquitto *mosq, uint16_t *mid, const char *sub, int qos);

/*
 * Function: mosquitto_unsubscribe
 *
 * Unsubscribe from a topic.
 *
 * Parameters:
 *	mosq - a valid mosquitto instance.
 *	mid -  a pointer to a uint16_t. If not NULL, the function will set this to
 *	       the message id of this particular message. This can be then used
 *	       with the unsubscribe callback to determine when the message has been
 *	       sent.
 *	sub -  the unsubscription pattern.
 *
 * Returns:
 *	MOSQ_ERR_SUCCESS - on success.
 * 	MOSQ_ERR_INVAL -   if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -   if an out of memory condition occurred.
 * 	MOSQ_ERR_NO_CONN - if the client isn't connected to a broker.
 */
libmosq_EXPORT int mosquitto_unsubscribe(struct mosquitto *mosq, uint16_t *mid, const char *sub);

/*
 * Function: mosquitto_message_copy
 *
 * Copy the contents of a mosquitto message to another message.
 * Useful for preserving a message received in the on_message() callback.
 *
 * Parameters:
 *	dst - a pointer to a valid mosquitto_message struct to copy to.
 *	src - a pointer to a valid mosquitto_message struct to copy from.
 *
 * Returns:
 *	MOSQ_ERR_SUCCESS - on success.
 * 	MOSQ_ERR_INVAL -   if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -   if an out of memory condition occurred.
 *
 * See Also:
 * 	<mosquitto_message_free>
 */
libmosq_EXPORT int mosquitto_message_copy(struct mosquitto_message *dst, const struct mosquitto_message *src);

/*
 * Function: mosquitto_message_free
 * 
 * Completely free a mosquitto_message struct.
 *
 * Parameters:
 *	message - pointer to a mosquitto_message pointer to free.
 *
 * See Also:
 * 	<mosquitto_message_copy>
 */
libmosq_EXPORT void mosquitto_message_free(struct mosquitto_message **message);

/*
 * Function: mosquitto_loop
 *
 * The main network loop for the client. You must call this frequently in order
 * to keep communications between the client and broker working.
 *
 * This calls select() to monitor the client network socket. If you want to
 * integrate mosquitto client operation with your own select() call, use
 * <mosquitto_socket>, <mosquitto_loop_read>, <mosquitto_loop_write> and
 * <mosquitto_loop_misc>.
 *
 * Parameters:
 *	mosq -    a valid mosquitto instance.
 *	timeout - Maximum number of milliseconds to wait for network activity in
 *            the select() call before timing out. Set to 0 for instant return.
 *            Set negative to use the default of 1000ms.
 * 
 * Returns:
 *	MOSQ_ERR_SUCCESS -   on success.
 * 	MOSQ_ERR_INVAL -     if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -     if an out of memory condition occurred.
 * 	MOSQ_ERR_NO_CONN -   if the client isn't connected to a broker.
 *  MOSQ_ERR_CONN_LOST - if the connection to the broker was lost.
 *	MOSQ_ERR_PROTOCOL -  if there is a protocol error communicating with the
 *                       broker.
 */
libmosq_EXPORT int mosquitto_loop(struct mosquitto *mosq, int timeout);

/*
 * Function: mosquitto_socket
 *
 * Return the socket handle for a mosquitto instance. Useful if you want to
 * include a mosquitto client in your own select() calls.
 *
 * Parameters:
 *	mosq - a valid mosquitto instance.
 *
 * Returns:
 *	The socket for the mosquitto client or -1 on failure.
 */
libmosq_EXPORT int mosquitto_socket(struct mosquitto *mosq);

/*
 * Function: mosquitto_loop_read
 *
 * Carry out network read operations.
 * This should only be used if you are not using mosquitto_loop() and are
 * monitoring the client network socket for activity yourself.
 *
 * Parameters:
 *	mosq - a valid mosquitto instance.
 *
 * Returns:
 *	MOSQ_ERR_SUCCESS -   on success.
 * 	MOSQ_ERR_INVAL -     if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -     if an out of memory condition occurred.
 * 	MOSQ_ERR_NO_CONN -   if the client isn't connected to a broker.
 *  MOSQ_ERR_CONN_LOST - if the connection to the broker was lost.
 *	MOSQ_ERR_PROTOCOL -  if there is a protocol error communicating with the
 *                       broker.
 *
 * See Also:
 *	<mosquitto_socket>, <mosquitto_loop_write>, <mosquitto_loop_misc>
 */
libmosq_EXPORT int mosquitto_loop_read(struct mosquitto *mosq);

/*
 * Function: mosquitto_loop_write
 *
 * Carry out network write operations.
 * This should only be used if you are not using mosquitto_loop() and are
 * monitoring the client network socket for activity yourself.
 *
 * Parameters:
 *	mosq - a valid mosquitto instance.
 *
 * Returns:
 *	MOSQ_ERR_SUCCESS -   on success.
 * 	MOSQ_ERR_INVAL -     if the input parameters were invalid.
 * 	MOSQ_ERR_NOMEM -     if an out of memory condition occurred.
 * 	MOSQ_ERR_NO_CONN -   if the client isn't connected to a broker.
 *  MOSQ_ERR_CONN_LOST - if the connection to the broker was lost.
 *	MOSQ_ERR_PROTOCOL -  if there is a protocol error communicating with the
 *                       broker.
 *
 * See Also:
 *	<mosquitto_socket>, <mosquitto_loop_read>, <mosquitto_loop_misc>
 */
libmosq_EXPORT int mosquitto_loop_write(struct mosquitto *mosq);

/*
 * Function: mosquitto_loop_misc
 *
 * Carry out miscellaneous operations required as part of the network loop.
 * This should only be used if you are not using mosquitto_loop() and are
 * monitoring the client network socket for activity yourself.
 *
 * This function deals with handling PINGs and checking whether messages need
 * to be retried, so should be called fairly frequently.
 *
 * Parameters:
 *	mosq - a valid mosquitto instance.
 *
 * Returns:
 *	MOSQ_ERR_SUCCESS -   on success.
 * 	MOSQ_ERR_INVAL -     if the input parameters were invalid.
 *
 * See Also:
 *	<mosquitto_socket>, <mosquitto_loop_read>, <mosquitto_loop_write>
 */
libmosq_EXPORT int mosquitto_loop_misc(struct mosquitto *mosq);

/* 
 * Function: mosquitto_connect_callback_set
 *
 * Set the connect callback. This is called when the broker sends a CONNACK
 * message in response to a connection.
 *
 * Parameters:
 *  mosq -       a valid mosquitto instance.
 *  on_connect - a callback function in the following form:
 *               void callback(void *obj, int rc)
 *
 * Callback Parameters:
 *  obj - the user data provided in <mosquitto_new>
 *  rc -  the return code of the connection response, one of:
 *
 * * 0 - success
 * * 1 - connection refused (unacceptable protocol version)
 * * 2 - connection refused (identifier rejected)
 * * 3 - connection refused (broker unavailable)
 * * 4-255 - reserved for future use
 */
libmosq_EXPORT void mosquitto_connect_callback_set(struct mosquitto *mosq, void (*on_connect)(void *, int));
 
/*
 * Function: mosquitto_disconnect_callback_set
 *
 * Set the disconnect callback. This is called when the broker has received the
 * DISCONNECT command and has disconnected the client.
 * 
 * Parameters:
 *  mosq -          a valid mosquitto instance.
 *  on_disconnect - a callback function in the following form:
 *                  void callback(void *obj)
 *
 * Callback Parameters:
 *  obj - the user data provided in <mosquitto_new>
 */
libmosq_EXPORT void mosquitto_disconnect_callback_set(struct mosquitto *mosq, void (*on_disconnect)(void *));
 
/*
 * Function: mosquitto_publish_callback_set
 *
 * Set the publish callback. This is called when a message initiated with
 * <mosquitto_publish> has been sent to the broker successfully.
 * 
 * Parameters:
 *  mosq -       a valid mosquitto instance.
 *  on_publish - a callback function in the following form:
 *               void callback(void *obj, uint16_t mid)
 *
 * Callback Parameters:
 *  obj - the user data provided in <mosquitto_new>
 *  mid - the message id of the sent message.
 */
libmosq_EXPORT void mosquitto_publish_callback_set(struct mosquitto *mosq, void (*on_publish)(void *, uint16_t));

/*
 * Function: mosquitto_message_callback_set
 *
 * Set the message callback. This is called when a message is received from the
 * broker.
 * 
 * Parameters:
 *  mosq -       a valid mosquitto instance.
 *  on_message - a callback function in the following form:
 *               void callback(void *obj, const struct mosquitto_message *message)
 *
 * Callback Parameters:
 *  obj -     the user data provided in <mosquitto_new>
 *  message - the message data. This variable and associated memory will be
 *            freed by the library after the callback completes. The client
 *            should make copies of any of the data it requires.
 *
 * See Also:
 * 	<mosquitto_message_copy>
 */
libmosq_EXPORT void mosquitto_message_callback_set(struct mosquitto *mosq, void (*on_message)(void *, const struct mosquitto_message *));

/*
 * Function: mosquitto_subscribe_callback_set
 *
 * Set the subscribe callback. This is called when the broker responds to a
 * subscription request.
 * 
 * Parameters:
 *  mosq -         a valid mosquitto instance.
 *  on_subscribe - a callback function in the following form:
 *                 void callback(void *obj, uint16_t mid, int qos_count, const uint8_t *granted_qos)
 *
 * Callback Parameters:
 *  obj -         the user data provided in <mosquitto_new>
 *  mid -         the message id of the subscribe message.
 *  qos_count -   the number of granted subscriptions (size of granted_qos).
 *  granted_qos - an array of integers indicating the granted QoS for each of
 *                the subscriptions.
 */
libmosq_EXPORT void mosquitto_subscribe_callback_set(struct mosquitto *mosq, void (*on_subscribe)(void *, uint16_t, int, const uint8_t *));

/*
 * Function: mosquitto_unsubscribe_callback_set
 *
 * Set the unsubscribe callback. This is called when the broker responds to a
 * unsubscription request.
 * 
 * Parameters:
 *  mosq -           a valid mosquitto instance.
 *  on_unsubscribe - a callback function in the following form:
 *                   void callback(void *obj, uint16_t mid)
 *
 * Callback Parameters:
 *  obj - the user data provided in <mosquitto_new>
 *  mid - the message id of the unsubscribe message.
 */
libmosq_EXPORT void mosquitto_unsubscribe_callback_set(struct mosquitto *mosq, void (*on_unsubscribe)(void *, uint16_t));

/*
 * Function: mosquitto_message_retry_set
 *
 * Set the number of seconds to wait before retrying messages. This applies to
 * publish messages with QoS>0. May be called at any time.
 * 
 * Parameters:
 *  mosq -          a valid mosquitto instance.
 *  message_retry - the number of seconds to wait for a response before
 *                  retrying. Defaults to 60.
 */
libmosq_EXPORT void mosquitto_message_retry_set(struct mosquitto *mosq, unsigned int message_retry);

#ifdef __cplusplus
}
#endif

#endif
