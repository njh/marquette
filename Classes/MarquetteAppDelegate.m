//
//  MarquetteAppDelegate.m
//  Marquette
//
//  Created by Nicholas Humfrey on 15/01/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "MarquetteAppDelegate.h"
#import "MarquetteViewController.h"

#include "mosquitto.h"


@implementation MarquetteAppDelegate

@synthesize window;
@synthesize viewController;


#pragma mark -
#pragma mark Application lifecycle

- (id) init
{
    self = [super init];
    if (self != nil)
    {
		// Initialise the mosquitto library
        mosquitto_lib_init();
    }

    return self;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

    // Override point for customization after application launch.

	// Set the view controller as the window's root view controller and display.
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];

	mosq = mosquitto_new("marquette_iphone", NULL);

	// FIXME: only if compiled in debug mode?
	mosquitto_log_init(mosq, MOSQ_LOG_ALL, MOSQ_LOG_STDERR);

    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
	mosquitto_disconnect(mosq);
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
	mosquitto_disconnect(mosq);
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
	mosquitto_reconnect(mosq);
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc {
    [viewController release];
    [window release];

	// Free resources used by the mosquitto library
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

    [super dealloc];
}


@end
