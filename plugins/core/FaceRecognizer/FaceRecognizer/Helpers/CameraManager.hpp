//
//  CameraManager.hpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/29/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef CameraManager_hpp
#define CameraManager_hpp


BEGIN_NAMESPACE_MW


class CameraManager : boost::noncopyable {
    
public:
    CameraManager();
    ~CameraManager();
    
    bool initialize(const std::string &cameraUniqueID);
    bool startCaptureSession();
    void stopCaptureSession();
    
    cf::DataPtr captureImage();
    
private:
    static bool acquireCameraAccess();
    static AVCaptureDevice * discoverCamera(const std::string &cameraUniqueID);
    static AVCaptureSession * createCaptureSession(AVCaptureDevice *camera);
    static void captureImage(AVCaptureSession *captureSession, CGFloat videoRotationAngle, cf::DataPtr &image);
    
    AVCaptureDevice *camera;
#if TARGET_OS_IPHONE
    AVCaptureDeviceRotationCoordinator *rotationCoordinator;
#endif
    AVCaptureSession *captureSession;
    id<NSObject> captureSessionRuntimeErrorObserver;
    
};


END_NAMESPACE_MW


#endif /* CameraManager_hpp */
