//
//  MaskStimulus.hpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#ifndef MaskStimulus_hpp
#define MaskStimulus_hpp

#include "MaskStimulusShaderTypes.h"

using namespace mw::mask_stimulus;


BEGIN_NAMESPACE_MW


class MaskStimulus : public TransformStimulus {
    
public:
    static const std::string MASK;
    static const std::string INVERTED;
    static const std::string STD_DEV;
    static const std::string MEAN;
    static const std::string NORMALIZED;
    static const std::string EDGE_WIDTH;
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit MaskStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    static MaskType maskTypeFromName(const std::string &name);
    
    void loadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void configureBlending(MTLRenderPipelineColorAttachmentDescriptor *colorAttachment) const override;
    
    const VariablePtr maskTypeName;
    const VariablePtr inverted;
    const VariablePtr std_dev;
    const VariablePtr mean;
    const VariablePtr normalized;
    const VariablePtr edgeWidth;
    const bool haveColor;
    
    VariablePtr colorRed, colorGreen, colorBlue;
    
    std::string current_mask_type_name;
    MaskType current_mask_type;
    bool current_inverted;
    double current_std_dev, current_mean;
    bool current_normalized;
    double current_edge_width;
    double current_color_red, current_color_green, current_color_blue;
    
};


END_NAMESPACE_MW


#endif /* MaskStimulus_hpp */
