//
//  TextStimulus.cpp
//  TextStimulus
//
//  Created by Christopher Stawarz on 9/13/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "TextStimulus.hpp"


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


VariablePtr variableOrText(const ParameterValue &param) {
    try {
        return VariablePtr(param);
    } catch (const SimpleException &) {
        return boost::make_shared<ConstantVariable>(param.str());
    }
}


inline cf::StringPtr createCFString(const std::string &bytesUTF8) {
    return cf::StringPtr::created(CFStringCreateWithBytes(kCFAllocatorDefault,
                                                          reinterpret_cast<const UInt8 *>(bytesUTF8.data()),
                                                          bytesUTF8.size(),
                                                          kCFStringEncodingUTF8,
                                                          false));
}


END_NAMESPACE()


const std::string TextStimulus::TEXT("text");
const std::string TextStimulus::FONT_NAME("font_name");
const std::string TextStimulus::FONT_SIZE("font_size");


void TextStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/text");
    
    info.addParameter(TEXT);
    info.addParameter(FONT_NAME);
    info.addParameter(FONT_SIZE);
}


TextStimulus::TextStimulus(const ParameterValueMap &parameters) :
    ColoredTransformStimulus(parameters),
    text(registerVariable(variableOrText(parameters[TEXT]))),
    fontName(registerVariable(variableOrText(parameters[FONT_NAME]))),
    fontSize(registerVariable(parameters[FONT_SIZE])),
    pixelsPerDegree(0.0),
    pointsPerPixel(0.0)
{ }


Datum TextStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "text");
    announceData.addElement(TEXT, lastText);
    announceData.addElement(FONT_NAME, lastFontName);
    announceData.addElement(FONT_SIZE, lastFontSize);
    
    return std::move(announceData);
}


gl::Shader TextStimulus::getVertexShader() const {
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


gl::Shader TextStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     uniform vec4 color;
     uniform sampler2D textTexture;
     in vec2 varyingTexCoords;
     out vec4 fragColor;
     
     void main() {
         fragColor = color * texture(textTexture, varyingTexCoords);
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


void TextStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    ColoredTransformStimulus::prepare(display);
    
    GLdouble xMin, xMax, yMin, yMax;
    display->getDisplayBounds(xMin, xMax, yMin, yMax);
    
    auto glcm = OpenGLContextManager::instance();
    NSOpenGLView *displayView = glcm->getFullscreenView();
    if (!displayView) {
        displayView = glcm->getMirrorView();
    }
    
    __block NSSize displaySizeInPoints;
    dispatch_sync(dispatch_get_main_queue(), ^{
        displaySizeInPoints = displayView.frame.size;
    });
    
    GLint width, height;
    display->getCurrentViewportSize(width, height);
    pixelsPerDegree = double(width) / (xMax - xMin);
    pointsPerPixel = displaySizeInPoints.width / double(width);
    texture = 0;
    
    glUniform1i(glGetUniformLocation(program, "textTexture"), 0);
    
    glGenBuffers(1, &texCoordsBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(texCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void TextStimulus::destroy(const boost::shared_ptr<StimulusDisplay> &display) {
    glDeleteBuffers(1, &texCoordsBuffer);
    glDeleteTextures(1, &texture);
    
    ColoredTransformStimulus::destroy(display);
}


void TextStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    ColoredTransformStimulus::preDraw(display);
    
    currentText = text->getValue().getString();
    currentFontName = fontName->getValue().getString();
    currentFontSize = fontSize->getValue().getFloat();
    
    bindTexture();
}


void TextStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glBindTexture(GL_TEXTURE_2D, 0);
    
    lastText = currentText;
    lastFontName = currentFontName;
    lastFontSize = currentFontSize;
    
    ColoredTransformStimulus::postDraw(display);
}


void TextStimulus::bindTexture() {
    if (texture &&
        current_sizex == last_sizex &&
        current_sizey == last_sizey &&
        currentText == lastText &&
        currentFontName == lastFontName &&
        currentFontSize == lastFontSize)
    {
        // No relevant parameters have changed since we last generated the texture, so use
        // the existing one
        glBindTexture(GL_TEXTURE_2D, texture);
        return;
    }
    
    // Create a bitmap context
    const std::size_t bitmapWidth = current_sizex * pixelsPerDegree;
    const std::size_t bitmapHeight = current_sizey * pixelsPerDegree;
    auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateDeviceRGB());
    auto context = cf::ObjectPtr<CGContextRef>::created(CGBitmapContextCreate(nullptr,
                                                                              bitmapWidth,
                                                                              bitmapHeight,
                                                                              8,
                                                                              bitmapWidth * 4,
                                                                              colorSpace.get(),
                                                                              kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));
    
    // Flip the context's coordinate system (so that the origin is in the bottom left corner, as in OpenGL) and
    // convert it from pixels to points
    CGContextTranslateCTM(context.get(), 0, bitmapHeight);
    CGContextScaleCTM(context.get(), pointsPerPixel, -pointsPerPixel);
    
    // Create the text string with the requested font applied
    auto attrString = cf::ObjectPtr<CFMutableAttributedStringRef>::created(CFAttributedStringCreateMutable(kCFAllocatorDefault, 0));
    CFAttributedStringReplaceString(attrString.get(), CFRangeMake(0, 0), createCFString(currentText).get());
    const auto fullRange = CFRangeMake(0, CFAttributedStringGetLength(attrString.get()));
    CFAttributedStringSetAttribute(attrString.get(),
                                   fullRange,
                                   kCTForegroundColorAttributeName,
                                   CGColorGetConstantColor(kCGColorWhite));
    auto font = cf::ObjectPtr<CTFontRef>::created(CTFontCreateWithName(createCFString(currentFontName).get(),
                                                                       currentFontSize,
                                                                       nullptr));
    CFAttributedStringSetAttribute(attrString.get(), fullRange, kCTFontAttributeName, font.get());
    
    // Generate a Core Text frame
    auto framesetter = cf::ObjectPtr<CTFramesetterRef>::created(CTFramesetterCreateWithAttributedString(attrString.get()));
    auto path = cf::ObjectPtr<CGPathRef>::created(CGPathCreateWithRect(CGRectMake(0,
                                                                                  0,
                                                                                  bitmapWidth * pointsPerPixel,
                                                                                  bitmapHeight * pointsPerPixel),
                                                                       nullptr));
    auto frame = cf::ObjectPtr<CTFrameRef>::created(CTFramesetterCreateFrame(framesetter.get(),
                                                                             CFRangeMake(0, 0),
                                                                             path.get(),
                                                                             nullptr));
    
    // Draw the frame in the context
    CGContextSetTextMatrix(context.get(), CGAffineTransformIdentity);
    CTFrameDraw(frame.get(), context.get());
    if (CTFrameGetVisibleStringRange(frame.get()).length != fullRange.length) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Text for stimulus \"%s\" does not fit within the specified region and will be truncated",
                 getTag().c_str());
    }
    
    //
    // Create an OpenGL texture from the bitmap context
    //
    
    if (!texture) {
        glGenTextures(1, &texture);
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 bitmapWidth,
                 bitmapHeight,
                 0,
                 GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV,
                 CGBitmapContextGetData(context.get()));
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


const TextStimulus::VertexPositionArray TextStimulus::texCoords {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};


END_NAMESPACE_MW



























