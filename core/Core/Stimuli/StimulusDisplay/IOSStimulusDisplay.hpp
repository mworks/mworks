//
//  IOSStimulusDisplay.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/15/17.
//
//

#ifndef IOSStimulusDisplay_hpp
#define IOSStimulusDisplay_hpp

#include "AppleStimulusDisplay.hpp"


BEGIN_NAMESPACE_MW


class IOSStimulusDisplay : public AppleStimulusDisplay {
    
public:
    explicit IOSStimulusDisplay(const Configuration &config);
    ~IOSStimulusDisplay();
    
private:
    CADisplayLink * getDisplayLink() const override { return displayLink; }
    
    void prepareContext(int context_id, bool isMainContext) override;
    
    CADisplayLink *displayLink;
    
};


END_NAMESPACE_MW


#endif /* IOSStimulusDisplay_hpp */
