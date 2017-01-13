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
    fontSize(registerVariable(parameters[FONT_SIZE]))
{ }


void TextStimulus::load(boost::shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
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
    
    pixelsPerDegree.clear();
    pointsPerPixel.clear();
    texture.clear();
    
    for (int i = 0; i < display->getNContexts(); i++) {
        OpenGLContextLock ctxLock = display->setCurrent(i);
        GLint width, height;
        display->getCurrentViewportSize(width, height);
        pixelsPerDegree.push_back(double(width) / (xMax - xMin));
        pointsPerPixel.push_back(displaySizeInPoints.width / double(width));
        texture.push_back(0);
    }
    
    loaded = true;
}


void TextStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    for (int i = 0; i < display->getNContexts(); i++) {
        OpenGLContextLock ctxLock = display->setCurrent(i);
        glDeleteTextures(1, &(texture.at(i)));
    }
    
    loaded = false;
}


void TextStimulus::drawInUnitSquare(boost::shared_ptr<StimulusDisplay> display) {
    currentText = text->getValue().getString();
    currentFontName = fontName->getValue().getString();
    currentFontSize = fontSize->getValue().getFloat();
    
    glEnable(GL_TEXTURE_2D);
    bindTexture(display->getCurrentContextIndex());
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(current_r, current_g, current_b, current_alpha);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    
    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    
    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.0);
    
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, 1.0, 0.0);
    
    glEnd();
    
    glDisable(GL_BLEND);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    
    lastText = currentText;
    lastFontName = currentFontName;
    lastFontSize = currentFontSize;
}


Datum TextStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "text");
    announceData.addElement(TEXT, lastText);
    announceData.addElement(FONT_NAME, lastFontName);
    announceData.addElement(FONT_SIZE, lastFontSize);
    
    return std::move(announceData);
}


void TextStimulus::bindTexture(int currentContextIndex) {
    auto &currentTexture = texture.at(currentContextIndex);
    
    if (currentTexture &&
        current_sizex == last_sizex &&
        current_sizey == last_sizey &&
        currentText == lastText &&
        currentFontName == lastFontName &&
        currentFontSize == lastFontSize)
    {
        // No relevant parameters have changed since we last generated the texture, so use
        // the existing one
        glBindTexture(GL_TEXTURE_2D, currentTexture);
        return;
    }
    
    const auto currentPixelsPerDegree = pixelsPerDegree.at(currentContextIndex);
    const auto currentPointsPerPixel = pointsPerPixel.at(currentContextIndex);
    
    // Create a bitmap context
    const std::size_t bitmapWidth = current_sizex * currentPixelsPerDegree;
    const std::size_t bitmapHeight = current_sizey * currentPixelsPerDegree;
    auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(CGColorSpaceCreateDeviceRGB());
    auto context = cf::ObjectPtr<CGContextRef>::created(CGBitmapContextCreate(nullptr,
                                                                              bitmapWidth,
                                                                              bitmapHeight,
                                                                              8,
                                                                              bitmapWidth * 4,
                                                                              colorSpace.get(),
                                                                              kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host));
    
    // Flip the context's coordinate system (so that the origin is in the top left corner, as in OpenGL) and
    // convert it from pixels to points
    CGContextTranslateCTM(context.get(), 0, bitmapHeight);
    CGContextScaleCTM(context.get(), currentPointsPerPixel, -currentPointsPerPixel);
    
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
                                                                                  bitmapWidth * currentPointsPerPixel,
                                                                                  bitmapHeight * currentPointsPerPixel),
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
    
    if (!currentTexture) {
        glGenTextures(1, &currentTexture);
    }
    glBindTexture(GL_TEXTURE_2D, currentTexture);
    
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
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
    
    glPopClientAttrib();
}


END_NAMESPACE_MW



























