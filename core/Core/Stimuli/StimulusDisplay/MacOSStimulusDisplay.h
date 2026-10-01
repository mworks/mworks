/*
 *  MacOSStimulusDisplay.h
 *  MWorksCore
 *
 *  Created by David Cox on 6/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MACOS_STIMULUS_DISPLAY_H_
#define MACOS_STIMULUS_DISPLAY_H_

#include <thread>

#import <QuartzCore/QuartzCore.h>

#include "AppleStimulusDisplay.hpp"


BEGIN_NAMESPACE_MW


class MacOSStimulusDisplay : public AppleStimulusDisplay {
    
    static void displayLinkCallback(CADisplayLink *displayLink, MacOSStimulusDisplay &display);
    
public:
    using DisplayLinkCallback = decltype(&displayLinkCallback);
    
    explicit MacOSStimulusDisplay(const Configuration &config);
    ~MacOSStimulusDisplay();
    
    CFTimeInterval getCurrentTargetTimestamp() const { return lastTargetTimestamp; }
    
private:
    void prepareContext(int context_id, bool isMainContext) override;
    void setDisplayGamma(CGDirectDisplayID displayID,
                         CGGammaValue redGamma,
                         CGGammaValue greenGamma,
                         CGGammaValue blueGamma);
    void startDisplayUpdates() override;
    void stopDisplayUpdates() override;
    
    CADisplayLink *displayLink;
    std::thread displayLinkThread;
    CFTimeInterval lastTargetTimestamp;
    
    bool didSetDisplayGamma;
    
};


END_NAMESPACE_MW


#endif
