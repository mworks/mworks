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

#include "AppleStimulusDisplay.hpp"


BEGIN_NAMESPACE_MW


class MacOSStimulusDisplay : public AppleStimulusDisplay {
    
public:
    explicit MacOSStimulusDisplay(const Configuration &config);
    ~MacOSStimulusDisplay();
    
private:
    CADisplayLink * getDisplayLink() const override { return displayLink; }
    
    void prepareContext(int context_id, bool isMainContext) override;
    void setDisplayGamma(CGDirectDisplayID displayID,
                         CGGammaValue redGamma,
                         CGGammaValue greenGamma,
                         CGGammaValue blueGamma);
    
    CADisplayLink *displayLink;
    bool didSetDisplayGamma;
    
};


END_NAMESPACE_MW


#endif
