//
//  DriftingGratingStimulusShaderTypes.h
//  DriftingGratingStimulusPlugin
//
//  Created by Christopher Stawarz on 1/28/21.
//

#ifndef DriftingGratingStimulusShaderTypes_h
#define DriftingGratingStimulusShaderTypes_h


#ifndef __METAL_VERSION__
BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(drifting_grating_stimulus)
#endif


enum {
    nonMaskGratingFunctionConstantIndex
};


enum {
    mvpMatrixBufferIndex,
    gratingCoordsBufferIndex,
    aspectRatioBufferIndex
};


enum {
    gratingParamsBufferIndex,
    maskParamsBufferIndex,
    colorBufferIndex
};


enum class GratingType {
    sinusoid,
    square,
    triangle,
    sawtooth
};


enum class MaskType {
    rectangle,
    ellipse,
    gaussian
};


struct GratingParameters {
    GratingType gratingType;
    bool inverted;
};


struct MaskParameters {
    MaskType maskType;
    float stdDev;
    float mean;
    bool normalized;
};


#ifndef __METAL_VERSION__
END_NAMESPACE(drifting_grating_stimulus)
END_NAMESPACE_MW
#endif


#endif /* DriftingGratingStimulusShaderTypes_h */
