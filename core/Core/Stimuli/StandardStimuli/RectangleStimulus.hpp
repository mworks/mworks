//
//  RectangleStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#ifndef RectangleStimulus_hpp
#define RectangleStimulus_hpp

#include "ColoredTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


class RectangleStimulus : public ColoredTransformStimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using ColoredTransformStimulus::ColoredTransformStimulus;
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void loadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
};


END_NAMESPACE_MW


#endif /* RectangleStimulus_hpp */
