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


inline cf::StringPtr createCFString(const std::string &bytesUTF8) {
    return cf::StringPtr::created(CFStringCreateWithBytes(kCFAllocatorDefault,
                                                          reinterpret_cast<const UInt8 *>(bytesUTF8.data()),
                                                          bytesUTF8.size(),
                                                          kCFStringEncodingUTF8,
                                                          false));
}


CTTextAlignment textAlignmentFromName(const std::string &name) {
    if (name == "left") {
        return kCTTextAlignmentLeft;
    } else if (name == "right") {
        return kCTTextAlignmentRight;
    } else if (name == "center") {
        return kCTTextAlignmentCenter;
    }
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Invalid text alignment: \"%s\"", name.c_str());
    return kCTTextAlignmentLeft;
}


END_NAMESPACE()


const std::string TextStimulus::TEXT("text");
const std::string TextStimulus::FONT_NAME("font_name");
const std::string TextStimulus::FONT_SIZE("font_size");
const std::string TextStimulus::TEXT_ALIGNMENT("text_alignment");


void TextStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/text");
    
    info.addParameter(TEXT);
    info.addParameter(FONT_NAME);
    info.addParameter(FONT_SIZE);
    info.addParameter(TEXT_ALIGNMENT, "left");
}


TextStimulus::TextStimulus(const ParameterValueMap &parameters) :
    ColoredTransformStimulus(parameters),
    text(registerVariable(variableOrText(parameters[TEXT]))),
    fontName(registerVariable(variableOrText(parameters[FONT_NAME]))),
    fontSize(registerVariable(parameters[FONT_SIZE])),
    textAlignment(registerVariable(variableOrText(parameters[TEXT_ALIGNMENT]))),
    viewportWidth(0),
    viewportHeight(0),
    pixelsPerDegree(0.0),
    pointsPerPixel(0.0)
{ }


Datum TextStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "text");
    announceData.addElement(TEXT, lastText);
    announceData.addElement(FONT_NAME, lastFontName);
    announceData.addElement(FONT_SIZE, lastFontSize);
    announceData.addElement(TEXT_ALIGNMENT, lastTextAlignment);
    
    return announceData;
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
    
    double xMin, xMax, yMin, yMax;
    display->getDisplayBounds(xMin, xMax, yMin, yMax);
    
    __block CGSize displaySizeInPoints;
    dispatch_sync(dispatch_get_main_queue(), ^{
        // If there's only a mirror window, getFullscreenView will return its view,
        // so there's no need to call getMirrorView
        auto glcm = boost::dynamic_pointer_cast<AppleOpenGLContextManager>(OpenGLContextManager::instance());
        auto displayView = glcm->getFullscreenView();
        displaySizeInPoints = displayView.frame.size;
    });
    
    display->getCurrentViewportSize(viewportWidth, viewportHeight);
    pixelsPerDegree = double(viewportWidth) / (xMax - xMin);
    pointsPerPixel = displaySizeInPoints.width / double(viewportWidth);
    
    glGenTextures(1, &texture);
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(texture);
    {
        // Attempt to provide initial data for the text texture, in hopes that this will force
        // the driver and/or GPU to allocate memory for it now, at load time, rather than on
        // first use
        
        float sizeX = 0.0;
        float sizeY = 0.0;
        getCurrentSize(sizeX, sizeY);
        
        std::size_t bitmapWidth = 0;
        std::size_t bitmapHeight = 0;
        computeBitmapDimensions(sizeX, sizeY, bitmapWidth, bitmapHeight);
        const auto numPixels = bitmapWidth * bitmapHeight;
        
        if (numPixels > 0) {
            std::vector<std::uint8_t> data(4 * numPixels, 0);
            gl::resetPixelStorageUnpackParameters();
            
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         (display->getUseColorManagement() ? GL_SRGB8_ALPHA8 : GL_RGBA8),
                         bitmapWidth,
                         bitmapHeight,
                         0,
#if MWORKS_OPENGL_ES
                         GL_BGRA_EXT,
                         GL_UNSIGNED_BYTE,
#else
                         GL_BGRA,
                         GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
                         data.data());
        }
    }
    
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
    currentTextAlignment = textAlignment->getValue().getString();
    
    bindTexture(display);
}


void TextStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glBindTexture(GL_TEXTURE_2D, 0);
    
    lastText = currentText;
    lastFontName = currentFontName;
    lastFontSize = currentFontSize;
    lastTextAlignment = currentTextAlignment;
    
    ColoredTransformStimulus::postDraw(display);
}


