//
//  ImageAnalyzer.cpp
//  ImageAnalyzer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "ImageAnalyzer.hpp"


BEGIN_NAMESPACE_MW


const std::string ImageAnalyzer::MODEL_PATH("model_path");
const std::string ImageAnalyzer::REGION_OF_INTEREST("region_of_interest");
const std::string ImageAnalyzer::MINIMUM_CONFIDENCE("minimum_confidence");
const std::string ImageAnalyzer::CAMERA_UNIQUE_ID("camera_unique_id");
const std::string ImageAnalyzer::IMAGE_CAPTURE_INTERVAL("image_capture_interval");
const std::string ImageAnalyzer::IMAGE_LOGGING_INTERVAL("image_logging_interval");
const std::string ImageAnalyzer::IMAGE_DATA("image_data");


void ImageAnalyzer::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.addParameter(MODEL_PATH);
    info.addParameter(REGION_OF_INTEREST, "[0.0, 0.0, 1.0, 1.0]");
    info.addParameter(MINIMUM_CONFIDENCE, "0.0");
    info.addParameter(CAMERA_UNIQUE_ID, false);
    info.addParameter(IMAGE_CAPTURE_INTERVAL);
    info.addParameter(IMAGE_LOGGING_INTERVAL, "0");
    info.addParameter(IMAGE_DATA, false);
}


ImageAnalyzer::ImageAnalyzer(const ParameterValueMap &parameters) :
    IODevice(parameters),
    modelPath(pathFromParameterValue(variableOrText(parameters[MODEL_PATH]))),
    regionOfInterest(parameters[REGION_OF_INTEREST]),
    minimumConfidence(parameters[MINIMUM_CONFIDENCE]),
    cameraUniqueID(optionalVariableOrText(parameters[CAMERA_UNIQUE_ID])),
    imageCaptureInterval(parameters[IMAGE_CAPTURE_INTERVAL]),
    imageLoggingInterval(parameters[IMAGE_LOGGING_INTERVAL]),
    imageData(optionalVariable(parameters[IMAGE_DATA])),
    clock(Clock::instance())
{ }


ImageAnalyzer::~ImageAnalyzer() {
    lock_guard lock(mutex);
    
    if (analysisTask) {
        analysisTask->cancel();
    }
    
    if (regionOfInterestNotification) {
        regionOfInterestNotification->remove();
    }
}


bool ImageAnalyzer::initialize() {
    lock_guard lock(mutex);
    
    announceInitializationStarting();
    
    {
        std::string currentCameraUniqueID;
        if (cameraUniqueID) {
            currentCameraUniqueID = cameraUniqueID->getValue().getString();
        }
        if (!camera.initialize(currentCameraUniqueID)) {
            return false;
        }
    }
    
    if (!model.loadModel(modelPath)) {
        return false;
    }
    
    {
        boost::weak_ptr<ImageAnalyzer> weakThis(component_shared_from_this<ImageAnalyzer>());
        auto callback = [weakThis](const Datum &data, MWTime time) {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                if (sharedThis->analysisTask) {
                    sharedThis->setCurrentRegionOfInterest(data);
                }
            }
        };
        regionOfInterestNotification = boost::make_shared<VariableCallbackNotification>(callback);
        regionOfInterest->addNotification(regionOfInterestNotification);
    }
    
    announceInitializationComplete();
    
    return true;
}


bool ImageAnalyzer::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (analysisTask) {
        // Already started
        return true;
    }
    
    if (!updateParameterValues() || !camera.startCaptureSession()) {
        return false;
    }
    
    lastLogTime = clock->getCurrentTimeUS();
    
    boost::weak_ptr<ImageAnalyzer> weakThis(component_shared_from_this<ImageAnalyzer>());
    analysisTask = Scheduler::instance()->scheduleUS(FILELINE,
                                                     currentCaptureInterval,
                                                     currentCaptureInterval,
                                                     M_REPEAT_INDEFINITELY,
                                                     [weakThis]() {
                                                         if (auto sharedThis = weakThis.lock()) {
                                                             sharedThis->performAnalysis();
                                                         }
                                                         return nullptr;
                                                     },
                                                     M_DEFAULT_IODEVICE_PRIORITY,
                                                     M_DEFAULT_IODEVICE_WARN_SLOP_US,
                                                     M_DEFAULT_IODEVICE_FAIL_SLOP_US,
                                                     M_MISSED_EXECUTION_DROP);
    
    return true;
}


