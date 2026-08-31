//
//  FaceRecognizer.cpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "FaceRecognizer.hpp"


BEGIN_NAMESPACE_MW


const std::string FaceRecognizer::RESULT_HISTORY("result_history");
const std::string FaceRecognizer::REQUIRED_MATCHES("required_matches");
const std::string FaceRecognizer::IDENTIFIER("identifier");
const std::string FaceRecognizer::CONFIDENCE("confidence");


void FaceRecognizer::describeComponent(ComponentInfo &info) {
    ImageAnalyzer::describeComponent(info);
    
    info.setSignature("iodevice/face_recognizer");
    
    info.addParameter(RESULT_HISTORY, "1");
    info.addParameter(REQUIRED_MATCHES, "1");
    info.addParameter(IDENTIFIER);
    info.addParameter(CONFIDENCE, false);
}


FaceRecognizer::FaceRecognizer(const ParameterValueMap &parameters) :
    ImageAnalyzer(parameters),
    resultHistory(parameters[RESULT_HISTORY]),
    requiredMatches(parameters[REQUIRED_MATCHES]),
    identifier(parameters[IDENTIFIER]),
    confidence(optionalVariable(parameters[CONFIDENCE]))
{ }


void FaceRecognizer::announceInitializationStarting() const {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Configuring face recognizer \"%s\"...", getTag().c_str());
}


void FaceRecognizer::announceInitializationComplete() const {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Face recognizer \"%s\" is ready", getTag().c_str());
}


bool FaceRecognizer::updateParameterValues() {
    if (!ImageAnalyzer::updateParameterValues()) {
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
    
    identifierHistory.clear();
    
    return true;
}


void FaceRecognizer::analyzeImage(const ModelManager &model,
                                  const cf::DataPtr &image,
                                  const CGRect &regionOfInterest,
                                  VNConfidence minimumConfidence)
{
    std::string currentIdentifier;
    VNConfidence currentConfidence = 0.0;
    
    model.classifyImage(image,
                        regionOfInterest,
                        minimumConfidence,
                        currentIdentifier,
                        currentConfidence);
    
    identifierHistory.push_back(currentIdentifier);
    if (identifierHistory.size() > currentResultHistory) {
        identifierHistory.pop_front();
    }
    
    if (std::count(identifierHistory.begin(), identifierHistory.end(), currentIdentifier) >= currentRequiredMatches) {
        if (updateVariable(identifier, Datum(std::move(currentIdentifier)))) {
            if (confidence) {
                updateVariable(confidence, Datum(currentConfidence));
            }
        }
    }
}


END_NAMESPACE_MW
