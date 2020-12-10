//
//  BlankScreenStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#ifndef BlankScreenStimulus_hpp
#define BlankScreenStimulus_hpp

#include "MetalStimulus.hpp"


BEGIN_NAMESPACE_MW


class BlankScreenStimulus : public MetalStimulus {
    
public:
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit BlankScreenStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void draw(id<MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor *renderPassDescriptor) override;
    
    VariablePtr r;
    VariablePtr g;
    VariablePtr b;
    
    double current_r, current_g, current_b;
    double last_r, last_g, last_b;
    
};


END_NAMESPACE_MW


#endif /* BlankScreenStimulus_hpp */
