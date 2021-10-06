//
//  ParametricShape.cpp
//  ParametricShape
//
//  Created by Christopher Stawarz on 10/6/21.
//

#include "ParametricShape.hpp"


BEGIN_NAMESPACE_MW


const std::string ParametricShape::VERTICES("vertices");
const std::string ParametricShape::MAX_SIZE_X("max_size_x");
const std::string ParametricShape::MAX_SIZE_Y("max_size_y");


void ParametricShape::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/parametric_shape");
    
    info.addParameter(VERTICES);
    info.addParameter(MAX_SIZE_X, false);
    info.addParameter(MAX_SIZE_Y, false);
}


ParametricShape::ParametricShape(const ParameterValueMap &parameters) :
    ColoredTransformStimulus(parameters),
    vertices(registerVariable(parameters[VERTICES])),
    maxSizeX(optionalVariable(parameters[MAX_SIZE_X])),
    maxSizeY(optionalVariable(parameters[MAX_SIZE_Y])),
    texturePool(nil),
    currentTexture(nil)
{ }


ParametricShape::~ParametricShape() {
    @autoreleasepool {
        currentTexture = nil;
        texturePool = nil;
    }
}


Datum ParametricShape::getCurrentAnnounceDrawData() {
    auto announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "parametric_shape");
    announceData.addElement(VERTICES, currentVertices);
    
    return announceData;
}


bool ParametricShape::validateVertices(const Datum &value) {
    if (!value.isList()) {
        return false;
    }
    
    auto &list = value.getList();
    if (list.size() < 2) {
        return false;
    }
    
    for (auto &listItem : list) {
        if (!listItem.isList()) {
            return false;
        }
        
        auto &subList = listItem.getList();
        if (subList.size() != 2) {
            return false;
        }
        
        for (auto &subListItem : subList) {
            if (!subListItem.isNumber()) {
                return false;
            }
            
            const auto number = subListItem.getFloat();
            if (number < vertexCoordMin || number > vertexCoordMax) {
                return false;
            }
        }
    }
    
    return true;
}


static inline simd::double2 double2FromDatum(const Datum &value) {
    auto &list = value.getList();
    return simd::make_double2(list.at(0).getFloat(), list.at(1).getFloat());
}


std::vector<CGPoint> ParametricShape::generatePoints(const Datum::list_value_type &vertexCoords)  {
    std::vector<simd::packed::double2> controlPoints;
    controlPoints.push_back(double2FromDatum(vertexCoords.at(vertexCoords.size() - 2)));
    for (auto &coord : vertexCoords) {
        controlPoints.push_back(double2FromDatum(coord));
    }
    controlPoints.push_back(double2FromDatum(vertexCoords.at(1)));
    
    constexpr std::size_t numSamples = 50;
    std::vector<CGPoint> points;
    for (std::size_t i = 0; i < controlPoints.size() - 3; i++) {
        for (std::size_t sample = 0; sample < numSamples; sample++) {
            const auto ip = double(sample) / double(numSamples);
            auto point = simd::make_double2(0.0, 0.0);
            point += controlPoints.at(i+0) * (-ip*ip*ip + 3.0*ip*ip - 3.0*ip + 1.0);
            point += controlPoints.at(i+1) * (3.0*ip*ip*ip - 6.0*ip*ip + 4.0);
            point += controlPoints.at(i+2) * (-3.0*ip*ip*ip + 3.0*ip*ip + 3.0*ip + 1.0);
            point += controlPoints.at(i+3) * (ip*ip*ip);
            point /= 6.0;
            points.emplace_back(CGPointMake(point.x, point.y));
        }
    }
    
    return points;
}


