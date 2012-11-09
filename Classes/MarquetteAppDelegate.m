//
//  MarquetteAppDelegate.m
//  Marquette
//
//  Created by Nicholas Humfrey on 15/01/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "MarquetteAppDelegate.h"
#import "MarquetteViewController.h"

#include "getMacAddress.h"
#include "mosquitto.h"


@implementation MarquetteAppDelegate

@synthesize window;
@synthesize viewController;
@synthesize mosquittoClient;


#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.

	// Set the view controller as the window's root view controller and display.
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];

	// FIXME: is sending out the MAC address a security issue?
	NSString *clientId = [NSString stringWithFormat:@"marquette_%@", getMacAddress()];
	NSLog(@"Client ID: %@", clientId);
    mosquittoClient = [[MosquittoClient alloc] initWithClientId:clientId];
	// FIXME: deal with failed inits

	// FIXME: only if compiled in debug mode?
	//[mosquittoClient setLogPriorities:MOSQ_LOG_ALL destinations:MOSQ_LOG_STDERR];
	[mosquittoClient setDelegate: self.viewController];

    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
	[mosquittoClient disconnect];
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
	[mosquittoClient disconnect];
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
	[mosquittoClient reconnect];
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
	[mosquittoClient disconnect];
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
	if (mosquittoClient) [mosquittoClient dealloc];

    [super dealloc];
}


@end
