/*!
* @header Marionette
 *
 */

#import <Cocoa/Cocoa.h>
#import "MonkeyWorksCore/Client.h"

using namespace mw;

@interface MarionetteClient : NSObject {
	Client *client;
	
	BOOL sent_run_event;
	BOOL sent_experiment;
	BOOL experiment_loaded;
	BOOL sent_close_experiment;
	BOOL sent_open_data_file;
	BOOL sent_close_data_file;
	BOOL data_file_open;
	BOOL state_system_running;
	BOOL experiment_ended;
	BOOL asserted;
	
	NSArray *permitted_error_messages;
	NSMutableArray *expected_messages;
	NSMutableArray *expected_events;
	
}

@property (readwrite, copy) NSArray * permittedErrorMessages;
@property (readwrite, copy) NSMutableArray * expectedMessages;
@property (readwrite, copy) NSMutableArray * expectedEvents;
@property (readwrite, assign) BOOL sentRunEvent, stateSystemRunning;
@property (readwrite, assign) BOOL asserted, experimentEnded;
@property (readwrite, assign) BOOL experimentLoaded, sentExperiment, sentCloseExperiment;
@property (readwrite, assign) BOOL sentOpenDataFile, sentCloseDataFile, dataFileOpen;

@end
