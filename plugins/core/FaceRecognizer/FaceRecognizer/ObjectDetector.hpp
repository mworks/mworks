//
//  ObjectDetector.hpp
//  ObjectDetector
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef ObjectDetector_hpp
#define ObjectDetector_hpp

#include "ImageAnalyzer.hpp"


BEGIN_NAMESPACE_MW


class ObjectDetector : public ImageAnalyzer {
    
public:
    static const std::string RESULTS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ObjectDetector(const ParameterValueMap &parameters);
    
private:
    void announceInitializationStarting() const override;
    void announceInitializationComplete() const override;
    void analyzeImage(const ModelManager &model,
                      const cf::DataPtr &image,
                      const CGRect &regionOfInterest,
                      VNConfidence minimumConfidence) override;
    
    const VariablePtr results;
    
};


END_NAMESPACE_MW


#endif /* ObjectDetector_hpp */
