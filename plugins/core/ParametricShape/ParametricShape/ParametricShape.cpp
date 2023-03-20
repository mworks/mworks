//
//  ParametricShape.cpp
//  ParametricShape
//
//  Created by Christopher Stawarz on 10/6/21.
//

#include "ParametricShape.hpp"


BEGIN_NAMESPACE_MW


const std::string ParametricShape::VERTICES("vertices");
const std::string ParametricShape::SPLINE_RESOLUTION("spline_resolution");
const std::string ParametricShape::VERTEX_COORD_CENTER_X("vertex_coord_center_x");
const std::string ParametricShape::VERTEX_COORD_CENTER_Y("vertex_coord_center_y");
const std::string ParametricShape::VERTEX_COORD_RANGE("vertex_coord_range");
const std::string ParametricShape::MARGIN_PIXELS("margin_pixels");
const std::string ParametricShape::USE_ANTIALIASING("use_antialiasing");
const std::string ParametricShape::MAX_SIZE_X("max_size_x");
const std::string ParametricShape::MAX_SIZE_Y("max_size_y");


void ParametricShape::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/parametric_shape");
    
    info.addParameter(VERTICES);
    info.addParameter(SPLINE_RESOLUTION, "50");
    info.addParameter(VERTEX_COORD_CENTER_X, false);
    info.addParameter(VERTEX_COORD_CENTER_Y, false);
    info.addParameter(VERTEX_COORD_RANGE, false);
    info.addParameter(MARGIN_PIXELS, "0");
    info.addParameter(USE_ANTIALIASING, "YES");
    info.addParameter(MAX_SIZE_X, false);
    info.addParameter(MAX_SIZE_Y, false);
}


ParametricShape::ParametricShape(const ParameterValueMap &parameters) :
    ColoredTransformStimulus(parameters),
    vertices(parameters[VERTICES]),
    splineResolution(parameters[SPLINE_RESOLUTION]),
    vertexCoordCenterX(optionalVariable(parameters[VERTEX_COORD_CENTER_X])),
    vertexCoordCenterY(optionalVariable(parameters[VERTEX_COORD_CENTER_Y])),
    vertexCoordRange(optionalVariable(parameters[VERTEX_COORD_RANGE])),
    marginPixels(registerVariable(parameters[MARGIN_PIXELS])),
    useAntialiasing(registerVariable(parameters[USE_ANTIALIASING])),
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
    announceData.addElement(SPLINE_RESOLUTION, currentSplineResolution);
    
    if (vertexCoordCenterX) {
        announceData.addElement(VERTEX_COORD_CENTER_X, currentVertexCoordCenterX);
    }
    if (vertexCoordCenterY) {
        announceData.addElement(VERTEX_COORD_CENTER_Y, currentVertexCoordCenterY);
    }
    if (vertexCoordRange) {
        announceData.addElement(VERTEX_COORD_RANGE, currentVertexCoordRange);
    }
    
    // Only announce non-zero margins
    if (currentMarginPixels > 0.0) {
        announceData.addElement(MARGIN_PIXELS, currentMarginPixels);
    }
    
    // Anti-aliasing should always be enabled in normal circumstances, so only announce
    // when it's disabled
    if (!currentUseAntialiasing) {
        announceData.addElement(USE_ANTIALIASING, false);
    }
    
    return announceData;
}


bool ParametricShape::validateVertices(Datum &value) {
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
        }
    }
    
    // Ensure that the first and last vertices are identical, because the curve approximation
    // algorithm assumes that they are
    if (list.back() != list.front()) {
        value.addElement(list.front());
    }
    
    return true;
}


static inline simd::double2 double2FromDatum(const Datum &value) {
    auto &list = value.getList();
    return simd::make_double2(list.at(0).getFloat(), list.at(1).getFloat());
}


