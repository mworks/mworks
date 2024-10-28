/*
 *  DriftingGratingStimulus.h
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef DRIFTING_GRATNG_STIMULUS_H
#define DRIFTING_GRATNG_STIMULUS_H

#include "DriftingGratingStimulusShaderTypes.h"

using namespace mw::drifting_grating_stimulus;


BEGIN_NAMESPACE_MW


using DriftingGratingStimulusBase = DynamicStimulusBase<ColoredTransformStimulus>;


class DriftingGratingStimulus : public DriftingGratingStimulusBase {
    
public:
    static const std::string DIRECTION;
    static const std::string CENTRAL_STARTING_PHASE;
    static const std::string STARTING_PHASE;
    static const std::string FREQUENCY;
    static const std::string SPEED;
    static const std::string COMPUTE_PHASE_INCREMENTALLY;
    static const std::string GRATING_TYPE;
    static const std::string MASK;
    static const std::string INVERTED;
    static const std::string STD_DEV;
    static const std::string MEAN;
    static const std::string NORMALIZED;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit DriftingGratingStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    static GratingType gratingTypeFromName(const std::string &name);
    static MaskType maskTypeFromName(const std::string &name);
    
    void loadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void configureBlending(MTLRenderPipelineColorAttachmentDescriptor *colorAttachment) const override;
    simd::float4x4 getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const override;
    
    void stopPlaying() override;
    
    const boost::shared_ptr<Variable> direction_in_degrees;
    const boost::shared_ptr<Variable> central_starting_phase;
    const boost::shared_ptr<Variable> starting_phase;
    const boost::shared_ptr<Variable> spatial_frequency;
    const boost::shared_ptr<Variable> speed;
    const bool computePhaseIncrementally;
    const boost::shared_ptr<Variable> gratingTypeName;
    const boost::shared_ptr<Variable> maskTypeName;
    const boost::shared_ptr<Variable> inverted;
    const boost::shared_ptr<Variable> std_dev;
    const boost::shared_ptr<Variable> mean;
    const boost::shared_ptr<Variable> normalized;
    
    double current_direction_in_degrees, current_starting_phase, current_spatial_frequency, current_speed;
    std::string current_grating_type_name;
    GratingType current_grating_type;
    std::string current_mask_type_name;
    MaskType current_mask_type;
    bool current_grating_is_mask, current_inverted;
    double current_std_dev, current_mean;
    bool current_normalized;
    
    double lastElapsedSeconds;
    double last_phase;
    
};


END_NAMESPACE_MW


#endif /* DRIFTING_GRATNG_STIMULUS_H */
