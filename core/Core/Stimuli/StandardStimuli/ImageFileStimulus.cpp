//
//  ImageFileStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/20/21.
//

#include "ImageFileStimulus.hpp"

#include <Accelerate/Accelerate.h>

#include "FileHash.hpp"


BEGIN_NAMESPACE_MW


const std::string ImageFileStimulus::PATH("path");
const std::string ImageFileStimulus::ANNOUNCE_LOAD("announce_load");


cf::ObjectPtr<CGImageSourceRef> ImageFileStimulus::loadImageFile(const std::string &filename,
                                                                 std::string &fileHash,
                                                                 bool announce)
{
    @autoreleasepool {
        if (announce) {
            mprintf("Loading image %s", filename.c_str());
        }
        
        //
        // Create image source
        //
        auto imageURL = [NSURL fileURLWithPath:@(filename.c_str()) isDirectory:NO];
        auto imageSource = cf::ObjectPtr<CGImageSourceRef>::created(CGImageSourceCreateWithURL(static_cast<CFURLRef>(imageURL), nullptr));
        if (CGImageSourceGetCount(imageSource.get()) < 1) {
            throw SimpleException(boost::format("Cannot load image from file (status = %d)") %
                                  CGImageSourceGetStatus(imageSource.get()));
        }
        
        //
        // Compute file hash
        //
        fileHash = computeFileHash(filename);
        
        return imageSource;
    }
}


void ImageFileStimulus::describeComponent(ComponentInfo &info) {
    BaseImageStimulus::describeComponent(info);
    
    info.setSignature("stimulus/image_file");
    
    info.addParameter(PATH);
    info.addParameter(ANNOUNCE_LOAD, "YES");
}


ImageFileStimulus::ImageFileStimulus(const ParameterValueMap &parameters) :
    BaseImageStimulus(parameters),
    path(variableOrText(parameters[PATH])),
    announceLoad(parameters[ANNOUNCE_LOAD]),
    width(0),
    height(0),
    texture(nil)
{ }


ImageFileStimulus::~ImageFileStimulus() {
    @autoreleasepool {
        texture = nil;
    }
}


Datum ImageFileStimulus::getCurrentAnnounceDrawData() {
    auto announceData = BaseImageStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, STIM_TYPE_IMAGE);
    announceData.addElement(STIM_FILENAME, filename);
    announceData.addElement(STIM_FILE_HASH, fileHash);
    
    return announceData;
}


void ImageFileStimulus::loadMetal(MetalDisplay &display) {
    BaseImageStimulus::loadMetal(display);
    
    // Evaluate and store file path
    filename = pathFromParameterValue(path).string();
    
    //
    // Obtain image data
    //
    
    std::vector<std::uint8_t> data;
    
    {
        auto imageSource = loadImageFile(filename, fileHash, announceLoad);
        auto image = cf::ObjectPtr<CGImageRef>::created(CGImageSourceCreateImageAtIndex(imageSource.get(), 0, nullptr));
        
        // If we're not using color management, replace the image's color space with an appropriate
        // device-dependent one, so that its color values are not altered in any way
        if (!display.getUseColorManagement()) {
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
        data.resize(width * height * 4);
        
        // If we're using color management, convert the image to sRGB.  Otherwise, draw it in the
        // device-dependent RGB color space.
        auto colorSpace = cf::ObjectPtr<CGColorSpaceRef>::created(display.getUseColorManagement() ?
                                                                  CGColorSpaceCreateWithName(kCGColorSpaceSRGB) :
                                                                  CGColorSpaceCreateDeviceRGB());
        
        
        vImage_Buffer buf = {
            .data = data.data(),
            .height = height,
            .width = width,
            .rowBytes = width * 4
        };
        
        vImage_CGImageFormat format = {
            .bitsPerComponent = 8,
            .bitsPerPixel = 32,
            .colorSpace = colorSpace.get(),
            .bitmapInfo = CGBitmapInfo(kCGImageAlphaFirst) | kCGBitmapByteOrder32Little,
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
    
    //
    // Prepare texture
    //
    
    {
        // Create shared buffer
        auto buffer = [display.getMetalDevice() newBufferWithBytes:data.data()
                                                            length:data.size()
                                                           options:MTLResourceStorageModeShared];
        
        // Create private texture
        auto pixelFormat = (display.getUseColorManagement() ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm);
        auto textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                                    width:width
                                                                                   height:height
                                                                                mipmapped:NO];
        textureDescriptor.storageMode = MTLStorageModePrivate;
        texture = [display.getMetalDevice() newTextureWithDescriptor:textureDescriptor];
        
        // Copy data from shared buffer to private texture
        auto commandBuffer = [display.getMetalCommandQueue() commandBuffer];
        auto blitCommandEncoder = [commandBuffer blitCommandEncoder];
        [blitCommandEncoder copyFromBuffer:buffer
                              sourceOffset:0
                         sourceBytesPerRow:width * 4
                       sourceBytesPerImage:data.size()
                                sourceSize:MTLSizeMake(width, height, 1)
                                 toTexture:texture
                          destinationSlice:0
                          destinationLevel:0
                         destinationOrigin:MTLOriginMake(0, 0, 0)];
        [blitCommandEncoder endEncoding];
        [commandBuffer commit];
    }
    
    if (announceLoad) {
        mprintf("Image loaded into texture %p", texture);
    }
}


void ImageFileStimulus::unloadMetal(MetalDisplay &display) {
    if (announceLoad) {
        mprintf("Image unloaded from texture %p", texture);
    }
    
    texture = nil;
    
    BaseImageStimulus::unloadMetal(display);
}


double ImageFileStimulus::getCurrentAspectRatio() const {
    return double(width) / double(height);
}


id<MTLTexture> ImageFileStimulus::getCurrentTexture() const {
    return texture;
}


END_NAMESPACE_MW
