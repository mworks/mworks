//
//  IOSStimulusDisplay.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/15/17.
//
//

#ifndef IOSStimulusDisplay_hpp
#define IOSStimulusDisplay_hpp

#include <thread>

#import <QuartzCore/QuartzCore.h>

#include "AppleStimulusDisplay.hpp"



BEGIN_NAMESPACE_MW


class IOSStimulusDisplay : public AppleStimulusDisplay {
    
    static void displayLinkCallback(CADisplayLink *displayLink, IOSStimulusDisplay &display, int contextIndex);
    
public:
    using DisplayLinkCallback = decltype(&displayLinkCallback);
    
    explicit IOSStimulusDisplay(bool useColorManagement);
    ~IOSStimulusDisplay();
    
    CFTimeInterval getCurrentTargetTimestamp() const { return lastTargetTimestamp; }
    
private:
    void prepareContext(int contextIndex) override;
    void setMainDisplayRefreshRate() override;
    void startDisplayUpdates() override;
    void stopDisplayUpdates() override;
    
    NSMutableArray<CADisplayLink *> *displayLinks;
    std::vector<std::thread> displayLinkThreads;
    CFTimeInterval lastTargetTimestamp;
    
};


END_NAMESPACE_MW


#endif /* IOSStimulusDisplay_hpp */