bool ImageAnalyzer::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (analysisTask) {
        analysisTask->cancel();
        analysisTask.reset();
    }
    
    camera.stopCaptureSession();
    
    return true;
}


bool ImageAnalyzer::updateParameterValues() {
    if (!setCurrentRegionOfInterest(regionOfInterest->getValue())) {
        return false;
    }
    
    // The meaning of confidence depends on the model, so don't enforce any constraints
    // on the minimum
    currentMinimumConfidence = minimumConfidence->getValue().getFloat();
    
    currentCaptureInterval = imageCaptureInterval->getValue().getInteger();
    if (currentCaptureInterval <= 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s must be greater than zero", IMAGE_CAPTURE_INTERVAL.c_str());
        return false;
    }
    
    currentLoggingInterval = imageLoggingInterval->getValue().getInteger();
    if (currentLoggingInterval < 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s must be greater than or equal to zero", IMAGE_LOGGING_INTERVAL.c_str());
        return false;
    }
    
    if (currentLoggingInterval > 0 && !imageData) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "%s is required when %s is greater than zero",
               IMAGE_DATA.c_str(),
               IMAGE_LOGGING_INTERVAL.c_str());
        return false;
    }
    
    return true;
}


bool ImageAnalyzer::setCurrentRegionOfInterest(const Datum &data) {
    const auto defaultRegionOfInterest = CGRectMake(0.0, 0.0, 1.0, 1.0);
    
    if (!data.isList()) {
        // Non-list is OK and implies default region of interest
        currentRegionOfInterest = defaultRegionOfInterest;
        return true;
    }
    
    auto &list = data.getList();
    if (list.empty()) {
        // Empty list is OK and implies default region of interest
        currentRegionOfInterest = defaultRegionOfInterest;
        return true;
    }
    
    if (list.size() != 4) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Wrong number of elements in region of interest list (expected 4, got %lu)",
               list.size());
        return false;
    }
    
    std::vector<double> values;
    for (auto &item : list) {
        if (!item.isNumber()) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Wrong element type in region of interest list (expected number, got %s)",
                   item.getDataTypeName());
            return false;
        }
        values.push_back(item.getFloat());
        if (values.back() < 0.0 || values.back() > 1.0) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Values in region of interest list must be between zero and one (inclusive); got %g",
                   values.back());
            return false;
        }
    }
    
    auto &minX = values.at(0);
    auto &minY = values.at(1);
    auto &maxX = values.at(2);
    auto &maxY = values.at(3);
    
    if (minX >= maxX || minY >= maxY) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Invalid region of interest: [%g, %g, %g, %g]", minX, minY, maxX, maxY);
        return false;
    }
    
    currentRegionOfInterest = CGRectMake(minX, minY, maxX - minX, maxY - minY);
    
    return true;
}


void ImageAnalyzer::performAnalysis() {
    lock_guard lock(mutex);
    
    if (!analysisTask) {
        // We've already been canceled, so don't try to process another image
        return;
    }
    
    if (auto image = camera.captureImage()) {
        analyzeImage(model, image, currentRegionOfInterest, currentMinimumConfidence);
        
        if (currentLoggingInterval > 0) {
            auto currentTime = clock->getCurrentTimeUS();
            if (currentTime - lastLogTime >= currentLoggingInterval) {
                imageData->setValue(Datum(std::string(reinterpret_cast<const char *>(CFDataGetBytePtr(image.get())),
                                                      CFDataGetLength(image.get()))));
                lastLogTime = currentTime;
            }
        }
    }
}


END_NAMESPACE_MW
