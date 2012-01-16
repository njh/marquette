//
//  MosquittoClient.h
//
//  Copyright 2012 Nicholas Humfrey. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MosquittoClient : NSObject {
    struct mosquitto *mosq;
    NSString *host;
    unsigned short port;
    unsigned short keepAlive;
    BOOL cleanSession;

    NSTimer *timer;
}

@property (readwrite,retain) NSString *host;
@property (readwrite,assign) unsigned short port;
@property (readwrite,assign) unsigned short keepAlive;
@property (readwrite,assign) BOOL cleanSession;

+ (void) initialize;
+ (NSString*) version;


- (MosquittoClient*) initWithClientId: (NSString *)clientId;
- (void) setLogPriorities: (int)priorities destinations:(int)destinations;
- (void) connect;
//- (void) connectToHost: (NSString*) host;
- (void) reconnect;
- (void) disconnect;

- (void)publishString: (NSString *)payload toTopic:(NSString *)topic retain:(BOOL)retain;
//- (void)publishData

// This is called automatically when connected
- (void) loop: (NSTimer *)timer;

@end
