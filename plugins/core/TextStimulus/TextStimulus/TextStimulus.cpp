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
const std::string TextStimulus::MAX_SIZE_X("max_size_x");
const std::string TextStimulus::MAX_SIZE_Y("max_size_y");


void TextStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/text");
    
    info.addParameter(TEXT);
    info.addParameter(FONT_NAME);
    info.addParameter(FONT_SIZE);
    info.addParameter(TEXT_ALIGNMENT, "left");
    info.addParameter(MAX_SIZE_X, false);
    info.addParameter(MAX_SIZE_Y, false);
}


TextStimulus::TextStimulus(const ParameterValueMap &parameters) :
    ColoredTransformStimulus(parameters),
    text(registerVariable(variableOrText(parameters[TEXT]))),
    fontName(registerVariable(variableOrText(parameters[FONT_NAME]))),
    fontSize(registerVariable(parameters[FONT_SIZE])),
    textAlignment(registerVariable(variableOrText(parameters[TEXT_ALIGNMENT]))),
    maxSizeX(optionalVariable(parameters[MAX_SIZE_X])),
    maxSizeY(optionalVariable(parameters[MAX_SIZE_Y])),
    texturePool(nil),
    currentTexture(nil)
{ }


TextStimulus::~TextStimulus() {
    @autoreleasepool {
        currentTexture = nil;
        texturePool = nil;
    }
}


Datum TextStimulus::getCurrentAnnounceDrawData() {
    auto announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "text");
    announceData.addElement(TEXT, currentText);
    announceData.addElement(FONT_NAME, currentFontName);
    announceData.addElement(FONT_SIZE, currentFontSize);
    announceData.addElement(TEXT_ALIGNMENT, currentTextAlignment);
    
    return announceData;
}


void TextStimulus::loadMetal(MetalDisplay &display) {
    ColoredTransformStimulus::loadMetal(display);
    
    //
    // Determine maximum size
    //
    
    if (!fullscreen) {
        float defaultMaxSizeX = 0.0;
        float defaultMaxSizeY = 0.0;
        
        // Don't evaluate x_size and y_size unless we need them
        if (!(maxSizeX && maxSizeY)) {
            getCurrentSize(defaultMaxSizeX, defaultMaxSizeY);
        }
        
        if (maxSizeX) {
            currentMaxSizeX = maxSizeX->getValue().getFloat();
        } else {
            currentMaxSizeX = defaultMaxSizeX;
        }
        if (currentMaxSizeX <= 0.0) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Maximum horizontal size must be greater than zero");
        }
        
        if (maxSizeY) {
            currentMaxSizeY = maxSizeY->getValue().getFloat();
        } else {
            currentMaxSizeY = defaultMaxSizeY;
        }
        if (currentMaxSizeY <= 0.0) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Maximum vertical size must be greater than zero");
        }
    }
    
    //
    // Determine viewport size and compute conversions from degrees to pixels
    // and points to pixels
    //
    {
        __block CGSize displaySizeInPoints;
        __block CGSize displaySizeInPixels;
        dispatch_sync(dispatch_get_main_queue(), ^{
            // If there's only a mirror window, getMainView will return its view,
            // so there's no need to call getMirrorView
            auto displayView = display.getMainView();
            displaySizeInPoints = displayView.frame.size;
            displaySizeInPixels = displayView.drawableSize;
        });
        
        viewportWidth = displaySizeInPixels.width;
        viewportHeight = displaySizeInPixels.height;
        
        double xMin, xMax, yMin, yMax;
        display.getDisplayBounds(xMin, xMax, yMin, yMax);
        pixelsPerDegree = double(viewportWidth) / (xMax - xMin);
        pixelsPerPoint = double(viewportWidth) / displaySizeInPoints.width;
    }
    
    //
    // Determine texture dimensions and allocate CPU-side texture data
    //
    
    computeTextureDimensions(currentMaxSizeX, currentMaxSizeY, textureWidth, textureHeight);
    textureBytesPerRow = textureWidth;  // Texture contains only alpha values
    textureData.reset(new std::uint8_t[textureBytesPerRow * textureHeight]);
    
    //
    // Create bitmap context
    //
    
    context = cf::ObjectPtr<CGContextRef>::created(CGBitmapContextCreate(textureData.get(),
                                                                         textureWidth,
                                                                         textureHeight,
                                                                         8,
                                                                         textureBytesPerRow,
                                                                         nullptr,  // OK for alpha-only bitmap
                                                                         kCGImageAlphaOnly));
    
    // Enable antialiasing
    CGContextSetAllowsAntialiasing(context.get(), true);
    CGContextSetShouldAntialias(context.get(), true);
    
    // Set the text matrix
    CGContextSetTextMatrix(context.get(), CGAffineTransformIdentity);
    
    //
    // Create texture pool
    //
    {
        auto textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
                                                                                    width:textureWidth
                                                                                   height:textureHeight
                                                                                mipmapped:NO];
        // Keep the default value of storageMode: MTLStorageModeManaged on macOS, MTLStorageModeShared on iOS
        
        texturePool = [MWKTripleBufferedMTLResource textureWithDevice:display.getMetalDevice()
                                                           descriptor:textureDescriptor];
    }
    
    //
    // Create render pipeline state
    //
    {
        auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
        auto vertexFunction = loadShaderFunction(library, "vertexShader");
        auto fragmentFunction = loadShaderFunction(library, "fragmentShader");
        auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
        renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
    }
}


void TextStimulus::unloadMetal(MetalDisplay &display) {
    currentTexture = nil;
    texturePool = nil;
    context.reset();
    textureData.reset();
    
    ColoredTransformStimulus::unloadMetal(display);
}


