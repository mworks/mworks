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

#include "Mask.h"
#include "GratingData.h"


BEGIN_NAMESPACE_MW


using DriftingGratingStimulusBase = DynamicStimulusBase<BasicTransformStimulus>;


class DriftingGratingStimulus : public DriftingGratingStimulusBase {
    
public:
    static const std::string DIRECTION;
    static const std::string STARTING_PHASE;
    static const std::string FREQUENCY;
    static const std::string SPEED;
    static const std::string GRATING_TYPE;
    static const std::string MASK;
    static const std::string GRATING_SAMPLE_RATE;
    static const std::string INVERTED;
    static const std::string STD_DEV;
    static const std::string MEAN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit DriftingGratingStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
    VertexPositionArray getVertexPositions() const override;
    GLKMatrix4 getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const override;
    
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void destroy(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    void postDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    void drawFrame(boost::shared_ptr<StimulusDisplay> display) override;
    
    shared_ptr<Variable> direction_in_degrees;
    shared_ptr<Variable> spatial_frequency;
    shared_ptr<Variable> speed;
    shared_ptr<Variable> starting_phase;
    
    shared_ptr<Mask> mask;
    shared_ptr<GratingData> grating;
    
    GLint alphaUniformLocation = -1;
    GLuint gratingTexCoordBuffer = 0;
    GLuint maskTexCoordsBuffer = 0;
    GLuint gratingTexture = 0;
    GLuint maskTexture = 0;
    
    float last_phase;
    
};


END_NAMESPACE_MW


#endif 





















