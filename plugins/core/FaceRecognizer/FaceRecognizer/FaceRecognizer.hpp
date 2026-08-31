//
//  FaceRecognizer.hpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef FaceRecognizer_hpp
#define FaceRecognizer_hpp

#include "ImageAnalyzer.hpp"


BEGIN_NAMESPACE_MW


class FaceRecognizer : public ImageAnalyzer {
    
public:
    static const std::string RESULT_HISTORY;
    static const std::string REQUIRED_MATCHES;
    static const std::string IDENTIFIER;
    static const std::string CONFIDENCE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FaceRecognizer(const ParameterValueMap &parameters);
    
private:
    void announceInitializationStarting() const override;
    void announceInitializationComplete() const override;
    bool updateParameterValues() override;
    void analyzeImage(const ModelManager &model,
                      const cf::DataPtr &image,
                      const CGRect &regionOfInterest,
                      VNConfidence minimumConfidence) override;
    
    const VariablePtr resultHistory;
    const VariablePtr requiredMatches;
    const VariablePtr identifier;
    const VariablePtr confidence;
    
    int currentResultHistory;
    int currentRequiredMatches;
    
    std::deque<std::string> identifierHistory;
    
};


END_NAMESPACE_MW


#endif /* FaceRecognizer_hpp */
