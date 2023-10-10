//
//  CameraManager.cpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/29/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "CameraManager.hpp"


using MWKPhotoCaptureDelegateCallback = void (^)(AVCapturePhoto *photo, NSError *error);


@interface MWKPhotoCaptureDelegate : NSObject <AVCapturePhotoCaptureDelegate>

@property(nonatomic, strong) MWKPhotoCaptureDelegateCallback callback;

@end


@implementation MWKPhotoCaptureDelegate


- (void)captureOutput:(AVCapturePhotoOutput *)output
didFinishProcessingPhoto:(AVCapturePhoto *)photo
error:(NSError *)error
{
    self.callback(photo, error);
}


@end


BEGIN_NAMESPACE_MW


CameraManager::CameraManager() :
    camera(nil),
#if TARGET_OS_IPHONE
    rotationCoordinator(nil),
#endif
    captureSession(nil),
    captureSessionRuntimeErrorObserver(nil)
{ }


CameraManager::~CameraManager() {
    @autoreleasepool {
        captureSessionRuntimeErrorObserver = nil;
        captureSession = nil;
#if TARGET_OS_IPHONE
        rotationCoordinator = nil;
#endif
        camera = nil;
    }
}


bool CameraManager::initialize(const std::string &cameraUniqueID) {
    @autoreleasepool {
        if (!acquireCameraAccess()) {
            return false;
        }
        
        camera = nil;
        if (!(camera = discoverCamera(cameraUniqueID))) {
            return false;
        }
        
        mprintf(M_IODEVICE_MESSAGE_DOMAIN,
                "Using camera \"%s\" (unique ID: \"%s\")",
                camera.localizedName.UTF8String,
                camera.uniqueID.UTF8String);
        
#if TARGET_OS_IPHONE
        rotationCoordinator = [[AVCaptureDeviceRotationCoordinator alloc] initWithDevice:camera previewLayer:nil];
#endif
        
        return true;
    }
}


bool CameraManager::startCaptureSession() {
    @autoreleasepool {
        if (!captureSession) {
            if (!(captureSession = createCaptureSession(camera))) {
                return false;
            }
            
            {
                auto callback = [](NSNotification *notification) {
                    NSError *error = notification.userInfo[AVCaptureSessionErrorKey];
                    if (error) {
                        merror(M_IODEVICE_MESSAGE_DOMAIN,
                               "Capture session error: %s",
                               error.localizedDescription.UTF8String);
                    }
                };
                captureSessionRuntimeErrorObserver = [NSNotificationCenter.defaultCenter addObserverForName:AVCaptureSessionRuntimeErrorNotification
                                                                                                     object:captureSession
                                                                                                      queue:nil
                                                                                                 usingBlock:callback];
            }
            
            [captureSession startRunning];
        }
        return true;
    }
}


void CameraManager::stopCaptureSession() {
    @autoreleasepool {
        if (captureSession) {
            [captureSession stopRunning];
            
            [NSNotificationCenter.defaultCenter removeObserver:captureSessionRuntimeErrorObserver];
            captureSessionRuntimeErrorObserver = nil;
            
            captureSession = nil;
        }
    }
}


cf::DataPtr CameraManager::captureImage() {
    @autoreleasepool {
        cf::DataPtr image;
        if (captureSession) {
            CGFloat videoRotationAngle = 0.0;
#if TARGET_OS_IPHONE
            videoRotationAngle = rotationCoordinator.videoRotationAngleForHorizonLevelCapture;
#endif
            captureImage(captureSession, videoRotationAngle, image);
        }
        return image;
    }
}


bool CameraManager::acquireCameraAccess() {
    bool accessGranted = false;
    
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo]) {
        case AVAuthorizationStatusNotDetermined: {
            // Use a shared promise, because the user can still grant or deny access (and thereby invoke the
            // completion handler) after this method times out and exits
            auto p = std::make_shared<std::promise<bool>>();
            auto f = p->get_future();
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
                                     completionHandler:[p](BOOL granted) { p->set_value(granted); }];
            mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Waiting for user authorization to access the camera...");
            if (std::future_status::ready != f.wait_for(std::chrono::seconds(60))) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Timed out waiting for user authorization to access the camera");
            } else if (!f.get()) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "User denied access to the camera");
            } else {
                mprintf(M_IODEVICE_MESSAGE_DOMAIN, "User granted access to the camera");
                accessGranted = true;
            }
            break;
        }
            
        case AVAuthorizationStatusRestricted:
            merror(M_IODEVICE_MESSAGE_DOMAIN, "User is not allowed to access the camera");
            break;
            
        case AVAuthorizationStatusDenied:
            merror(M_IODEVICE_MESSAGE_DOMAIN, "User previously denied access to the camera");
            break;
            
        case AVAuthorizationStatusAuthorized:
            // User previously granted access to the camera
            accessGranted = true;
            break;
    }
    
    return accessGranted;
}


