//
//  MarquetteViewController.h
//  Marquette
//
//  Created by Nicholas Humfrey on 15/01/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface MarquetteViewController : UIViewController {
	UISwitch *ledSwitch;

}

@property (nonatomic, retain) IBOutlet UISwitch *ledSwitch;

- (IBAction) ledSwitchAction:(id)sender;

@end

