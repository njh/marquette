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

@synthesize redLedSwitch;
@synthesize greenLedSwitch;
@synthesize hostField;
@synthesize connectButton;


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

- (IBAction) redLedSwitchAction:(id)sender {
	MarquetteAppDelegate *app = [[UIApplication sharedApplication] delegate];
	MosquittoClient *mosq = [app mosquittoClient];
    if ([sender isOn]) {
        NSLog(@"Red LED On");
		[mosq publishString:@"1" toTopic:@"nanode/red_led" withQos:0 retain:YES];
    }
    else {
        NSLog(@"Red LED Off");
		[mosq publishString:@"0" toTopic:@"nanode/red_led" withQos:0 retain:YES];
    }
}

- (IBAction) greenLedSwitchAction:(id)sender {
	MarquetteAppDelegate *app = [[UIApplication sharedApplication] delegate];
	MosquittoClient *mosq = [app mosquittoClient];
    if ([sender isOn]) {
        NSLog(@"Green LED On");
		[mosq publishString:@"1" toTopic:@"nanode/green_led" withQos:0 retain:YES];
    }
    else {
        NSLog(@"Green LED Off");
		[mosq publishString:@"0" toTopic:@"nanode/green_led" withQos:0 retain:YES];
    }
}

- (IBAction) connectButtonAction:(id)sender {
	MarquetteAppDelegate *app = [[UIApplication sharedApplication] delegate];
	MosquittoClient *mosq = [app mosquittoClient];

	// (Re-)connect
	//[mosq disconnect]; UITextField
	[mosq setHost: [[self hostField] text]];
	[mosq connect];

	[mosq subscribe:@"nanode/red_led"];
	[mosq subscribe:@"nanode/green_led"];
}

- (void) didConnect:(NSUInteger)code {
	[[self connectButton] setTitle:@"Reconnect" forState:UIControlStateNormal];
}

- (void) didDisconnect {
	[[self connectButton] setTitle:@"Connect" forState:UIControlStateNormal];
}

- (void) didReceiveMessage:(MosquittoMessage*) mosq_msg {

	NSLog(@"%@ => %@", mosq_msg.topic, mosq_msg.payload);

	UISwitch *sw = nil;
	if ([mosq_msg.topic isEqualToString:@"nanode/red_led"]) {
		sw = redLedSwitch;
	} else if ([mosq_msg.topic isEqualToString:@"nanode/green_led"]) {
		sw = greenLedSwitch;
	} else {
		return;
	}

	if ([mosq_msg.payload isEqualToString:@"1"]) {
		[sw setOn: YES];
	} else if ([mosq_msg.payload isEqualToString:@"0"]) {
		[sw setOn: NO];
	}
}

- (void) didPublish: (NSUInteger)messageId {}
- (void) didSubscribe: (NSUInteger)messageId grantedQos:(NSArray*)qos {}
- (void) didUnsubscribe: (NSUInteger)messageId {}

- (void)dealloc {
    [super dealloc];
}

@end
