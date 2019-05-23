//
//  ModelManager.hpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/29/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef ModelManager_hpp
#define ModelManager_hpp


BEGIN_NAMESPACE_MW


class API_AVAILABLE(macos(10.13)) ModelManager : boost::noncopyable {
    
public:
    ModelManager();
    ~ModelManager();
    
    bool loadModel(const boost::filesystem::path &modelPath);
    bool classifyImage(const cf::DataPtr &image,
                       const CGRect &regionOfInterest,
                       VNConfidence minimumConfidence,
                       std::string &identifier,
                       VNConfidence &confidence);
    
private:
    static std::tuple<VNCoreMLModel *, NSURL *> _loadModel(const boost::filesystem::path &modelPath);
    static bool classifyImage(VNCoreMLModel *model,
                              const cf::DataPtr &image,
                              const CGRect &regionOfInterest,
                              VNConfidence minimumConfidence,
                              std::string &identifier,
                              VNConfidence &confidence);
    static void removeCompiledModel(NSURL *compiledModelURL);
    
    NSURL *compiledModelURL;
    VNCoreMLModel *model;
    
};


END_NAMESPACE_MW


#endif /* ModelManager_hpp */
