//
//  ModelManager.cpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/29/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "ModelManager.hpp"


BEGIN_NAMESPACE_MW


ModelManager::ModelManager() :
    compiledModelURL(nil),
    model(nil)
{ }


ModelManager::~ModelManager() {
    @autoreleasepool {
        model = nil;
        removeCompiledModel(compiledModelURL);
        compiledModelURL = nil;
    }
}


bool ModelManager::loadModel(const boost::filesystem::path &modelPath) {
    @autoreleasepool {
        model = nil;
        removeCompiledModel(compiledModelURL);
        compiledModelURL = nil;
        
        std::tie(model, compiledModelURL) = _loadModel(modelPath);
        if (!model) {
            return false;
        }
        
        return true;
    }
}


bool ModelManager::classifyImage(const cf::DataPtr &image,
                                 const CGRect &regionOfInterest,
                                 VNConfidence minimumConfidence,
                                 std::string &identifier,
                                 VNConfidence &confidence)
{
    @autoreleasepool {
        bool success = false;
        if (model) {
            success = classifyImage(model, image, regionOfInterest, minimumConfidence, identifier, confidence);
        }
        return success;
    }
}


std::tuple<VNCoreMLModel *, NSURL *> ModelManager::_loadModel(const boost::filesystem::path &modelPath) {
    VNCoreMLModel *model = nil;
    NSError *error = nil;
    
    auto modelURL = [NSURL fileURLWithPath:@(modelPath.string().c_str()) isDirectory:NO];
    auto compiledModelURL = [MLModel compileModelAtURL:modelURL error:&error];
    if (!compiledModelURL) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot compile model file: %s", error.localizedDescription.UTF8String);
    } else {
        auto baseModel = [MLModel modelWithContentsOfURL:compiledModelURL error:&error];
        if (!baseModel) {
            merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot load model: %s", error.localizedDescription.UTF8String);
        } else {
            model = [VNCoreMLModel modelForMLModel:baseModel error:&error];
            if (!model) {
                merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot use model: %s", error.localizedDescription.UTF8String);
            }
        }
    }
    
    return std::make_tuple(model, compiledModelURL);
}


bool ModelManager::classifyImage(VNCoreMLModel *model,
                                 const cf::DataPtr &image,
                                 const CGRect &regionOfInterest,
                                 VNConfidence minimumConfidence,
                                 std::string &identifier,
                                 VNConfidence &confidence)
{
    auto requestHandler = [[VNImageRequestHandler alloc] initWithData:static_cast<NSData *>(image.get()) options:@{}];
    
    auto requestCompletionHandler = [](VNRequest *request, NSError *error) {
        if (error) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Image classification request failed: %s",
                   error.localizedDescription.UTF8String);
        }
    };
    auto request = [[VNCoreMLRequest alloc] initWithModel:model completionHandler:requestCompletionHandler];
    request.regionOfInterest = regionOfInterest;
    request.imageCropAndScaleOption = VNImageCropAndScaleOptionCenterCrop;
    
    NSError *error = nil;
    if (![requestHandler performRequests:@[ request ] error:&error]) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Cannot classify image: %s", error.localizedDescription.UTF8String);
        return false;
    }
    
    auto results = request.results;
    if (!results) {
        return false;
    }
    
    for (VNClassificationObservation *result in results) {
        if ([result isKindOfClass:[VNClassificationObservation class]] &&
            result.confidence >= minimumConfidence)
        {
            identifier = result.identifier.UTF8String;
            confidence = result.confidence;
            return true;
        }
    }
    
    return false;
}


void ModelManager::removeCompiledModel(NSURL *compiledModelURL) {
    if (compiledModelURL) {
        NSError *error = nil;
        if (![NSFileManager.defaultManager removeItemAtURL:compiledModelURL error:&error]) {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "Cannot remove compiled model: %s",
                   error.localizedDescription.UTF8String);
        }
    }
}


END_NAMESPACE_MW