AVCaptureDevice * CameraManager::discoverCamera(const std::string &cameraUniqueID) {
#if TARGET_OS_IPHONE
    
    if (!cameraUniqueID.empty()) {
        mwarning(M_IODEVICE_MESSAGE_DOMAIN, "Camera unique ID is ignored on iOS");
    }
    auto camera = [AVCaptureDevice defaultDeviceWithDeviceType:AVCaptureDeviceTypeBuiltInWideAngleCamera
                                                     mediaType:AVMediaTypeVideo
                                                      position:AVCaptureDevicePositionFront];
    if (!camera) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Front-facing camera not found");
        return nil;
    }
    return camera;
    
#else
    
    auto discoverySession = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera,
                                                                                               AVCaptureDeviceTypeExternalUnknown]
                                                                                   mediaType:AVMediaTypeVideo
                                                                                    position:AVCaptureDevicePositionUnspecified];
    auto cameras = discoverySession.devices;
    if (cameras.count == 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "No cameras detected");
        return nil;
    }
    if (!cameraUniqueID.empty()) {
        for (AVCaptureDevice *camera in cameras) {
            if ([camera.uniqueID isEqualToString:@(cameraUniqueID.c_str())]) {
                return camera;
            }
        }
        merror(M_IODEVICE_MESSAGE_DOMAIN, "No camera detected with unique ID \"%s\"", cameraUniqueID.c_str());
        return nil;
    }
    if (cameras.count > 1) {
        std::ostringstream oss;
        oss << "Multiple cameras detected:\n";
        for (AVCaptureDevice *camera in cameras) {
            oss << "\nName:\t\t" << camera.localizedName.UTF8String;
            oss << "\nUnique ID:\t\"" << camera.uniqueID.UTF8String << "\"\n";
        }
        oss << "\nPlease specify the unique ID of the desired camera.\n";
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s", oss.str().c_str());
        return nil;
    }
    return cameras[0];
    
#endif
}


AVCaptureSession * CameraManager::createCaptureSession(AVCaptureDevice *camera) {
    auto captureSession = [[AVCaptureSession alloc] init];
    [captureSession beginConfiguration];
    
    {
        NSError *error = nil;
        auto captureInput = [AVCaptureDeviceInput deviceInputWithDevice:camera error:&error];
        if (!captureInput) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot use camera as capture input: %s",
                   error.localizedDescription.UTF8String);
            return nil;
        }
        if (![captureSession canAddInput:captureInput]) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot add camera to capture session");
            return nil;
        }
        [captureSession addInput:captureInput];
    }
    
    {
        auto captureOutput = [[AVCapturePhotoOutput alloc] init];
        if (![captureSession canAddOutput:captureOutput]) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot add image output to capture session");
            return nil;
        }
        [captureSession addOutput:captureOutput];
    }
    
    [captureSession commitConfiguration];
    return captureSession;
}


void CameraManager::captureImage(AVCaptureSession *captureSession, CGFloat videoRotationAngle, cf::DataPtr &image) {
    std::promise<void> p;
    auto f = p.get_future();
    
    auto captureOutput = static_cast<AVCapturePhotoOutput *>(captureSession.outputs[0]);
    auto settings = [AVCapturePhotoSettings photoSettingsWithFormat:@{ AVVideoCodecKey: AVVideoCodecTypeJPEG }];
    auto delegate = [[MWKPhotoCaptureDelegate alloc] init];
    delegate.callback = [&image, &p](AVCapturePhoto *photo, NSError *error) {
        if (error) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Image capture failed: %s", error.localizedDescription.UTF8String);
        } else {
            auto jpegData = [photo fileDataRepresentation];
            image = cf::DataPtr::borrowed((__bridge CFDataRef)jpegData);
        }
        p.set_value();
    };
#if TARGET_OS_IPHONE
    auto connection = captureOutput.connections[0];
    if ([connection isVideoRotationAngleSupported:videoRotationAngle]) {
        connection.videoRotationAngle = videoRotationAngle;
    }
#endif
    [captureOutput capturePhotoWithSettings:settings delegate:delegate];
    
    if (std::future_status::ready != f.wait_for(std::chrono::seconds(1))) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Timed out waiting for image capture to complete");
    }
}


END_NAMESPACE_MW