std::vector<CGPoint> ParametricShape::generatePoints(const Datum::list_value_type &vertexCoords,
                                                     std::size_t numSamples)
{
    std::vector<CGPoint> points;
    
    if (numSamples == 0) {
        // Return the vertices unmodified
        for (auto &coord : vertexCoords) {
            auto point = double2FromDatum(coord);
            points.emplace_back(CGPointMake(point.x, point.y));
        }
        return points;
    }
    
    std::vector<simd::packed::double2> controlPoints;
    controlPoints.push_back(double2FromDatum(vertexCoords.at(vertexCoords.size() - 2)));
    for (auto &coord : vertexCoords) {
        controlPoints.push_back(double2FromDatum(coord));
    }
    controlPoints.push_back(double2FromDatum(vertexCoords.at(1)));
    
    //
    // Use a cubic B-spline to approximate the curve.  Note that this is *not* interpolation, as the computed
    // curve is not constrained to pass through the specified vertices (which is why we refer to them as
    // "control points", as they determine the shape of the curve but don't necessarily form part of it).
    //
    //   https://math.stackexchange.com/questions/699113/what-is-the-relationship-between-cubic-b-splines-and-cubic-splines
    //   http://www2.cs.uregina.ca/~anima/408/Notes/Interpolation/UniformBSpline.htm
    //
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
                                  "Vertices must be provided as a list of 2 or more sub-lists, with each sub-list "
                                  "containing two numbers");
        }
        currentVertices = std::move(value);
    }
    
    //
    // Get spline resolution
    //
    
    currentSplineResolution = splineResolution->getValue().getInteger();
    if (currentSplineResolution < 0) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Spline resolution must be greater than or equal to zero");
    }
    
    //
    // Create path
    //
    {
        auto mutablePath = cf::ObjectPtr<CGMutablePathRef>::created(CGPathCreateMutable());
        auto points = generatePoints(currentVertices.getList(), currentSplineResolution);
        CGPathAddLines(mutablePath.get(), nullptr, points.data(), points.size());
        CGPathCloseSubpath(mutablePath.get());
        path = cf::ObjectPtr<CGPathRef>::created(CGPathCreateCopy(mutablePath.get()));
    }
    
    //
    // Determine vertex coordinate parameters
    //
    {
        auto pathBoundingBox = CGPathGetPathBoundingBox(path.get());
        
        if (vertexCoordCenterX) {
            currentVertexCoordCenterX = vertexCoordCenterX->getValue().getFloat();
        } else {
            currentVertexCoordCenterX = CGRectGetMidX(pathBoundingBox);
        }
        
        if (vertexCoordCenterY) {
            currentVertexCoordCenterY = vertexCoordCenterY->getValue().getFloat();
        } else {
            currentVertexCoordCenterY = CGRectGetMidY(pathBoundingBox);
        }
        
        if (vertexCoordRange) {
            currentVertexCoordRange = vertexCoordRange->getValue().getFloat();
        } else {
            auto halfWidth = std::max(std::abs(currentVertexCoordCenterX - CGRectGetMinX(pathBoundingBox)),
                                      std::abs(currentVertexCoordCenterX - CGRectGetMaxX(pathBoundingBox)));
            auto halfHeight = std::max(std::abs(currentVertexCoordCenterY - CGRectGetMinY(pathBoundingBox)),
                                       std::abs(currentVertexCoordCenterY - CGRectGetMaxY(pathBoundingBox)));
            currentVertexCoordRange = 2.0 * std::max(halfWidth, halfHeight);
        }
        if (currentVertexCoordRange <= 0.0) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Vertex coordinate range must be greater than zero");
        }
        
        auto vertexCoordBox = CGRectMake(currentVertexCoordCenterX - currentVertexCoordRange / 2.0,
                                         currentVertexCoordCenterY - currentVertexCoordRange / 2.0,
                                         currentVertexCoordRange,
                                         currentVertexCoordRange);
        if (!CGRectContainsRect(vertexCoordBox, pathBoundingBox)) {
            mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                     "Vertex coordinate grid for parametric shape stimulus \"%s\" does not contain all points "
                     "in the shape: coordinate grid covers (%g, %g) to (%g, %g), but shape points range from "
                     "(%g, %g) to (%g, %g)",
                     getTag().c_str(),
                     CGRectGetMinX(vertexCoordBox),
                     CGRectGetMinY(vertexCoordBox),
                     CGRectGetMaxX(vertexCoordBox),
                     CGRectGetMaxY(vertexCoordBox),
                     CGRectGetMinX(pathBoundingBox),
                     CGRectGetMinY(pathBoundingBox),
                     CGRectGetMaxX(pathBoundingBox),
                     CGRectGetMaxY(pathBoundingBox));
        }
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
    // Determine viewport size and compute conversion from degrees to pixels
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
    
    // Allow antialiasing, but disable it initially
    CGContextSetAllowsAntialiasing(context.get(), true);
    CGContextSetShouldAntialias(context.get(), false);
    
    // Flip the context's coordinate system, so that the origin is in the upper left corner,
    // as it is for Metal textures
    CGContextTranslateCTM(context.get(), 0, textureHeight);
    CGContextScaleCTM(context.get(), 1.0, -1.0);
    
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
    currentMarginPixels = std::max(0.0, marginPixels->getValue().getFloat());  // Ignore negative values
    currentUseAntialiasing = useAntialiasing->getValue().getBool();
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
    lastMarginPixels = currentMarginPixels;
    lastUseAntialiasing = currentUseAntialiasing;
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
        currentHeightPixels == lastHeightPixels &&
        currentMarginPixels == lastMarginPixels &&
        currentUseAntialiasing == lastUseAntialiasing)
    {
        // No relevant parameters have changed since we last generated the texture, so use
        // the existing one
        return;
    }
    
    //
    // NOTE: Although it doesn't seem to be documented anywhere, the base, pixel-aligned coordinate
    // system for a CGBitmapContext appears to range from 0 to num_pixels-1 in each dimension, with
    // each whole-numbered coordinate presumably marking the center of a pixel.  Therefore, we need
    // to subtract one from the width and height in pixels when we scale the shape's vertex
    // coordinates.
    //
    
    // Clear the relevant portion of the context.  Clear one pixel past the current width and height
    // (i.e. don't subtract one from currentWidthPixels and currentHeightPixels) to ensure that old
    // data doesn't bleed in to the current region.
    CGContextClearRect(context.get(), CGRectMake(0, 0, currentWidthPixels, currentHeightPixels));
    
    // Draw the shape in the context
    CGContextSaveGState(context.get());
    CGContextSetShouldAntialias(context.get(), currentUseAntialiasing);
    CGContextTranslateCTM(context.get(), currentMarginPixels, currentMarginPixels);
    CGContextScaleCTM(context.get(),
                      ((currentWidthPixels - 1) - 2.0 * currentMarginPixels) / currentVertexCoordRange,
                      ((currentHeightPixels - 1) - 2.0 * currentMarginPixels) / currentVertexCoordRange);
    CGContextTranslateCTM(context.get(), currentVertexCoordRange / 2.0, currentVertexCoordRange / 2.0);
    CGContextScaleCTM(context.get(), 1.0, -1.0);
    CGContextTranslateCTM(context.get(), -currentVertexCoordCenterX, -currentVertexCoordCenterY);
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
