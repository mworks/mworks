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

#include "StimulusDisplay.h"


BEGIN_NAMESPACE_MW


class MacOSStimulusDisplay : public StimulusDisplay {
    
public:
    explicit MacOSStimulusDisplay(bool announceIndividualStimuli);
    ~MacOSStimulusDisplay();
    
    const CVTimeStamp& getCurrentOutputTimeStamp() const { return currentOutputTimeStamp; }
    
private:
    static CVReturn displayLinkCallback(CVDisplayLinkRef displayLink,
                                        const CVTimeStamp *now,
                                        const CVTimeStamp *outputTime,
                                        CVOptionFlags flagsIn,
                                        CVOptionFlags *flagsOut,
                                        void *_context);
    
    void prepareContext(int contextIndex) override;
    void setMainDisplayRefreshRate() override;
    void startDisplayUpdates() override;
    void stopDisplayUpdates() override;
    
    std::vector<CVDisplayLinkRef> displayLinks;
    using DisplayLinkContext = std::pair<MacOSStimulusDisplay *, int>;
    std::vector<std::unique_ptr<DisplayLinkContext>> displayLinkContexts;
    int64_t lastFrameTime;
    CVTimeStamp currentOutputTimeStamp;
    
};


END_NAMESPACE_MW


#endif


























