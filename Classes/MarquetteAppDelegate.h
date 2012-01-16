//
//  MarquetteAppDelegate.h
//  Marquette
//
//  Created by Nicholas Humfrey on 15/01/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MarquetteViewController;

@interface MarquetteAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    MarquetteViewController *viewController;
	struct mosquitto *mosq;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet MarquetteViewController *viewController;

@end

