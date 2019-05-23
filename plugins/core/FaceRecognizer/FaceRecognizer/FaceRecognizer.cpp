//
//  FaceRecognizer.cpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "FaceRecognizer.hpp"


BEGIN_NAMESPACE_MW


const std::string FaceRecognizer::MODEL_PATH("model_path");
const std::string FaceRecognizer::REGION_OF_INTEREST("region_of_interest");
const std::string FaceRecognizer::MINIMUM_CONFIDENCE("minimum_confidence");
const std::string FaceRecognizer::RESULT_HISTORY("result_history");
const std::string FaceRecognizer::REQUIRED_MATCHES("required_matches");
const std::string FaceRecognizer::IDENTIFIER("identifier");
const std::string FaceRecognizer::CONFIDENCE("confidence");
const std::string FaceRecognizer::CAMERA_UNIQUE_ID("camera_unique_id");
const std::string FaceRecognizer::IMAGE_CAPTURE_INTERVAL("image_capture_interval");
const std::string FaceRecognizer::IMAGE_LOGGING_INTERVAL("image_logging_interval");
const std::string FaceRecognizer::IMAGE_DATA("image_data");


void FaceRecognizer::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/face_recognizer");
    
    info.addParameter(MODEL_PATH);
    info.addParameter(REGION_OF_INTEREST, "[0.0, 0.0, 1.0, 1.0]");
    info.addParameter(MINIMUM_CONFIDENCE, "0.0");
    info.addParameter(RESULT_HISTORY, "1");
    info.addParameter(REQUIRED_MATCHES, "1");
    info.addParameter(IDENTIFIER);
    info.addParameter(CONFIDENCE, false);
    info.addParameter(CAMERA_UNIQUE_ID, false);
    info.addParameter(IMAGE_CAPTURE_INTERVAL);
    info.addParameter(IMAGE_LOGGING_INTERVAL, "0");
    info.addParameter(IMAGE_DATA, false);
}


static inline VariablePtr optionalVariableOrText(const ParameterValue &param) {
    if (param.empty()) {
        return VariablePtr();
    }
    return variableOrText(param);
}


FaceRecognizer::FaceRecognizer(const ParameterValueMap &parameters) :
    IODevice(parameters),
    modelPath(pathFromParameterValue(variableOrText(parameters[MODEL_PATH]))),
    regionOfInterest(parameters[REGION_OF_INTEREST]),
    minimumConfidence(parameters[MINIMUM_CONFIDENCE]),
    resultHistory(parameters[RESULT_HISTORY]),
    requiredMatches(parameters[REQUIRED_MATCHES]),
    identifier(parameters[IDENTIFIER]),
    confidence(optionalVariable(parameters[CONFIDENCE])),
    cameraUniqueID(optionalVariableOrText(parameters[CAMERA_UNIQUE_ID])),
    imageCaptureInterval(parameters[IMAGE_CAPTURE_INTERVAL]),
    imageLoggingInterval(parameters[IMAGE_LOGGING_INTERVAL]),
    imageData(optionalVariable(parameters[IMAGE_DATA])),
    clock(Clock::instance())
{ }


FaceRecognizer::~FaceRecognizer() {
    lock_guard lock(mutex);
    
    if (recognizerTask) {
        recognizerTask->cancel();
    }
    
    if (regionOfInterestNotification) {
        regionOfInterestNotification->remove();
    }
}


bool FaceRecognizer::initialize() {
    lock_guard lock(mutex);
    
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Configuring face recognizer \"%s\"...", getTag().c_str());
    
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
        boost::weak_ptr<FaceRecognizer> weakThis(component_shared_from_this<FaceRecognizer>());
        auto callback = [weakThis](const Datum &data, MWTime time) {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                if (sharedThis->recognizerTask) {
                    sharedThis->setCurrentRegionOfInterest(data);
                }
            }
        };
        regionOfInterestNotification = boost::make_shared<VariableCallbackNotification>(callback);
        regionOfInterest->addNotification(regionOfInterestNotification);
    }
    
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Face recognizer \"%s\" is ready", getTag().c_str());
    
    return true;
}


