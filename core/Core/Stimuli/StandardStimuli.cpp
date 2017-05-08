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

#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>

#include <boost/uuid/sha1.hpp>

#include "CFObjectPtr.h"
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


const std::string BasicTransformStimulus::X_SIZE("x_size");
const std::string BasicTransformStimulus::Y_SIZE("y_size");
const std::string BasicTransformStimulus::X_POSITION("x_position");
const std::string BasicTransformStimulus::Y_POSITION("y_position");
const std::string BasicTransformStimulus::ROTATION("rotation");
const std::string BasicTransformStimulus::ALPHA_MULTIPLIER("alpha_multiplier");


void BasicTransformStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.addParameter(X_SIZE);
    info.addParameter(Y_SIZE);
    info.addParameter(X_POSITION, "0.0");
    info.addParameter(Y_POSITION, "0.0");
    info.addParameter(ROTATION, "0.0");
    info.addParameter(ALPHA_MULTIPLIER, "1.0");
}


BasicTransformStimulus::BasicTransformStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    xoffset(registerVariable(parameters[X_POSITION])),
    yoffset(registerVariable(parameters[Y_POSITION])),
    xscale(registerVariable(parameters[X_SIZE])),
    yscale(registerVariable(parameters[Y_SIZE])),
    rotation(registerVariable(parameters[ROTATION])),
    alpha_multiplier(registerVariable(parameters[ALPHA_MULTIPLIER]))
{ }


BasicTransformStimulus::BasicTransformStimulus(const std::string &_tag,
                                               const shared_ptr<Variable> &_xoffset,
                                               const shared_ptr<Variable> &_yoffset,
                                               const shared_ptr<Variable> &_xscale,
                                               const shared_ptr<Variable> &_yscale,
                                               const shared_ptr<Variable> &_rot,
                                               const shared_ptr<Variable> &_alpha) :
    Stimulus(_tag),
    xoffset(registerVariable(_xoffset)),
    yoffset(registerVariable(_yoffset)),
    xscale(registerVariable(_xscale)),
    yscale(registerVariable(_yscale)),
    rotation(registerVariable(_rot)),
    alpha_multiplier(registerVariable(_alpha))
{ }


void BasicTransformStimulus::load(shared_ptr<StimulusDisplay> display) {
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


void BasicTransformStimulus::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    destroy(display);
    
    glDeleteBuffers(1, &vertexPositionBuffer);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteProgram(program);
    
    Stimulus::unload(display);
}


void BasicTransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    current_posx = *xoffset;
    current_posy = *yoffset;
    current_sizex = *xscale;
    current_sizey = *yscale;
    current_rot = *rotation;
    current_alpha = *alpha_multiplier;
    
    gl::ProgramUsage programUsage(program);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    auto currentMVPMatrix = getCurrentMVPMatrix(display->getProjectionMatrix());
    glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, currentMVPMatrix.m);
    
    preDraw(display);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
    
    postDraw(display);
    
    last_posx = current_posx;
    last_posy = current_posy;
    last_sizex = current_sizex;
    last_sizey = current_sizey;
    last_rot = current_rot;
    last_alpha = current_alpha;
}


Datum BasicTransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE,STIM_TYPE_BASICTRANSFORM);
    announceData.addElement(STIM_POSX,last_posx);  
    announceData.addElement(STIM_POSY,last_posy);  
    announceData.addElement(STIM_SIZEX,last_sizex);  
    announceData.addElement(STIM_SIZEY,last_sizey);  
    announceData.addElement(STIM_ROT,last_rot);  
    announceData.addElement(STIM_ALPHA,last_alpha);
    
    return announceData;
}