void ParametricShape::loadMetal(MetalDisplay &display) {
    ColoredTransformStimulus::loadMetal(display);
    
    //
    // Obtain vertices
    //
    {
        auto value = vertices->getValue();
        if (!validateVertices(value)) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Vertices must be provided as a list of 2 or more sub-lists, with "
                                                "each sub-list containing two numbers in the range [%1%, %2%]")
                                  % vertexCoordMin
                                  % vertexCoordMax);
        }
        currentVertices = std::move(value);
    }
    
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
        __block CGSize displaySizeInPixels;
        dispatch_sync(dispatch_get_main_queue(), ^{
            // If there's only a mirror window, getMainView will return its view,
            // so there's no need to call getMirrorView
            displaySizeInPixels = display.getMainView().drawableSize;
        });
        
        viewportWidth = displaySizeInPixels.width;
        viewportHeight = displaySizeInPixels.height;
        
        double xMin, xMax, yMin, yMax;
        display.getDisplayBounds(xMin, xMax, yMin, yMax);
        pixelsPerDegree = double(viewportWidth) / (xMax - xMin);
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
    
    // Flip the context's coordinate system, so that the origin is in the upper left corner,
    // as it is for Metal textures
    CGContextTranslateCTM(context.get(), 0, textureHeight);
    CGContextScaleCTM(context.get(), 1.0, -1.0);
    
    //
    // Create path
    //
    {
        auto mutablePath = cf::ObjectPtr<CGMutablePathRef>::created(CGPathCreateMutable());
        auto points = generatePoints(currentVertices.getList());
        CGPathAddLines(mutablePath.get(), nullptr, points.data(), points.size());
        CGPathCloseSubpath(mutablePath.get());
        path = cf::ObjectPtr<CGPathRef>::created(CGPathCreateCopy(mutablePath.get()));
    }
    
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


void ParametricShape::unloadMetal(MetalDisplay &display) {
    currentTexture = nil;
    texturePool = nil;
    path.reset();
    context.reset();
    textureData.reset();
    
    ColoredTransformStimulus::unloadMetal(display);
}


void ParametricShape::drawMetal(MetalDisplay &display) {
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
    updateTexture(display);
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    setCurrentMVPMatrix(display, renderCommandEncoder, 0);
    {
        auto texCoordScale = simd::make_float2(currentWidthPixels - 1, currentHeightPixels - 1);
        setVertexBytes(renderCommandEncoder, texCoordScale, 1);
    }
    
    [renderCommandEncoder setFragmentTexture:currentTexture atIndex:0];
    setCurrentColor(renderCommandEncoder, 0);
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
    
    lastWidthPixels = currentWidthPixels;
    lastHeightPixels = currentHeightPixels;
}


void ParametricShape::computeTextureDimensions(double widthDegrees,
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


void ParametricShape::updateTexture(MetalDisplay &display) {
    if (currentTexture &&
        currentWidthPixels == lastWidthPixels &&
        currentHeightPixels == lastHeightPixels)
    {
        // No relevant parameters have changed since we last generated the texture, so use
        // the existing one
        return;
    }
    
    // Clear the relevant portion of the context
    CGContextClearRect(context.get(), CGRectMake(0, 0, currentWidthPixels, currentHeightPixels));
    
    // Draw the shape in the context
    CGContextSaveGState(context.get());
    CGContextScaleCTM(context.get(), currentWidthPixels / vertexCoordScale, currentHeightPixels / vertexCoordScale);
    CGContextTranslateCTM(context.get(), vertexCoordScale / 2.0, vertexCoordScale / 2.0);
    CGContextScaleCTM(context.get(), 1.0, -1.0);
    CGContextAddPath(context.get(), path.get());
    CGContextFillPath(context.get());
    CGContextRestoreGState(context.get());
    
    // Copy the texture data to the next-available texture from the pool
    currentTexture = [texturePool acquireWithCommandBuffer:display.getCurrentMetalCommandBuffer()];
    [currentTexture replaceRegion:MTLRegionMake2D(0, 0, currentWidthPixels, currentHeightPixels)
                      mipmapLevel:0
                        withBytes:textureData.get()
                      bytesPerRow:textureBytesPerRow];
}


END_NAMESPACE_MW
