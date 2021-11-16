//
//  LegacyStandardStimuli.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#ifndef LegacyStandardStimuli_hpp
#define LegacyStandardStimuli_hpp

#include <ImageIO/ImageIO.h>

#include "CFObjectPtr.h"
#include "Stimulus.h"


#if MWORKS_HAVE_OPENGL


BEGIN_NAMESPACE_MW

inline namespace legacy_standard_stimuli {


class BlankScreen : public Stimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit BlankScreen(const Map<ParameterValue> &parameters);
    
    RenderingMode getRenderingMode() const override { return RenderingMode::OpenGL; }
    
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    shared_ptr<Variable> r;
    shared_ptr<Variable> g;
    shared_ptr<Variable> b;
    float last_r,last_g,last_b;
    
};


class TransformStimulus : public Stimulus, boost::noncopyable {
    
public:
    static const std::string X_SIZE;
    static const std::string Y_SIZE;
    static const std::string X_POSITION;
    static const std::string Y_POSITION;
    static const std::string ROTATION;
    static const std::string FULLSCREEN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TransformStimulus(const Map<ParameterValue> &parameters);
    
    RenderingMode getRenderingMode() const override { return RenderingMode::OpenGL; }
    
    void load(shared_ptr<StimulusDisplay> display) override;
    void unload(shared_ptr<StimulusDisplay> display) override;
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    static constexpr GLint numVertices = 4;
    static constexpr GLint componentsPerVertex = 2;
    using VertexPositionArray = std::array<GLfloat, numVertices*componentsPerVertex>;
    
    virtual gl::Shader getVertexShader() const = 0;
    virtual gl::Shader getFragmentShader() const = 0;
    
    virtual VertexPositionArray getVertexPositions() const;
    void getCurrentSize(float &sizeX, float &sizeY) const;
    virtual GLKMatrix4 getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const;
    
    virtual void setBlendEquation() = 0;
    
    virtual void prepare(const boost::shared_ptr<StimulusDisplay> &display) { }
    virtual void destroy(const boost::shared_ptr<StimulusDisplay> &display) { }
    virtual void preDraw(const boost::shared_ptr<StimulusDisplay> &display) { }
    virtual void postDraw(const boost::shared_ptr<StimulusDisplay> &display) { }
    
    const shared_ptr<Variable> xoffset;
    const shared_ptr<Variable> yoffset;
    shared_ptr<Variable> xscale;
    shared_ptr<Variable> yscale;
    const shared_ptr<Variable> rotation;
    const bool fullscreen;
    
    float current_posx, current_posy, current_sizex, current_sizey, current_rot;
    float last_posx, last_posy, last_sizex, last_sizey, last_rot;
    
    GLuint program = 0;
    GLint mvpMatrixUniformLocation = -1;
    GLuint vertexArray = 0;
    GLuint vertexPositionBuffer = 0;
    
};


class AlphaBlendedTransformStimulus : public TransformStimulus {
    
public:
    static const std::string ALPHA_MULTIPLIER;
    static const std::string SOURCE_BLEND_FACTOR;
    static const std::string DEST_BLEND_FACTOR;
    static const std::string SOURCE_ALPHA_BLEND_FACTOR;
    static const std::string DEST_ALPHA_BLEND_FACTOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AlphaBlendedTransformStimulus(const Map<ParameterValue> &parameters);
    
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    static constexpr GLenum defaultSourceBlendFactor = GL_SRC_ALPHA;
    static constexpr GLenum defaultDestBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
    
    static GLenum blendFactorFromName(const std::string &name, GLenum fallback);
    
    void setBlendEquation() override;
    
    const shared_ptr<Variable> alpha_multiplier;
    const shared_ptr<Variable> sourceBlendFactorName;
    const shared_ptr<Variable> destBlendFactorName;
    const shared_ptr<Variable> sourceAlphaBlendFactorName;
    const shared_ptr<Variable> destAlphaBlendFactorName;
    
    float current_alpha;
    std::string current_source_blend_factor_name;
    GLenum current_source_blend_factor;
    std::string current_dest_blend_factor_name;
    GLenum current_dest_blend_factor;
    std::string current_source_alpha_blend_factor_name;
    GLenum current_source_alpha_blend_factor;
    std::string current_dest_alpha_blend_factor_name;
    GLenum current_dest_alpha_blend_factor;
    
    float last_alpha;
    std::string last_source_blend_factor_name;
    GLenum last_source_blend_factor;
    std::string last_dest_blend_factor_name;
    GLenum last_dest_blend_factor;
    std::string last_source_alpha_blend_factor_name;
    GLenum last_source_alpha_blend_factor;
    std::string last_dest_alpha_blend_factor_name;
    GLenum last_dest_alpha_blend_factor;
    
};


// Alias to avoid breaking existing code
using BasicTransformStimulus = AlphaBlendedTransformStimulus;


class ColoredTransformStimulus : public BasicTransformStimulus {
    
public:
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ColoredTransformStimulus(const Map<ParameterValue> &parameters);
    
    void draw(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    shared_ptr<Variable> r;
    shared_ptr<Variable> g;
    shared_ptr<Variable> b;
    
    float current_r, current_g, current_b;
    float last_r, last_g, last_b;
    
    GLint colorUniformLocation = -1;
    
};


class RectangleStimulus : public ColoredTransformStimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using ColoredTransformStimulus::ColoredTransformStimulus;
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
};


class CircleStimulus : public ColoredTransformStimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using ColoredTransformStimulus::ColoredTransformStimulus;
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
};


class BaseImageStimulus : public BasicTransformStimulus {
    
public:
    static VertexPositionArray getVertexPositions(double aspectRatio);
    
    using BasicTransformStimulus::BasicTransformStimulus;
    
protected:
    gl::Shader getVertexShader() const override;
    gl::Shader getFragmentShader() const override;
    
    VertexPositionArray getVertexPositions() const override;
    
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void destroy(const boost::shared_ptr<StimulusDisplay> &display) override;
    void preDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    void postDraw(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    virtual double getAspectRatio() const = 0;
    
    GLint alphaUniformLocation = -1;
    GLuint texture = 0;
    GLuint texCoordsBuffer = 0;
    
};


class ImageStimulus : public BaseImageStimulus {
    
public:
    static const std::string PATH;
    static const std::string ANNOUNCE_LOAD;
    
    static cf::ObjectPtr<CGImageSourceRef> loadImageFile(const std::string &filename,
                                                         std::string &fileHash,
                                                         bool announce = true);
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ImageStimulus(const Map<ParameterValue> &parameters);
    
    void load(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void prepare(const boost::shared_ptr<StimulusDisplay> &display) override;
    void destroy(const boost::shared_ptr<StimulusDisplay> &display) override;
    
    double getAspectRatio() const override;
    
    const VariablePtr path;
    const bool announceLoad;
    
    std::string filename;
    std::string fileHash;
    
    std::size_t width;
    std::size_t height;
    std::unique_ptr<std::uint32_t[]> data;
    
};


} // inline namespace legacy_standard_stimuli

END_NAMESPACE_MW


#endif // MWORKS_HAVE_OPENGL


#endif /* LegacyStandardStimuli_hpp */