bool FaceRecognizer::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (recognizerTask) {
        // Already started
        return true;
    }
    
    setCurrentRegionOfInterest(regionOfInterest->getValue());
    
    currentMinimumConfidence = minimumConfidence->getValue().getFloat();
    if (currentMinimumConfidence < 0.0 || currentMinimumConfidence > 1.0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s must be between zero and one (inclusive)", MINIMUM_CONFIDENCE.c_str());
        return false;
    }
    
    currentResultHistory = resultHistory->getValue().getInteger();
    if (currentResultHistory < 1) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s must be greater than zero", RESULT_HISTORY.c_str());
        return false;
    }
    
    currentRequiredMatches = requiredMatches->getValue().getInteger();
    if (currentRequiredMatches < 1 || currentRequiredMatches > currentResultHistory) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "%s must be between one and %s (inclusive)",
               REQUIRED_MATCHES.c_str(),
               RESULT_HISTORY.c_str());
        return false;
    }
    
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
    
    if (!camera.startCaptureSession()) {
        return false;
    }
    
    lastLogTime = clock->getCurrentTimeUS();
    identifierHistory.clear();
    
    boost::weak_ptr<FaceRecognizer> weakThis(component_shared_from_this<FaceRecognizer>());
    recognizerTask = Scheduler::instance()->scheduleUS(FILELINE,
                                                       currentCaptureInterval,
                                                       currentCaptureInterval,
                                                       M_REPEAT_INDEFINITELY,
                                                       [weakThis]() {
                                                           if (auto sharedThis = weakThis.lock()) {
                                                               sharedThis->performRecognition();
                                                           }
                                                           return nullptr;
                                                       },
                                                       M_DEFAULT_IODEVICE_PRIORITY,
                                                       M_DEFAULT_IODEVICE_WARN_SLOP_US,
                                                       M_DEFAULT_IODEVICE_FAIL_SLOP_US,
                                                       M_MISSED_EXECUTION_DROP);
    
    return true;
}


bool FaceRecognizer::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (recognizerTask) {
        recognizerTask->cancel();
        recognizerTask.reset();
    }
    
    camera.stopCaptureSession();
    
    return true;
}


void FaceRecognizer::setCurrentRegionOfInterest(const Datum &data) {
    currentRegionOfInterest = CGRectMake(0.0, 0.0, 1.0, 1.0);
    
    if (!data.isList()) {
        // Non-list is OK and implies default region of interest
        return;
    }
    
    auto &list = data.getList();
    if (list.empty()) {
        // Empty list is OK and implies default region of interest
        return;
    }
    
    if (list.size() != 4) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Wrong number of elements in region of interest list (expected 4, got %lu)",
               list.size());
        return;
    }
    
    std::vector<double> values;
    for (auto &item : list) {
        if (!item.isNumber()) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Wrong element type in region of interest list (expected number, got %s)",
                   item.getDataTypeName());
            return;
        }
        values.push_back(item.getFloat());
        if (values.back() < 0.0 || values.back() > 1.0) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Values in region of interest list must be between zero and one (inclusive); got %g",
                   values.back());
            return;
        }
    }
    
    auto &minX = values.at(0);
    auto &minY = values.at(1);
    auto &maxX = values.at(2);
    auto &maxY = values.at(3);
    
    if (minX >= maxX || minY >= maxY) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Invalid region of interest: [%g, %g, %g, %g]", minX, minY, maxX, maxY);
        return;
    }
    
    currentRegionOfInterest = CGRectMake(minX, minY, maxX - minX, maxY - minY);
}


static inline bool updateVariable(const VariablePtr &var, Datum value) {
    if (var->getValue() != value) {
        var->setValue(value);
        return true;
    }
    return false;
}


void FaceRecognizer::performRecognition() {
    lock_guard lock(mutex);
    
    if (!recognizerTask) {
        // We've already been canceled, so don't try to process another image
        return;
    }
    
    std::string currentIdentifier;
    VNConfidence currentConfidence = 0.0;
    
    if (auto image = camera.captureImage()) {
        model.classifyImage(image,
                            currentRegionOfInterest,
                            currentMinimumConfidence,
                            currentIdentifier,
                            currentConfidence);
        
        if (currentLoggingInterval > 0) {
            auto currentTime = clock->getCurrentTimeUS();
            if (currentTime - lastLogTime >= currentLoggingInterval) {
                imageData->setValue(Datum(std::string(reinterpret_cast<const char *>(CFDataGetBytePtr(image.get())),
                                                      CFDataGetLength(image.get()))));
                lastLogTime = currentTime;
            }
        }
    }
    
    identifierHistory.push_back(currentIdentifier);
    if (identifierHistory.size() > currentResultHistory) {
        identifierHistory.pop_front();
    }
    
    if (std::count(identifierHistory.begin(), identifierHistory.end(), currentIdentifier) >= currentRequiredMatches) {
        if (updateVariable(identifier, Datum(currentIdentifier))) {
            if (confidence) {
                updateVariable(confidence, Datum(currentConfidence));
            }
        }
    }
}


END_NAMESPACE_MW
