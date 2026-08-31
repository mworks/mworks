//
//  ObjectDetector.cpp
//  ObjectDetector
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "ObjectDetector.hpp"


BEGIN_NAMESPACE_MW


const std::string ObjectDetector::RESULTS("results");


void ObjectDetector::describeComponent(ComponentInfo &info) {
    ImageAnalyzer::describeComponent(info);
    
    info.setSignature("iodevice/object_detector");
    
    info.addParameter(RESULTS);
}


ObjectDetector::ObjectDetector(const ParameterValueMap &parameters) :
    ImageAnalyzer(parameters),
    results(parameters[RESULTS])
{ }


void ObjectDetector::announceInitializationStarting() const {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Configuring object detector \"%s\"...", getTag().c_str());
}


void ObjectDetector::announceInitializationComplete() const {
    mprintf(M_IODEVICE_MESSAGE_DOMAIN, "Object detector \"%s\" is ready", getTag().c_str());
}


void ObjectDetector::analyzeImage(const ModelManager &model,
                                  const cf::DataPtr &image,
                                  const CGRect &regionOfInterest,
                                  VNConfidence minimumConfidence)
{
    ModelManager::DetectedObjects objects;
    
    model.detectObjects(image,
                        regionOfInterest,
                        minimumConfidence,
                        objects);
    
    Datum::list_value_type currentResults;
    for (auto &object : objects) {
        auto &boundingBox = std::get<CGRect>(object);
        currentResults.emplace_back(Datum::list_value_type{
            Datum(std::get<std::string>(std::move(object))),
            Datum(Datum::list_value_type{
                Datum(CGRectGetMinX(boundingBox)),
                Datum(CGRectGetMinY(boundingBox)),
                Datum(CGRectGetMaxX(boundingBox)),
                Datum(CGRectGetMaxY(boundingBox))
            }),
            Datum(std::get<VNConfidence>(object))
        });
    }
    
    updateVariable(results, Datum(std::move(currentResults)));
}


END_NAMESPACE_MW