void TextStimulus::drawMetal(MetalDisplay &display) {
    ColoredTransformStimulus::drawMetal(display);
    
    if (!fullscreen) {
        if (current_sizex > currentMaxSizeX) {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Current horizontal size of stimulus \"%s\" (%g) exceeds maximum (%g).  To resolve this issue, "
                   "set %s to an appropriate value.",
                   getTag().c_str(),
                   current_sizex,
                   currentMaxSizeX,
                   MAX_SIZE_X.c_str());
            return;
        }
        if (current_sizey > currentMaxSizeY) {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Current vertical size of stimulus \"%s\" (%g) exceeds maximum (%g).  To resolve this issue, "
                   "set %s to an appropriate value.",
                   getTag().c_str(),
                   current_sizey,
                   currentMaxSizeY,
                   MAX_SIZE_Y.c_str());
            return;
        }
    }
    
    computeTextureDimensions(current_sizex, current_sizey, currentWidthPixels, currentHeightPixels);
    currentText = text->getValue().getString();
    currentFontName = fontName->getValue().getString();
    currentFontSize = fontSize->getValue().getFloat();
    currentTextAlignment = textAlignment->getValue().getString();
    
    updateTexture(display);
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    setCurrentMVPMatrix(display, renderCommandEncoder, 0);
    {
        auto texCoordScale = simd::make_float2(currentWidthPixels, currentHeightPixels);
        setVertexBytes(renderCommandEncoder, texCoordScale, 1);
    }
    
    [renderCommandEncoder setFragmentTexture:currentTexture atIndex:0];
    setCurrentColor(renderCommandEncoder, 0);
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
    
    lastWidthPixels = currentWidthPixels;
    lastHeightPixels = currentHeightPixels;
    lastText = currentText;
    lastFontName = currentFontName;
    lastFontSize = currentFontSize;
    lastTextAlignment = currentTextAlignment;
}


void TextStimulus::computeTextureDimensions(double widthDegrees,
                                            double heightDegrees,
                                            std::size_t &widthPixels,
                                            std::size_t &heightPixels) const
{
    if (fullscreen) {
        widthPixels = viewportWidth;
        heightPixels = viewportHeight;
    } else {
        // Replace negative width or height with 0, and ensure that the texture
        // contains at least one pixel
        widthPixels = std::max(1.0, pixelsPerDegree * std::max(0.0, widthDegrees));
        heightPixels = std::max(1.0, pixelsPerDegree * std::max(0.0, heightDegrees));
    }
}


void TextStimulus::updateTexture(MetalDisplay &display) {
    if (currentTexture &&
        currentWidthPixels == lastWidthPixels &&
        currentHeightPixels == lastHeightPixels &&
        currentText == lastText &&
        currentFontName == lastFontName &&
        currentFontSize == lastFontSize &&
        currentTextAlignment == lastTextAlignment)
    {
        // No relevant parameters have changed since we last generated the texture, so use
        // the existing one
        return;
    }
    
    //
    // Clear the relevant portion of the context
    //
    // NOTE: By default, Core Graphics contexts place the origin in the lower left corner, whereas
    // the origin for Metal textures is in the upper left.  To simplify the texture coordinates used
    // by Metal, we want to draw in the upper-left corner of the texture.  This would be easier to
    // do if we just flipped the coordinate system of the CGBitmapContext.  Unfortunately, Core Text
    // resists such flips and insists on making "up" be the direction of increasing y, meaning that
    // text rendered into a y-flipped context is drawn with y flipped.  Therefore, we just live with
    // the slight annoyance of defining our drawing rectangle relative to Core Graphics' default,
    // origin-at-lower-left coordinate system.
    //
    const auto rect = CGRectMake(0,
                                 textureHeight - currentHeightPixels,
                                 currentWidthPixels,
                                 currentHeightPixels);
    CGContextClearRect(context.get(), rect);
    
    // Create the text string
    auto attrString = cf::ObjectPtr<CFMutableAttributedStringRef>::created(CFAttributedStringCreateMutable(kCFAllocatorDefault, 0));
    CFAttributedStringReplaceString(attrString.get(), CFRangeMake(0, 0), createCFString(currentText).get());
    const auto fullRange = CFRangeMake(0, CFAttributedStringGetLength(attrString.get()));
    
    // Set the font, converting size from points to pixels
    auto font = cf::ObjectPtr<CTFontRef>::created(CTFontCreateWithName(createCFString(currentFontName).get(),
                                                                       currentFontSize * pixelsPerPoint,
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
    auto path = cf::ObjectPtr<CGPathRef>::created(CGPathCreateWithRect(rect, nullptr));
    auto frame = cf::ObjectPtr<CTFrameRef>::created(CTFramesetterCreateFrame(framesetter.get(),
                                                                             CFRangeMake(0, 0),
                                                                             path.get(),
                                                                             nullptr));
    
    // Draw the frame in the context
    CTFrameDraw(frame.get(), context.get());
    if (CTFrameGetVisibleStringRange(frame.get()).length != fullRange.length) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Text for stimulus \"%s\" does not fit within the specified region and will be truncated",
                 getTag().c_str());
    }
    
    // Copy the texture data to the next-available texture from the pool
    currentTexture = [texturePool acquireWithCommandBuffer:display.getCurrentMetalCommandBuffer()];
    [currentTexture replaceRegion:MTLRegionMake2D(0, 0, currentWidthPixels, currentHeightPixels)
                      mipmapLevel:0
                        withBytes:textureData.get()
                      bytesPerRow:textureBytesPerRow];
}


END_NAMESPACE_MW
