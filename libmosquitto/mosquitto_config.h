/* ============================================================
 * Control compile time options.
 *
 * Largely, these are options that are designed to make mosquitto run more
 * easily in restrictive environments by removing features.
 * ============================================================ */

#ifndef CMAKE
/* Only use the compile time options defined here from the standard Makefile. */


/* Uncomment to compile with tcpd/libwrap support. */
//#define WITH_WRAP

/* Compile with database upgrading support? If disabled, mosquitto won't
 * automatically upgrade old database versions. */
//#define WITH_DB_UPGRADE

/* Compile with memory tracking support? If disabled, mosquitto won't track
 * heap memory usage nor export '$SYS/broker/heap/current size', but will use
 * slightly less memory and CPU time. */
//#define WITH_MEMORY_TRACKING

/* Compile with persistent database support. This allows the broker to store
 * retained messages and durable subscriptions to a file periodically and on
 * shutdown. This is usually desirable (and is suggested by the MQTT spec), but
 * it can be disabled by commenting out this define if required.
 */
//#define WITH_PERSISTENCE

/* Compile with bridge support included. This allow the broker to connect to
 * other brokers and subscribe/publish to topics. You probably want to leave
 * this included unless you want to save a very small amount of memory size and
 * CPU time.
 */
//#define WITH_BRIDGE

/* Compile with strict protocol support. This means that both the client
 * library and the broker will be very strict about protocol compliance on
 * incoming data. Neither of them will return an error on incorrect "remaining
 * length" values if this is commented out. The old behaviour (prior to 0.12)
 * is equivalent to compiling with WITH_STRICT_PROTOCOL defined and means that
 * clients will be immediately disconnected from the broker on non-compliance.
 */
//#define WITH_STRICT_PROTOCOL

/* Use the username/password and ACL checks defined in security_external.c
 * This is empty by default, but gives a more straightforward way of adding
 * support for existing username/password databases to mosquitto.
 * Uncommenting without adding your own code to security_external.c will
 * result in all access being denied.
 * It also enables the db_* config options in mosquitto.conf.
 * Get in touch with the authors if you need help adding support for your
 * system.
 */
//#define WITH_EXTERNAL_SECURITY_CHECKS
#endif
