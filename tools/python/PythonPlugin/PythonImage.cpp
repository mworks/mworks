//
//  PythonImage.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 10/1/20.
//  Copyright © 2020 MWorks Project. All rights reserved.
//

#include "PythonImage.hpp"

#include "ObjectPtr.h"
#include "PythonException.h"


BEGIN_NAMESPACE_MW_PYTHON


const std::string PythonImage::PIXEL_BUFFER_FORMAT("pixel_buffer_format");
const std::string PythonImage::PIXEL_BUFFER_WIDTH("pixel_buffer_width");
const std::string PythonImage::PIXEL_BUFFER_HEIGHT("pixel_buffer_height");
const std::string PythonImage::PIXEL_BUFFER_EXPR("pixel_buffer_expr");
const std::string PythonImage::ELAPSED_TIME("elapsed_time");


void PythonImage::describeComponent(ComponentInfo &info) {
    PythonImageBase::describeComponent(info);
    
    info.setSignature("stimulus/python_image");
    
    info.addParameter(PIXEL_BUFFER_FORMAT);
    info.addParameter(PIXEL_BUFFER_WIDTH);
    info.addParameter(PIXEL_BUFFER_HEIGHT);
    info.addParameter(PIXEL_BUFFER_EXPR);
    info.addParameter(ELAPSED_TIME, false);
    info.addParameter(AUTOPLAY, "YES");  // Change autoplay's default to YES
}


PythonImage::PythonImage(const ParameterValueMap &parameters) :
    PythonImageBase(parameters),
    pixelBufferFormat(variableOrText(parameters[PIXEL_BUFFER_FORMAT])),
    pixelBufferWidth(parameters[PIXEL_BUFFER_WIDTH]),
    pixelBufferHeight(parameters[PIXEL_BUFFER_HEIGHT]),
    pixelBufferExpr(parameters[PIXEL_BUFFER_EXPR].str()),
    elapsedTime(optionalVariable(parameters[ELAPSED_TIME])),
    pixelBufferExprEvaluator(pixelBufferExpr, true),
    width(0),
    height(0),
    expectedBytes(0),
    texturePoolSemaphore(nil),
    textureBytesPerRow(0),
    texturePool(nil),
    currentTextureIndex(0)
{ }


PythonImage::~PythonImage() {
    @autoreleasepool {
        texturePool = nil;
        texturePoolSemaphore = nil;
    }
}


Datum PythonImage::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    auto announceData = PythonImageBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "python_image");
    announceData.addElement(PIXEL_BUFFER_FORMAT, formatName);
    announceData.addElement(PIXEL_BUFFER_WIDTH, width);
    announceData.addElement(PIXEL_BUFFER_HEIGHT, height);
    announceData.addElement(PIXEL_BUFFER_EXPR, pixelBufferExpr);
    
    return announceData;
}


void PythonImage::loadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    BaseImageStimulus::loadMetal(display);
    
    //
    // Determine pixel buffer format and dimensions
    //
    {
        formatName = pixelBufferFormat->getValue().getString();
        
        std::size_t bytesPerPixel = 0;
        if (formatName == "RGB8" || formatName == "sRGB8") {
            format = Format::RGB8;
            bytesPerPixel = 3;
        } else if (formatName == "RGBA8" || formatName == "sRGBA8") {
            format = Format::RGBA8;
            bytesPerPixel = 4;
        } else if (formatName == "RGBA16F") {
            format = Format::RGBA16F;
            bytesPerPixel = 8;
        } else {
            throw SimpleException(boost::format("Invalid pixel buffer format: \"%1%\"") % formatName);
        }
        
        width = pixelBufferWidth->getValue().getInteger();
        height = pixelBufferHeight->getValue().getInteger();
        if (width < 1 || height < 1) {
            throw SimpleException("Pixel buffer width and height must be greater than or equal to one");
        }
        
        expectedBytes = bytesPerPixel * width * height;
    }
    
    //
    // Create texture pool.  This allows us to employ triple buffering, as recommended in the
    // Metal Best Practices Guide:
    // https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/TripleBuffering.html
    //
    {
        texturePoolSemaphore = dispatch_semaphore_create(texturePoolSize);
        
        MTLPixelFormat pixelFormat = MTLPixelFormatInvalid;
        switch (format) {
            case Format::RGB8:
                // Metal doesn't support RGB-only textures, so we need to use an RGBA pixel format
                // and add the alpha channel ourselves
                [[clang::fallthrough]];
            case Format::RGBA8:
                pixelFormat = (formatName.front() == 's' ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm);
                textureBytesPerRow = width * 4;
                break;
            case Format::RGBA16F:
                pixelFormat = MTLPixelFormatRGBA16Float;
                textureBytesPerRow = width * 8;
                break;
        }
        
        auto textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                                    width:width
                                                                                   height:height
                                                                                mipmapped:NO];
        // Keep the default value of storageMode: MTLStorageModeManaged on macOS, MTLStorageModeShared on iOS
        
        NSMutableArray<id<MTLTexture>> *mutableTexturePool = [NSMutableArray arrayWithCapacity:texturePoolSize];
        while (mutableTexturePool.count < texturePoolSize) {
            [mutableTexturePool addObject:[display.getMetalDevice() newTextureWithDescriptor:textureDescriptor]];
        }
        texturePool = [mutableTexturePool copy];
        
        currentTextureIndex = texturePoolSize - 1;
    }
    
    // Allocate a buffer to use when adding an alpha channel to RGB-only data
    if (Format::RGB8 == format) {
        rgbaData.reset(new std::uint8_t[height * textureBytesPerRow]);
    }
}


