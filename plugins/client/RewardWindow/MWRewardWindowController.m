
#import "MWRewardWindowController.h"

@implementation MWRewardWindowController

#define MW_REWARD_WINDOW_DURATION @"Reward Window - duration (ms)"
#define MW_REWARD_WINDOW_VAR_NAME @"Reward Window - var name"

@synthesize delegate;

- (void)setDelegate:(id)new_delegate {
	if(![new_delegate respondsToSelector:@selector(codeForTag:)] ||
	   ![new_delegate respondsToSelector:@selector(updateVariableWithCode:withData:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWRewardWindowController"];		
	}
	
	delegate = new_delegate;
	
	NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];	
	duration_ms = [ud floatForKey:MW_REWARD_WINDOW_DURATION];
	reward_var_name = [[ud stringForKey:MW_REWARD_WINDOW_VAR_NAME] copy];
	
}

@synthesize rewardVarName = reward_var_name;

- (void)setRewardVarName:(NSString *)new_reward_var_name {
	reward_var_name = [new_reward_var_name copy];
	[[NSUserDefaults standardUserDefaults] setObject:reward_var_name forKey:MW_REWARD_WINDOW_VAR_NAME];
}

@synthesize duration = duration_ms;
- (void)setDuration:(float)new_duration {
	duration_ms = new_duration;
	[[NSUserDefaults standardUserDefaults] setFloat:new_duration forKey:MW_REWARD_WINDOW_DURATION];
}

- (IBAction)sendReward:(id)sender {
    // Finish editing in all fields
    if (![self.window makeFirstResponder:self.window]) {
        [self.window endEditingFor:nil];
    }
    
	if(delegate != nil) {
		if(self.duration < 0) {
			self.duration = 0;
		}
        
        int rewardVarCode = [delegate codeForTag:self.rewardVarName].intValue;
        if (rewardVarCode < 0) {
            merror(mw::M_CLIENT_MESSAGE_DOMAIN,
                   "Cannot send reward: Variable \"%s\" was not found",
                   self.rewardVarName.UTF8String);
        } else {
            mw::Datum value(self.duration * 1000);
            [delegate updateVariableWithCode:rewardVarCode withData:&value];
        }
	}
}


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    if (self.rewardVarName) {
        [workspaceState setObject:self.rewardVarName forKey:@"rewardVarName"];
    }
    [workspaceState setObject:[NSNumber numberWithFloat:self.duration] forKey:@"rewardDurationMS"];
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSString *newRewardVarName = [workspaceState objectForKey:@"rewardVarName"];
    if (newRewardVarName && [newRewardVarName isKindOfClass:[NSString class]]) {
        self.rewardVarName = newRewardVarName;
    }
    
    NSNumber *newRewardDurationMS = [workspaceState objectForKey:@"rewardDurationMS"];
    if (newRewardDurationMS && [newRewardDurationMS isKindOfClass:[NSNumber class]]) {
        self.duration = newRewardDurationMS.floatValue;
    }
}


@end
