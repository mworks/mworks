/* MWRewardWindowController */

#import "MWorksCocoa/MWClientProtocol.h"


@interface MWRewardWindowController : NSWindowController <MWClientPluginWorkspaceState> {

	NSString *reward_var_name;
	float duration_ms;
	IBOutlet id<MWClientProtocol> __unsafe_unretained delegate;
}

@property (nonatomic, readwrite, unsafe_unretained) id delegate;
@property (nonatomic, readwrite, copy) NSString *rewardVarName;
@property (nonatomic, readwrite, assign) float duration;

- (IBAction)sendReward:(id)sender;

@end