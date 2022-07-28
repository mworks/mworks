/* MWCalibratorWindowController */
// COMMENTS!!!!!

#import "MWorksCocoa/MWClientProtocol.h"
#import "MWCalibratorRecord.h"

enum hIndicies{H_OFFSET_INDEX=0, H_GAIN_INDEX=1};
enum vIndicies{V_OFFSET_INDEX=0, V_GAIN_INDEX=2};
const int MIN_NUM_CALIBRATION_PARAMS = 3;
#define DEFAULT_CALIBRATOR_DISPLAY @"<No calibrator>"

#define DEFAULT_H_OFFSET 0.0
#define DEFAULT_V_OFFSET 0.0
#define DEFAULT_H_GAIN 1.0
#define DEFAULT_V_GAIN 1.0

@interface MWCalibratorWindowController : NSWindowController {			
	IBOutlet id<MWClientProtocol> __unsafe_unretained delegate;
	
	NSString *selected_calibrator_name;
	float v_offset;
	float v_gain;
	float h_offset;
	float h_gain;
	
	NSMutableArray *calibratorRecords;
	int displayedCalibratorIndex;
	NSNumber *calibratorAnnounceCode;

	NSLock *crLock;
}


- (IBAction)resetCalibratorParams:(id)sender;
- (IBAction)updateCalibratorParams:(id)sender;

// Accessors
@property (nonatomic, assign) BOOL preserveHiddenParamsOnUpdate;
@property (readwrite, assign) float vOffset, hOffset, vGain, hGain;
@property (readwrite, copy) NSString *selectedCalibratorName;
@property (nonatomic, readwrite, unsafe_unretained) id delegate;

@end