void PythonImage::unloadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    rgbaData.reset();
    texturePool = nil;
    texturePoolSemaphore = nil;
    
    BaseImageStimulus::unloadMetal(display);
}


bool PythonImage::prepareCurrentTexture(MetalDisplay &display) {
    if (elapsedTime) {
        elapsedTime->setValue(Datum(getElapsedTime()));
    }
    
    //
    // Obtain and validate the current pixel buffer
    //
    
    PythonEvaluator::EvalState es;
    auto pixelBuffer = ObjectPtr::owned(pixelBufferExprEvaluator.eval());
    
    if (!pixelBuffer) {
        PythonException::logError("Pixel buffer expression evaluation failed");
        return false;
    }
    if (pixelBuffer.isNone()) {
        // None means the texture doesn't need to be updated
        return true;
    }
    if (!PyObject_CheckBuffer(pixelBuffer.get())) {
        merror(M_PLUGIN_MESSAGE_DOMAIN, "Pixel buffer object does not support the Python buffer interface");
        return false;
    }
    
    Py_buffer view;
    std::memset(&view, 0, sizeof(view));
    if (-1 == PyObject_GetBuffer(pixelBuffer.get(), &view, PyBUF_SIMPLE)) {
        PythonException::logError("Cannot access pixel buffer data");
        return false;
    }
    BOOST_SCOPE_EXIT(&view) {
        PyBuffer_Release(&view);
    } BOOST_SCOPE_EXIT_END
    
    if (!(view.buf && view.len > 0)) {
        merror(M_PLUGIN_MESSAGE_DOMAIN, "Pixel buffer object contains no data");
        return false;
    }
    if (view.len != expectedBytes) {
        merror(M_PLUGIN_MESSAGE_DOMAIN,
               "Pixel buffer data contains wrong number of bytes (expected %lu, got %ld)",
               expectedBytes,
               view.len);
        return false;
    }
    
    //
    // Prepare the pixel buffer data for writing to a texture.  If the data already
    // includes an alpha channel, use it as is.  Otherwise, create a copy of the data
    // with an alpha channel added, and use the copy.
    //
    
    const void *textureData = nullptr;
    
    if (format != Format::RGB8) {
        textureData = view.buf;
    } else {
        vImage_Buffer rgbSrc;
        rgbSrc.data = view.buf;
        rgbSrc.height = height;
        rgbSrc.width = width;
        rgbSrc.rowBytes = width * 3;
        
        vImage_Buffer rgbaDst;
        rgbaDst.data = rgbaData.get();
        rgbaDst.height = height;
        rgbaDst.width = width;
        rgbaDst.rowBytes = width * 4;
        
        auto error = vImageConvert_RGB888toRGBA8888(&rgbSrc,
                                                    nullptr,
                                                    std::numeric_limits<Pixel_8>::max(),  // Set alpha to one
                                                    &rgbaDst,
                                                    false,
                                                    kvImageNoFlags);
        if (kvImageNoError != error) {
            merror(M_PLUGIN_MESSAGE_DOMAIN, "Cannot add alpha channel to RGB data (error = %ld)", error);
            return false;
        }
        
        textureData = rgbaData.get();
    }
    
    //
    // Copy the texture data to the next-available texture from the pool
    //
    
    currentTextureIndex = (currentTextureIndex + 1) % texturePoolSize;
    
    // Wait until the GPU is done rendering the current contents of the texture
    // before writing new data to it
    dispatch_semaphore_wait(texturePoolSemaphore, DISPATCH_TIME_FOREVER);
    [texturePool[currentTextureIndex] replaceRegion:MTLRegionMake2D(0, 0, width, height)
                                        mipmapLevel:0
                                          withBytes:textureData
                                        bytesPerRow:textureBytesPerRow];
    
    // Register a completed handler so we know when the current texture is once
    // again available to overwrite
    {
        __weak dispatch_semaphore_t semaphore = texturePoolSemaphore;
        [display.getCurrentMetalCommandBuffer() addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
            dispatch_semaphore_signal(semaphore);
        }];
    }
    
    return true;
}


double PythonImage::getCurrentAspectRatio() const {
    return double(width) / double(height);
}


id<MTLTexture> PythonImage::getCurrentTexture() const {
    return texturePool[currentTextureIndex];
}


void PythonImage::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    BaseImageStimulus::draw(display);
}


END_NAMESPACE_MW_PYTHON
