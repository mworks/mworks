//
//  ColoredTransformStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include "ColoredTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string ColoredTransformStimulus::COLOR("color");


void ColoredTransformStimulus::describeComponent(ComponentInfo &info) {
    AlphaBlendedTransformStimulus::describeComponent(info);
    info.addParameter(COLOR, "1.0,1.0,1.0");
}


ColoredTransformStimulus::ColoredTransformStimulus(const ParameterValueMap &parameters) :
    AlphaBlendedTransformStimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


Datum ColoredTransformStimulus::getCurrentAnnounceDrawData() {
    auto announceData = AlphaBlendedTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_COLOR_R, current_r);
    announceData.addElement(STIM_COLOR_G, current_g);
    announceData.addElement(STIM_COLOR_B, current_b);
    
    return announceData;
}


void ColoredTransformStimulus::drawMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::drawMetal(display);
    
    current_r = r->getValue().getFloat();
    current_g = g->getValue().getFloat();
    current_b = b->getValue().getFloat();
}


void ColoredTransformStimulus::setCurrentColor(id<MTLRenderCommandEncoder> renderCommandEncoder,
                                               NSUInteger bufferIndex) const
{
    auto currentColor = simd::make_float4(current_r, current_g, current_b, current_alpha);
    [renderCommandEncoder setFragmentBytes:&currentColor length:sizeof(currentColor) atIndex:bufferIndex];
}


END_NAMESPACE_MW
