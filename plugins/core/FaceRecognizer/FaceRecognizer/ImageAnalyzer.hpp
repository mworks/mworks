//
//  ImageAnalyzer.hpp
//  ImageAnalyzer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef ImageAnalyzer_hpp
#define ImageAnalyzer_hpp

#include "CameraManager.hpp"
#include "ModelManager.hpp"


BEGIN_NAMESPACE_MW


class ImageAnalyzer : public IODevice, boost::noncopyable {
    
public:
    static const std::string MODEL_PATH;
    static const std::string REGION_OF_INTEREST;
    static const std::string MINIMUM_CONFIDENCE;
    static const std::string CAMERA_UNIQUE_ID;
    static const std::string IMAGE_CAPTURE_INTERVAL;
    static const std::string IMAGE_LOGGING_INTERVAL;
    static const std::string IMAGE_DATA;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ImageAnalyzer(const ParameterValueMap &parameters);
    ~ImageAnalyzer();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
protected:
    static bool updateVariable(const VariablePtr &var, Datum value) {
        if (var->getValue() != value) {
            var->setValue(value);
            return true;
        }
        return false;
    }
    
    virtual void announceInitializationStarting() const = 0;
    virtual void announceInitializationComplete() const = 0;
    virtual bool updateParameterValues();
    virtual void analyzeImage(const ModelManager &model,
                              const cf::DataPtr &image,
                              const CGRect &regionOfInterest,
                              VNConfidence minimumConfidence) = 0;
    
private:
    bool setCurrentRegionOfInterest(const Datum &data);
    void performAnalysis();
    
    const boost::filesystem::path modelPath;
    const VariablePtr regionOfInterest;
    const VariablePtr minimumConfidence;
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
    MWTime currentCaptureInterval;
    MWTime currentLoggingInterval;
    
    MWTime lastLogTime;
    
    boost::shared_ptr<ScheduleTask> analysisTask;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* ImageAnalyzer_hpp */
