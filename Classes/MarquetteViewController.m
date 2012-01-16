//
//  MarquetteViewController.m
//  Marquette
//
//  Created by Nicholas Humfrey on 15/01/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "MarquetteViewController.h"
#import "MarquetteAppDelegate.h"
#import "MosquittoClient.h"

@implementation MarquetteViewController

@synthesize ledSwitch;


/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/


/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];

	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}

- (IBAction) ledSwitchAction:(id)sender {
	MarquetteAppDelegate *app = [[UIApplication sharedApplication] delegate];
	MosquittoClient *mosq = [app mosquittoClient];
    if ([sender isOn]) {
        NSLog(@"LED On");
		[mosq publishString:@"1" toTopic:@"nanode/red_led" retain:YES];
    }
    else {
        NSLog(@"LED Off");
		[mosq publishString:@"0" toTopic:@"nanode/red_led" retain:YES];
    }
}


- (void)dealloc {
    [super dealloc];
}

@end
