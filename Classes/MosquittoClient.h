//
//  MosquittoClient.h
//
//  Copyright 2012 Nicholas Humfrey. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol MosquittoClientDeligate

- (void) didConnect: (NSUInteger)code;
- (void) didDisconnect;
- (void) didPublish: (NSUInteger)messageId;

// FIXME: create MosquittoMessage class
- (void) didReceiveMessage: (NSString*)message topic:(NSString*)topic;
- (void) didSubscribe: (NSUInteger)messageId grantedQos:(NSArray*)qos;
- (void) didUnsubscribe: (NSUInteger)messageId;

@end


@interface MosquittoClient : NSObject {
    struct mosquitto *mosq;
    NSString *host;
    unsigned short port;
    unsigned short keepAlive;
    BOOL cleanSession;

    id<MosquittoClientDeligate> delegate;
    NSTimer *timer;
}

@property (readwrite,retain) NSString *host;
@property (readwrite,assign) id<MosquittoClientDeligate> delegate;
@property (readwrite,assign) unsigned short port;
@property (readwrite,assign) unsigned short keepAlive;
@property (readwrite,assign) BOOL cleanSession;

+ (void) initialize;
+ (NSString*) version;


- (MosquittoClient*) initWithClientId: (NSString *)clientId;
- (void) setLogPriorities: (int)priorities destinations:(int)destinations;
- (void) setMessageRetry: (NSUInteger)seconds;
- (void) connect;
- (void) connectToHost: (NSString*)host;
- (void) reconnect;
- (void) disconnect;

- (void)publishString: (NSString *)payload toTopic:(NSString *)topic retain:(BOOL)retain;
//- (void)publishMessage

- (void)subscribe: (NSString *)topic;
- (void)subscribe: (NSString *)topic withQos:(NSUInteger)qos;
- (void)unsubscribe: (NSString *)topic;


// This is called automatically when connected
- (void) loop: (NSTimer *)timer;

@end
