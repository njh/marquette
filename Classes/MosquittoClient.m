//
//  MosquittoClient.m
//
//  Copyright 2012 Nicholas Humfrey. All rights reserved.
//

#import "MosquittoClient.h"
#import "mosquitto.h"

@implementation MosquittoClient

@synthesize host;
@synthesize port;
@synthesize keepAlive;
@synthesize cleanSession;

// Initialize is called just before the first object is allocated
+ (void)initialize {
    NSLog(@"mosquitto_lib_init()");
    mosquitto_lib_init();
}

+ (NSString*)version {
    int major, minor, revision;
    mosquitto_lib_version(&major, &minor, &revision);
    return [NSString stringWithFormat:@"%d.%d.%d", major, minor, revision];
}

- (MosquittoClient*) initWithClientId: (NSString*) clientId {
    if ((self = [super init])) {
        const char* cstrClientId = [clientId cStringUsingEncoding:NSUTF8StringEncoding];
        [self setHost: nil];
        [self setPort: 1883];
        [self setKeepAlive: 30];
        [self setCleanSession: YES];

        mosq = mosquitto_new(cstrClientId, self);
        timer = nil;
    }
    return self;
}

- (void) setLogPriorities: (int)priorities destinations:(int)destinations {
    mosquitto_log_init(mosq, priorities, destinations);
}

- (void) connect {
    const char* cstrHost = [host cStringUsingEncoding:NSASCIIStringEncoding];

    mosquitto_connect(mosq, cstrHost, port, keepAlive, cleanSession);

    // Setup timer to handle network events
    // FIXME: better way to do this - hook into iOS Run Loop select() ?
    // or run in seperate thread?
    timer = [NSTimer scheduledTimerWithTimeInterval:0.01 // 10ms
                                             target:self
                                           selector:@selector(loop:)
                                           userInfo:nil
                                            repeats:YES];
}

- (void) reconnect {
    mosquitto_reconnect(mosq);
}

- (void) disconnect {
    mosquitto_disconnect(mosq);
}

- (void) loop: (NSTimer *)timer {
    mosquitto_loop(mosq, 0);
}

// FIXME: add retained parameter?
- (void)publishString: (NSString *)payload toTopic:(NSString *)topic retain:(BOOL)retain {
    const char* cstrTopic = [topic cStringUsingEncoding:NSUTF8StringEncoding];
    const uint8_t* cstrPayload = (const uint8_t*)[payload cStringUsingEncoding:NSUTF8StringEncoding];
    mosquitto_publish(mosq, NULL, cstrTopic, [payload length], cstrPayload, 0, retain);
}

- (void) dealloc {
    if (mosq) {
        [self disconnect];
        mosquitto_destroy(mosq);
        mosq = NULL;
    }

    if (timer) {
        [timer invalidate];
        timer = nil;
    }

    [super dealloc];
}

// FIXME: how and when to call mosquitto_lib_cleanup() ?

@end
