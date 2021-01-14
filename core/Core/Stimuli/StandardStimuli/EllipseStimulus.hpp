//
//  EllipseStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/14/21.
//

#ifndef EllipseStimulus_hpp
#define EllipseStimulus_hpp

#include "ColoredTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


class EllipseStimulus : public ColoredTransformStimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using ColoredTransformStimulus::ColoredTransformStimulus;
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void loadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
};


END_NAMESPACE_MW


#endif /* EllipseStimulus_hpp */
