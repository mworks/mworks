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

#include <CoreVideo/CVDisplayLink.h>

#include "AppleStimulusDisplay.hpp"


BEGIN_NAMESPACE_MW


class MacOSStimulusDisplay : public AppleStimulusDisplay {
    
public:
    explicit MacOSStimulusDisplay(const Configuration &config);
    ~MacOSStimulusDisplay();
    
    const CVTimeStamp& getCurrentOutputTimeStamp() const { return currentOutputTimeStamp; }
    
private:
    static CVReturn displayLinkCallback(CVDisplayLinkRef displayLink,
                                        const CVTimeStamp *now,
                                        const CVTimeStamp *outputTime,
                                        CVOptionFlags flagsIn,
                                        CVOptionFlags *flagsOut,
                                        void *_display);
    
    void prepareContext(int context_id, bool isMainContext) override;
    void setDisplayGamma(CGDirectDisplayID displayID,
                         CGGammaValue redGamma,
                         CGGammaValue greenGamma,
                         CGGammaValue blueGamma);
    void startDisplayUpdates() override;
    void stopDisplayUpdates() override;
    
    CVDisplayLinkRef displayLink;
    int64_t lastFrameTime;
    CVTimeStamp currentOutputTimeStamp;
    
    bool didSetDisplayGamma;
    
};


END_NAMESPACE_MW


#endif
