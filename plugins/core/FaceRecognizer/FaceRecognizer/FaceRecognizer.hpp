//
//  FaceRecognizer.hpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef FaceRecognizer_hpp
#define FaceRecognizer_hpp

#include "CameraManager.hpp"
#include "ModelManager.hpp"


BEGIN_NAMESPACE_MW


class API_AVAILABLE(macos(10.13)) FaceRecognizer : public IODevice, boost::noncopyable {
    
public:
    static const std::string MODEL_PATH;
    static const std::string REGION_OF_INTEREST;
    static const std::string MINIMUM_CONFIDENCE;
    static const std::string RESULT_HISTORY;
    static const std::string REQUIRED_MATCHES;
    static const std::string IDENTIFIER;
    static const std::string CONFIDENCE;
    static const std::string CAMERA_UNIQUE_ID;
    static const std::string IMAGE_CAPTURE_INTERVAL;
    static const std::string IMAGE_LOGGING_INTERVAL;
    static const std::string IMAGE_DATA;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FaceRecognizer(const ParameterValueMap &parameters);
    ~FaceRecognizer();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    void setCurrentRegionOfInterest(const Datum &data);
    void performRecognition();
    
    const boost::filesystem::path modelPath;
    const VariablePtr regionOfInterest;
    const VariablePtr minimumConfidence;
    const VariablePtr resultHistory;
    const VariablePtr requiredMatches;
    const VariablePtr identifier;
    const VariablePtr confidence;
    const VariablePtr cameraUniqueID;
    const VariablePtr imageCaptureInterval;
    const VariablePtr imageLoggingInterval;
    const VariablePtr imageData;
    
    const boost::shared_ptr<Clock> clock;
    CameraManager camera;
    ModelManager model;
    boost::shared_ptr<VariableNotification> regionOfInterestNotification;
    
    CGRect currentRegionOfInterest;
    VNConfidence currentMinimumConfidence;
    int currentResultHistory;
    int currentRequiredMatches;
    MWTime currentCaptureInterval;
    MWTime currentLoggingInterval;
    
    MWTime lastLogTime;
    std::deque<std::string> identifierHistory;
    
    boost::shared_ptr<ScheduleTask> recognizerTask;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* FaceRecognizer_hpp */