auto BasicTransformStimulus::getVertexPositions() const -> VertexPositionArray {
    return VertexPositionArray {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
}


GLKMatrix4 BasicTransformStimulus::getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const {
    auto currentMVPMatrix = GLKMatrix4Translate(projectionMatrix, current_posx, current_posy, 0.0);
    currentMVPMatrix = GLKMatrix4Rotate(currentMVPMatrix, GLKMathDegreesToRadians(current_rot), 0.0, 0.0, 1.0);
    currentMVPMatrix = GLKMatrix4Scale(currentMVPMatrix, current_sizex, current_sizey, 1.0);
    return GLKMatrix4Translate(currentMVPMatrix, -0.5, -0.5, 0.0);
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
    
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void ColoredTransformStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glDisable(GL_BLEND);
    BasicTransformStimulus::postDraw(display);
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
         if (dist > radius) {
             discard;
         }
         float alpha = 1.0 - smoothstep(radius - delta, radius, dist);
         fragColor.rgb = color.rgb;
         fragColor.a = alpha * color.a;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


const std::string ImageStimulus::PATH("path");


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


void ImageStimulus::describeComponent(ComponentInfo &info) {
    BasicTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/image_file");
    info.addParameter(PATH);
}


ImageStimulus::ImageStimulus(const ParameterValueMap &parameters) :
    BasicTransformStimulus(parameters),
    path(variableOrText(parameters[PATH])),
    aspectRatio(1.0)
{ }


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
         // imageColor.rgb has been pre-multiplied by imageColor.a, so we multiply only
         // by the user-specified alpha here
         fragColor = alpha * imageColor;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


auto ImageStimulus::getVertexPositions() const -> VertexPositionArray {
    return getVertexPositions(aspectRatio);
}


void ImageStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::prepare(display);
    
    alphaUniformLocation = glGetUniformLocation(program, "alpha");
    glUniform1i(glGetUniformLocation(program, "imageTexture"), 0);
    
    // Evaluate and store file path
    filename = pathFromParameterValue(path).string();
    
    mprintf("Loading image %s", filename.c_str());
    
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
        // Create texture
        //
        {
            //
            // The procedure used here is adapted from section "Creating a Texture from a Quartz Image Source" in
            // "OpenGL Programming Guide for Mac":
            //
            // https://developer.apple.com/library/content/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_texturedata/opengl_texturedata.html#//apple_ref/doc/uid/TP40001987-CH407-SW31
            //
            
            auto imageSource = cf::ObjectPtr<CGImageSourceRef>::created(CGImageSourceCreateWithData(static_cast<CFDataRef>(imageData), nullptr));
            if (CGImageSourceGetCount(imageSource.get()) < 1) {
                auto imageSourceStatus = CGImageSourceGetStatus(imageSource.get());
                throw SimpleException((boost::format("Cannot load image from file (status = %d)") % imageSourceStatus).str());
            }
            
            auto image = cf::ObjectPtr<CGImageRef>::created(CGImageSourceCreateImageAtIndex(imageSource.get(), 0, nullptr));
            auto width = CGImageGetWidth(image.get());
            auto height = CGImageGetHeight(image.get());
            aspectRatio = double(width) / double(height);
            
            auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateDeviceRGB());
            
            auto context = cf::ObjectPtr<CGContextRef>::created(CGBitmapContextCreate(nullptr,
                                                                                      width,
                                                                                      height,
                                                                                      8,
                                                                                      width * 4,
                                                                                      colorSpace.get(),
#if MWORKS_OPENGL_ES
                                                                                      kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big));
#else
                                                                                      kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));
#endif
            
            // Flip the context's coordinate system (so that the origin is in the bottom left corner, as in OpenGL)
            CGContextTranslateCTM(context.get(), 0, height);
            CGContextScaleCTM(context.get(), 1, -1);
            
            CGContextSetBlendMode(context.get(), kCGBlendModeCopy);
            
            CGContextDrawImage(context.get(), CGRectMake(0, 0, width, height), image.get());
            
            glGenTextures(1, &texture);
            gl::TextureBinding<GL_TEXTURE_2D> textureBinding(texture);
            
#if !MWORKS_OPENGL_ES
            glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
            glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
#endif
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RGBA8,
                         width,
                         height,
                         0,
#if MWORKS_OPENGL_ES
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
#else
                         GL_BGRA,
                         GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
                         CGBitmapContextGetData(context.get()));
            
            glGenerateMipmap(GL_TEXTURE_2D);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    }
    
    mprintf("Image loaded into texture_map %u", texture);
    
    glGenBuffers(1, &texCoordsBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(texCoordsBuffer);
    VertexPositionArray texCoords {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void ImageStimulus::destroy(const boost::shared_ptr<StimulusDisplay> &display) {
    glDeleteBuffers(1, &texCoordsBuffer);
    glDeleteTextures(1, &texture);
    mprintf("Image unloaded from texture_map %u", texture);
    
    BasicTransformStimulus::destroy(display);
}


void ImageStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::preDraw(display);
    
    glUniform1f(alphaUniformLocation, current_alpha);
    
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, // Source RGB values are pre-multiplied by source alpha
                GL_ONE_MINUS_SRC_ALPHA);
}


void ImageStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    BasicTransformStimulus::postDraw(display);
}


END_NAMESPACE_MW























