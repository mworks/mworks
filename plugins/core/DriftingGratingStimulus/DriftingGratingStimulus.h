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
    static const std::string INVERTED;
    static const std::string STD_DEV;
    static const std::string MEAN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit DriftingGratingStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    enum class GratingType {
        sinusoid = 1,
        square = 2,
        triangle = 3,
        sawtooth = 4
    };
    
    enum class MaskType {
        rectangle = 1,
        ellipse = 2,
        gaussian = 3
    };
    
    static GratingType gratingTypeFromName(const std::string &name);
    static MaskType maskTypeFromName(const std::string &name);
    
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
    VertexPositionArray getVertexPositions() const override;
    GLKMatrix4 getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const override;
    
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void destroy(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    void postDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    void drawFrame(boost::shared_ptr<StimulusDisplay> display) override;
    
    const boost::shared_ptr<Variable> direction_in_degrees;
    const boost::shared_ptr<Variable> starting_phase;
    const boost::shared_ptr<Variable> spatial_frequency;
    const boost::shared_ptr<Variable> speed;
    const std::string gratingTypeName;
    const GratingType gratingType;
    const std::string maskTypeName;
    const MaskType maskType;
    const boost::shared_ptr<Variable> inverted;
    const boost::shared_ptr<Variable> std_dev;
    const boost::shared_ptr<Variable> mean;
    
    double current_direction_in_degrees, current_starting_phase, current_spatial_frequency, current_speed;
    bool current_inverted;
    double current_std_dev, current_mean;
    
    double last_direction_in_degrees, last_starting_phase, last_spatial_frequency, last_speed;
    bool last_inverted;
    double last_std_dev, last_mean;
    
    double last_phase;
    
    GLint alphaUniformLocation = -1;
    GLint invertedUniformLocation = -1;
    GLint stdDevUniformLocation = -1;
    GLint meanUniformLocation = -1;
    GLuint gratingCoordBuffer = 0;
    GLuint maskCoordsBuffer = 0;
    
};


END_NAMESPACE_MW


#endif 





















