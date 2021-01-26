//
//  MaskStimulusShaderTypes.h
//  LayerStimulus
//
//  Created by Christopher Stawarz on 1/26/21.
//  Copyright © 2021 The MWorks Project. All rights reserved.
//

#ifndef MaskStimulusShaderTypes_h
#define MaskStimulusShaderTypes_h


#ifndef __METAL_VERSION__
BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(mask_stimulus)
#endif


enum class MaskType {
    rectangle,
    ellipse,
    gaussian,
    raisedCosine
};


struct MaskParameters {
    MaskType maskType;
    bool inverted;
    float stdDev;
    float mean;
    bool normalized;
    float edgeWidth;
};


#ifndef __METAL_VERSION__
END_NAMESPACE(mask_stimulus)
END_NAMESPACE_MW
#endif


#endif /* MaskStimulusShaderTypes_h */
