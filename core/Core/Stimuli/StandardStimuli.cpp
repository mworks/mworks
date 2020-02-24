/**
 * StandardStimuli.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Fixed spacing.  Added copy constructors, 
 *      destructors, and = operator to object that are added 
 *      to ExpandableLists.  Changed constructors to copy objects instead
 *      of just copying pointers.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "StandardStimuli.h"

#import <Accelerate/Accelerate.h>
#import <CoreGraphics/CoreGraphics.h>

#include <boost/uuid/detail/sha1.hpp>

#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


void BlankScreen::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.setSignature("stimulus/blank_screen");
    info.addParameter("color", "0.5,0.5,0.5");
}


BlankScreen::BlankScreen(const ParameterValueMap &parameters) :
    Stimulus(parameters)
{
    ParsedColorTrio pct(parameters["color"]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
    
    Datum rval = *r;
    Datum gval = *g;
    Datum bval = *b;
    
    last_r = (float)rval;
    last_g = (float)gval;
    last_b = (float)bval;
}


void BlankScreen::draw(shared_ptr<StimulusDisplay> display) {
    
    float _r = (float)*r;
    float _g = (float)*g;
    float _b = (float)*b;
    
    glClearColor(_r,_g,_b,1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    last_r = _r;
    last_g = _g;
    last_b = _b;
    
}


Datum BlankScreen::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_BLANK);
    announceData.addElement(STIM_COLOR_R,last_r);
    announceData.addElement(STIM_COLOR_G,last_g);
    announceData.addElement(STIM_COLOR_B,last_b);
    
    return announceData;
}


const std::string TransformStimulus::X_SIZE("x_size");
const std::string TransformStimulus::Y_SIZE("y_size");
const std::string TransformStimulus::X_POSITION("x_position");
const std::string TransformStimulus::Y_POSITION("y_position");
const std::string TransformStimulus::ROTATION("rotation");
const std::string TransformStimulus::FULLSCREEN("fullscreen");


void TransformStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.addParameter(X_SIZE, false);
    info.addParameter(Y_SIZE, false);
    info.addParameter(X_POSITION, "0.0");
    info.addParameter(Y_POSITION, "0.0");
    info.addParameter(ROTATION, "0.0");
    info.addParameter(FULLSCREEN, "NO");
}


TransformStimulus::TransformStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    xoffset(registerVariable(parameters[X_POSITION])),
    yoffset(registerVariable(parameters[Y_POSITION])),
    rotation(registerVariable(parameters[ROTATION])),
    fullscreen(parameters[FULLSCREEN])
{
    if (!(parameters[X_SIZE].empty())) {
        xscale = registerVariable(parameters[X_SIZE]);
    }
    if (!(parameters[Y_SIZE].empty())) {
        yscale = registerVariable(parameters[Y_SIZE]);
    }
    if (!(xscale || yscale || fullscreen)) {
        throw SimpleException("Either x_size or y_size must be specified");
    }
}


void TransformStimulus::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    auto vertexShader = getVertexShader();
    auto fragmentShader = getFragmentShader();
    program = gl::createProgram({ vertexShader.get(), fragmentShader.get() }).release();
    gl::ProgramUsage programUsage(program);
    
    mvpMatrixUniformLocation = glGetUniformLocation(program, "mvpMatrix");
    
    glGenVertexArrays(1, &vertexArray);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    prepare(display);
    
    glGenBuffers(1, &vertexPositionBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
    auto vertexPositions = getVertexPositions();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.data(), GL_STATIC_DRAW);
    GLint vertexPositionAttribLocation = glGetAttribLocation(program, "vertexPosition");
    glEnableVertexAttribArray(vertexPositionAttribLocation);
    glVertexAttribPointer(vertexPositionAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    Stimulus::load(display);
}


void TransformStimulus::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    destroy(display);
    
    glDeleteBuffers(1, &vertexPositionBuffer);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteProgram(program);
    
    Stimulus::unload(display);
}


void TransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    if (!fullscreen) {
        current_posx = *xoffset;
        current_posy = *yoffset;
        getCurrentSize(current_sizex, current_sizey);
        current_rot = *rotation;
    }
    
    gl::ProgramUsage programUsage(program);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    auto currentMVPMatrix = getCurrentMVPMatrix(display->getProjectionMatrix());
    glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, currentMVPMatrix.m);
    
    gl::Enabled<GL_BLEND> blendEnabled;
    setBlendEquation();
    
    preDraw(display);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
    
    postDraw(display);
    
    if (!fullscreen) {
        last_posx = current_posx;
        last_posy = current_posy;
        last_sizex = current_sizex;
        last_sizey = current_sizey;
        last_rot = current_rot;
    }
}


Datum TransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_BASICTRANSFORM);
    if (fullscreen) {
        announceData.addElement(STIM_FULLSCREEN, true);
    } else {
        announceData.addElement(STIM_POSX,last_posx);
        announceData.addElement(STIM_POSY,last_posy);
        announceData.addElement(STIM_SIZEX,last_sizex);
        announceData.addElement(STIM_SIZEY,last_sizey);
        announceData.addElement(STIM_ROT,last_rot);
    }
    
    return announceData;
}


auto TransformStimulus::getVertexPositions() const -> VertexPositionArray {
    return VertexPositionArray {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
}


void TransformStimulus::getCurrentSize(float &sizeX, float &sizeY) const {
    if (xscale) {
        sizeX = *xscale;
        if (!yscale) {
            sizeY = sizeX;
        }
    }
    if (yscale) {
        sizeY = *yscale;
        if (!xscale) {
            sizeX = sizeY;
        }
    }
}


GLKMatrix4 TransformStimulus::getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const {
    GLKMatrix4 currentMVPMatrix;
    
    if (fullscreen) {
        currentMVPMatrix = GLKMatrix4MakeScale(2.0, 2.0, 1.0);
    } else {
        currentMVPMatrix = GLKMatrix4Translate(projectionMatrix, current_posx, current_posy, 0.0);
        currentMVPMatrix = GLKMatrix4Rotate(currentMVPMatrix, GLKMathDegreesToRadians(current_rot), 0.0, 0.0, 1.0);
        currentMVPMatrix = GLKMatrix4Scale(currentMVPMatrix, current_sizex, current_sizey, 1.0);
    }
    
    return GLKMatrix4Translate(currentMVPMatrix, -0.5, -0.5, 0.0);
}


const std::string AlphaBlendedTransformStimulus::ALPHA_MULTIPLIER("alpha_multiplier");
const std::string AlphaBlendedTransformStimulus::SOURCE_BLEND_FACTOR("source_blend_factor");
const std::string AlphaBlendedTransformStimulus::DEST_BLEND_FACTOR("dest_blend_factor");
const std::string AlphaBlendedTransformStimulus::SOURCE_ALPHA_BLEND_FACTOR("source_alpha_blend_factor");
const std::string AlphaBlendedTransformStimulus::DEST_ALPHA_BLEND_FACTOR("dest_alpha_blend_factor");


void AlphaBlendedTransformStimulus::describeComponent(ComponentInfo &info) {
    TransformStimulus::describeComponent(info);
    info.addParameter(ALPHA_MULTIPLIER, "1.0");
    info.addParameter(SOURCE_BLEND_FACTOR, false);
    info.addParameter(DEST_BLEND_FACTOR, false);
    info.addParameter(SOURCE_ALPHA_BLEND_FACTOR, false);
    info.addParameter(DEST_ALPHA_BLEND_FACTOR, false);
}


AlphaBlendedTransformStimulus::AlphaBlendedTransformStimulus(const ParameterValueMap &parameters) :
    TransformStimulus(parameters),
    alpha_multiplier(registerVariable(parameters[ALPHA_MULTIPLIER])),
    sourceBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[SOURCE_BLEND_FACTOR]))),
    destBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[DEST_BLEND_FACTOR]))),
    sourceAlphaBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[SOURCE_ALPHA_BLEND_FACTOR]))),
    destAlphaBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[DEST_ALPHA_BLEND_FACTOR])))
{ }


void AlphaBlendedTransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    current_alpha = *alpha_multiplier;
    if (!sourceBlendFactorName) {
        current_source_blend_factor = defaultSourceBlendFactor;
    } else {
        current_source_blend_factor_name = sourceBlendFactorName->getValue().getString();
        current_source_blend_factor = blendFactorFromName(current_source_blend_factor_name,
                                                          defaultSourceBlendFactor);
    }
    if (!destBlendFactorName) {
        current_dest_blend_factor = defaultDestBlendFactor;
    } else {
        current_dest_blend_factor_name = destBlendFactorName->getValue().getString();
        current_dest_blend_factor = blendFactorFromName(current_dest_blend_factor_name,
                                                        defaultDestBlendFactor);
    }
    if (!sourceAlphaBlendFactorName) {
        current_source_alpha_blend_factor = current_source_blend_factor;
    } else {
        current_source_alpha_blend_factor_name = sourceAlphaBlendFactorName->getValue().getString();
        current_source_alpha_blend_factor = blendFactorFromName(current_source_alpha_blend_factor_name,
                                                                current_source_blend_factor);
    }
    if (!destAlphaBlendFactorName) {
        current_dest_alpha_blend_factor = current_dest_blend_factor;
    } else {
        current_dest_alpha_blend_factor_name = destAlphaBlendFactorName->getValue().getString();
        current_dest_alpha_blend_factor = blendFactorFromName(current_dest_alpha_blend_factor_name,
                                                              current_dest_blend_factor);
    }
    
    TransformStimulus::draw(display);
    
    last_alpha = current_alpha;
    if (sourceBlendFactorName) {
        last_source_blend_factor_name = current_source_blend_factor_name;
        last_source_blend_factor = current_source_blend_factor;
    }
    if (destBlendFactorName) {
        last_dest_blend_factor_name = current_dest_blend_factor_name;
        last_dest_blend_factor = current_dest_blend_factor;
    }
    if (sourceAlphaBlendFactorName) {
        last_source_alpha_blend_factor_name = current_source_alpha_blend_factor_name;
        last_source_alpha_blend_factor = current_source_alpha_blend_factor;
    }
    if (destAlphaBlendFactorName) {
        last_dest_alpha_blend_factor_name = current_dest_alpha_blend_factor_name;
        last_dest_alpha_blend_factor = current_dest_alpha_blend_factor;
    }
}


Datum AlphaBlendedTransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = TransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_ALPHA,last_alpha);
    
    // Announce blend factors only if they were set explicitly, in order to avoid
    // adding lots of redundant strings to the event file
    if (sourceBlendFactorName) {
        announceData.addElement(SOURCE_BLEND_FACTOR, last_source_blend_factor_name);
    }
    if (destBlendFactorName) {
        announceData.addElement(DEST_BLEND_FACTOR, last_dest_blend_factor_name);
    }
    if (sourceAlphaBlendFactorName) {
        announceData.addElement(SOURCE_ALPHA_BLEND_FACTOR, last_source_alpha_blend_factor_name);
    }
    if (destAlphaBlendFactorName) {
        announceData.addElement(DEST_ALPHA_BLEND_FACTOR, last_dest_alpha_blend_factor_name);
    }
    
    return announceData;
}


GLenum AlphaBlendedTransformStimulus::blendFactorFromName(const std::string &name, GLenum fallback) {
    if (name == "zero") {
        return GL_ZERO;
    } else if (name == "one") {
        return GL_ONE;
    } else if (name == "source_color") {
        return GL_SRC_COLOR;
    } else if (name == "one_minus_source_color") {
        return GL_ONE_MINUS_SRC_COLOR;
    } else if (name == "dest_color") {
        return GL_DST_COLOR;
    } else if (name == "one_minus_dest_color") {
        return GL_ONE_MINUS_DST_COLOR;
    } else if (name == "source_alpha") {
        return GL_SRC_ALPHA;
    } else if (name == "one_minus_source_alpha") {
        return GL_ONE_MINUS_SRC_ALPHA;
    } else if (name == "dest_alpha") {
        return GL_DST_ALPHA;
    } else if (name == "one_minus_dest_alpha") {
        return GL_ONE_MINUS_DST_ALPHA;
    }
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Invalid blend factor: %s", name.c_str());
    return fallback;
};


void AlphaBlendedTransformStimulus::setBlendEquation() {
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(current_source_blend_factor,
                        current_dest_blend_factor,
                        current_source_alpha_blend_factor,
                        current_dest_alpha_blend_factor);
}


const std::string ColoredTransformStimulus::COLOR("color");


void ColoredTransformStimulus::describeComponent(ComponentInfo &info) {
    BasicTransformStimulus::describeComponent(info);
    info.addParameter(COLOR, "1.0,1.0,1.0");
}


ColoredTransformStimulus::ColoredTransformStimulus(const ParameterValueMap &parameters) :
    BasicTransformStimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


void ColoredTransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    current_r = *r;
    current_g = *g;
    current_b = *b;
    
    BasicTransformStimulus::draw(display);
    
    last_r = current_r;
    last_g = current_g;
    last_b = current_b;
}


Datum ColoredTransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = BasicTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_COLOR_R,last_r);
    announceData.addElement(STIM_COLOR_G,last_g);
    announceData.addElement(STIM_COLOR_B,last_b);
    
    return announceData;
}


void ColoredTransformStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::prepare(display);
    colorUniformLocation = glGetUniformLocation(program, "color");
}


void ColoredTransformStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::preDraw(display);
    
    auto currentColor = GLKVector4Make(current_r, current_g, current_b, current_alpha);
    glUniform4fv(colorUniformLocation, 1, currentColor.v);
}


void RectangleStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/rectangle");
}


Datum RectangleStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "rectangle");
    return announceData;
}


gl::Shader RectangleStimulus::getVertexShader() const {
    static const std::string source
    (R"(
     uniform mat4 mvpMatrix;
     in vec4 vertexPosition;
     
     void main() {
         gl_Position = mvpMatrix * vertexPosition;
     }
     )");
    
    return gl::createShader(GL_VERTEX_SHADER, source);
}


gl::Shader RectangleStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     uniform vec4 color;
     out vec4 fragColor;
     
     void main() {
         fragColor = color;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


void CircleStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/circle");
}


Datum CircleStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "circle");
    return announceData;
}


gl::Shader CircleStimulus::getVertexShader() const {
    static const std::string source
    (R"(
     uniform mat4 mvpMatrix;
     in vec4 vertexPosition;
     out vec2 rawPosition;
     
     void main() {
         gl_Position = mvpMatrix * vertexPosition;
         rawPosition = vertexPosition.xy;
     }
     )");
    
    return gl::createShader(GL_VERTEX_SHADER, source);
}


gl::Shader CircleStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     const vec2 center = vec2(0.5, 0.5);
     const float radius = 0.5;
     
     uniform vec4 color;
     in vec2 rawPosition;
     out vec4 fragColor;
     
     void main() {
         //
         // For an explanation of the edge-smoothing technique used here, see either of the following:
         // https://rubendv.be/blog/opengl/drawing-antialiased-circles-in-opengl/
         // http://www.numb3r23.net/2015/08/17/using-fwidth-for-distance-based-anti-aliasing/
         //
         float dist = distance(rawPosition, center);
         float delta = fwidth(dist);
         float alpha = 1.0 - smoothstep(radius - delta, radius, dist);
         fragColor.rgb = color.rgb;
         fragColor.a = alpha * color.a;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


const std::string ImageStimulus::PATH("path");
const std::string ImageStimulus::ANNOUNCE_LOAD("announce_load");


auto ImageStimulus::getVertexPositions(double aspectRatio) -> VertexPositionArray {
    if (aspectRatio > 1.0) {
        return VertexPositionArray {
            0.0f, GLfloat(0.5 - 0.5/aspectRatio),
            1.0f, GLfloat(0.5 - 0.5/aspectRatio),
            0.0f, GLfloat((0.5 - 0.5/aspectRatio) + 1.0/aspectRatio),
            1.0f, GLfloat((0.5 - 0.5/aspectRatio) + 1.0/aspectRatio)
        };
    }
    return VertexPositionArray {
        GLfloat((1.0 - aspectRatio)/2.0), 0.0f,
        GLfloat((1.0 - aspectRatio)/2.0 + aspectRatio), 0.0f,
        GLfloat((1.0 - aspectRatio)/2.0), 1.0f,
        GLfloat((1.0 - aspectRatio)/2.0 + aspectRatio), 1.0f
    };
}


cf::ObjectPtr<CGImageSourceRef> ImageStimulus::loadImageFile(const std::string &filename,
                                                             std::string &fileHash,
                                                             bool announce)
{
    if (announce) {
        mprintf("Loading image %s", filename.c_str());
    }
    
    @autoreleasepool {
        NSData *imageData = nil;
        
        //
        // Load image data
        //
        {
            NSError *errorPtr = nil;
            imageData = [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:(filename.c_str())]
                                               options:0
                                                 error:&errorPtr];
            if (!imageData) {
                throw SimpleException("Cannot read image file", [[errorPtr localizedDescription] UTF8String]);
            }
        }
        
        //
        // Create image source
        //
        auto imageSource = cf::ObjectPtr<CGImageSourceRef>::created(CGImageSourceCreateWithData(static_cast<CFDataRef>(imageData), nullptr));
        if (CGImageSourceGetCount(imageSource.get()) < 1) {
            auto imageSourceStatus = CGImageSourceGetStatus(imageSource.get());
            throw SimpleException((boost::format("Cannot load image from file (status = %d)") % imageSourceStatus).str());
        }
        
        //
        // Compute the SHA-1 message digest of the raw file data, convert it to a hex string, and copy it to fileHash
        //
        {
            boost::uuids::detail::sha1 sha;
            sha.process_bytes([imageData bytes], [imageData length]);
            
            constexpr std::size_t digestSize = 5;
            unsigned int digest[digestSize];
            sha.get_digest(digest);
            
            std::ostringstream os;
            os.fill('0');
            os << std::hex;
            for (int i = 0; i < digestSize; i++) {
                os << std::setw(2 * sizeof(unsigned int)) << digest[i];
            }
            
            fileHash = os.str();
        }
        
        return imageSource;
    }
}


void ImageStimulus::describeComponent(ComponentInfo &info) {
    BasicTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/image_file");
    
    info.addParameter(PATH);
    info.addParameter(ANNOUNCE_LOAD, "YES");
}


ImageStimulus::ImageStimulus(const ParameterValueMap &parameters) :
    BasicTransformStimulus(parameters),
    path(variableOrText(parameters[PATH])),
    announceLoad(parameters[ANNOUNCE_LOAD]),
    width(0),
    height(0),
    aspectRatio(1.0)
{ }


void ImageStimulus::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
    // Evaluate and store file path
    filename = pathFromParameterValue(path).string();
    
    //
    // Obtain image data.  We do this here, rather than in prepare(), so that we don't block the stimulus display
    // thread by holding the OpenGL context lock.
    //
    {
        //
        // The procedure used here is adapted from section "Creating a Texture from a Quartz Image Source" in
        // "OpenGL Programming Guide for Mac":
        //
        // https://developer.apple.com/library/content/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_texturedata/opengl_texturedata.html#//apple_ref/doc/uid/TP40001987-CH407-SW31
        //
        
        auto imageSource = loadImageFile(filename, fileHash, announceLoad);
        auto image = cf::ObjectPtr<CGImageRef>::created(CGImageSourceCreateImageAtIndex(imageSource.get(), 0, nullptr));
        
        // If we're not using color management, replace the image's color space with an appropriate
        // device-dependent one, so that its color values are not altered in any way
        if (!display->getUseColorManagement()) {
            cf::ObjectPtr<CGColorSpaceRef> imageColorSpace;
            
            switch (CGColorSpaceGetModel(CGImageGetColorSpace(image.get()))) {
                case kCGColorSpaceModelMonochrome:
                    imageColorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateDeviceGray());
                    break;
                case kCGColorSpaceModelRGB:
                    imageColorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateDeviceRGB());
                    break;
                case kCGColorSpaceModelCMYK:
                    imageColorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateDeviceCMYK());
                    break;
                default:
                    throw SimpleException("Image uses an unsupported color model");
            }
            
            image = cf::ObjectPtr<CGImageRef>::created(CGImageCreateCopyWithColorSpace(image.get(),
                                                                                       imageColorSpace.get()));
        }
        
        width = CGImageGetWidth(image.get());
        height = CGImageGetHeight(image.get());
        aspectRatio = double(width) / double(height);
        data.reset(new std::uint32_t[width * height]);
        
        // If we're using color management, convert the image to sRGB.  Otherwise, draw it in the
        // device-dependent RGB color space.
        auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(display->getUseColorManagement() ?
                                                                  CGColorSpaceCreateWithName(kCGColorSpaceSRGB) :
                                                                  CGColorSpaceCreateDeviceRGB());
        
        
        vImage_Buffer buf = {
            .data = data.get(),
            .height = height,
            .width = width,
            .rowBytes = width * 4
        };
        
        vImage_CGImageFormat format = {
            .bitsPerComponent = 8,
            .bitsPerPixel = 32,
            .colorSpace = colorSpace.get(),
#if MWORKS_OPENGL_ES
            .bitmapInfo = kCGImageAlphaFirst | kCGBitmapByteOrder32Little,
#else
            .bitmapInfo = kCGImageAlphaFirst | kCGBitmapByteOrder32Host,
#endif
            .version = 0,
            .decode = nullptr,
            .renderingIntent = kCGRenderingIntentPerceptual
        };
        
        auto error = vImageBuffer_InitWithCGImage(&buf,
                                                  &format,
                                                  nullptr,
                                                  image.get(),
                                                  kvImageNoAllocate);
        if (kvImageNoError != error) {
            throw SimpleException(boost::format("Cannot extract image file data (error = %d)") % error);
        }
    }
    
    BasicTransformStimulus::load(display);
}


Datum ImageStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = BasicTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_IMAGE);
    announceData.addElement(STIM_FILENAME,filename);  
    announceData.addElement(STIM_FILE_HASH,fileHash);  
    
    return announceData;
}


gl::Shader ImageStimulus::getVertexShader() const {
    static const std::string source
    (R"(
     uniform mat4 mvpMatrix;
     in vec4 vertexPosition;
     in vec2 texCoords;
     out vec2 varyingTexCoords;
     
     void main() {
         gl_Position = mvpMatrix * vertexPosition;
         varyingTexCoords = texCoords;
     }
     )");
    
    return gl::createShader(GL_VERTEX_SHADER, source);
}


gl::Shader ImageStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     uniform float alpha;
     uniform sampler2D imageTexture;
     in vec2 varyingTexCoords;
     out vec4 fragColor;
     
     void main() {
         vec4 imageColor = texture(imageTexture, varyingTexCoords);
         fragColor.rgb = imageColor.rgb;
         fragColor.a = alpha * imageColor.a;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


auto ImageStimulus::getVertexPositions() const -> VertexPositionArray {
    if (fullscreen) {
        return BasicTransformStimulus::getVertexPositions();
    }
    return getVertexPositions(aspectRatio);
}


void ImageStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::prepare(display);
    
    alphaUniformLocation = glGetUniformLocation(program, "alpha");
    glUniform1i(glGetUniformLocation(program, "imageTexture"), 0);
    
    //
    // Create texture
    //
    {
        glGenTextures(1, &texture);
        gl::TextureBinding<GL_TEXTURE_2D> textureBinding(texture);
        
        gl::resetPixelStorageUnpackParameters();
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     (display->getUseColorManagement() ? GL_SRGB8_ALPHA8 : GL_RGBA8),
                     width,
                     height,
                     0,
#if MWORKS_OPENGL_ES
                     GL_BGRA_EXT,
                     GL_UNSIGNED_BYTE,
#else
                     GL_BGRA,
                     GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
                     data.get());
        
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    // We're done with the image data, so release it to free up memory
    data.reset();
    
    if (announceLoad) {
        mprintf("Image loaded into texture_map %u", texture);
    }
    
    glGenBuffers(1, &texCoordsBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(texCoordsBuffer);
    // Draw the texture flipped vertically to match OpenGL's coordinate system (origin in
    // the bottom left corner)
    VertexPositionArray texCoords {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void ImageStimulus::destroy(const boost::shared_ptr<StimulusDisplay> &display) {
    glDeleteBuffers(1, &texCoordsBuffer);
    glDeleteTextures(1, &texture);
    
    if (announceLoad) {
        mprintf("Image unloaded from texture_map %u", texture);
    }
    
    BasicTransformStimulus::destroy(display);
}


void ImageStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::preDraw(display);
    
    glUniform1f(alphaUniformLocation, current_alpha);
    
    glBindTexture(GL_TEXTURE_2D, texture);
}


void ImageStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glBindTexture(GL_TEXTURE_2D, 0);
    
    BasicTransformStimulus::postDraw(display);
}


END_NAMESPACE_MW