void TextStimulus::computeBitmapDimensions(float width,
                                           float height,
                                           std::size_t &bitmapWidth,
                                           std::size_t &bitmapHeight) const
{
    bitmapWidth = (fullscreen ? viewportWidth : (width * pixelsPerDegree));
    bitmapHeight = (fullscreen ? viewportHeight : (height * pixelsPerDegree));
}


void TextStimulus::bindTexture(const boost::shared_ptr<StimulusDisplay> &display) {
    if ((fullscreen || (current_sizex == last_sizex && current_sizey == last_sizey)) &&
        currentText == lastText &&
        currentFontName == lastFontName &&
        currentFontSize == lastFontSize &&
        currentTextAlignment == lastTextAlignment)
    {
        // No relevant parameters have changed since we last generated the texture, so use
        // the existing one
        glBindTexture(GL_TEXTURE_2D, texture);
        return;
    }
    
    // Create a bitmap context
    std::size_t bitmapWidth = 0;
    std::size_t bitmapHeight = 0;
    computeBitmapDimensions(current_sizex, current_sizey, bitmapWidth, bitmapHeight);
    auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(display->getUseColorManagement() ?
                                                              CGColorSpaceCreateWithName(kCGColorSpaceSRGB) :
                                                              CGColorSpaceCreateDeviceRGB());
    auto context = cf::ObjectPtr<CGContextRef>::created(CGBitmapContextCreate(nullptr,
                                                                              bitmapWidth,
                                                                              bitmapHeight,
                                                                              8,
                                                                              bitmapWidth * 4,
                                                                              colorSpace.get(),
#if MWORKS_OPENGL_ES
                                                                              kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little));
#else
                                                                              kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));
#endif
    
    // Flip the context's coordinate system (so that the origin is in the bottom left corner, as in OpenGL) and
    // convert it from pixels to points
    CGContextTranslateCTM(context.get(), 0, bitmapHeight);
    CGContextScaleCTM(context.get(), 1.0 / pointsPerPixel, -1.0 / pointsPerPixel);
    
    // Create the text string
    auto attrString = cf::ObjectPtr<CFMutableAttributedStringRef>::created(CFAttributedStringCreateMutable(kCFAllocatorDefault, 0));
    CFAttributedStringReplaceString(attrString.get(), CFRangeMake(0, 0), createCFString(currentText).get());
    const auto fullRange = CFRangeMake(0, CFAttributedStringGetLength(attrString.get()));
    
    // Set the color to white and fully opaque.  (The user-specified color and alpha multiplier are applied
    // in the fragment shader.)
    const std::array<CGFloat, 4> colorComponents { 1.0, 1.0, 1.0, 1.0 };
    auto color = cf::ObjectPtr<CGColorRef>::created(CGColorCreate(colorSpace.get(), colorComponents.data()));
    CFAttributedStringSetAttribute(attrString.get(),
                                   fullRange,
                                   kCTForegroundColorAttributeName,
                                   color.get());
    
    // Set the font
    auto font = cf::ObjectPtr<CTFontRef>::created(CTFontCreateWithName(createCFString(currentFontName).get(),
                                                                       currentFontSize,
                                                                       nullptr));
    CFAttributedStringSetAttribute(attrString.get(), fullRange, kCTFontAttributeName, font.get());
    
    // Set the text alignment
    const CTTextAlignment alignment = textAlignmentFromName(currentTextAlignment);
    const std::array<CTParagraphStyleSetting, 1> paragraphStyleSettings {
        { kCTParagraphStyleSpecifierAlignment, sizeof(alignment), &alignment }
    };
    auto paragraphStyle = cf::ObjectPtr<CTParagraphStyleRef>::created(CTParagraphStyleCreate(paragraphStyleSettings.data(),
                                                                                             paragraphStyleSettings.size()));
    CFAttributedStringSetAttribute(attrString.get(), fullRange, kCTParagraphStyleAttributeName, paragraphStyle.get());

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
    
    glBindTexture(GL_TEXTURE_2D, texture);
    
    gl::resetPixelStorageUnpackParameters();
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 (display->getUseColorManagement() ? GL_SRGB8_ALPHA8 : GL_RGBA8),
                 bitmapWidth,
                 bitmapHeight,
                 0,
#if MWORKS_OPENGL_ES
                 GL_BGRA_EXT,
                 GL_UNSIGNED_BYTE,
#else
                 GL_BGRA,
                 GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
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



























