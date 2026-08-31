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


class ModelManager : boost::noncopyable {
    
public:
    using DetectedObjects = std::vector<std::tuple<std::string, CGRect, VNConfidence>>;
    
    ModelManager();
    ~ModelManager();
    
    bool loadModel(const boost::filesystem::path &modelPath);
    bool classifyImage(const cf::DataPtr &image,
                       const CGRect &regionOfInterest,
                       VNConfidence minimumConfidence,
                       std::string &identifier,
                       VNConfidence &confidence) const;
    bool detectObjects(const cf::DataPtr &image,
                       const CGRect &regionOfInterest,
                       VNConfidence minimumConfidence,
                       DetectedObjects &objects) const;
    
private:
    static std::tuple<VNCoreMLModel *, NSURL *> loadModel(NSURL *modelURL);
    static VNCoreMLRequest * analyzeImage(VNCoreMLModel *model,
                                          const cf::DataPtr &image,
                                          const CGRect &regionOfInterest);
    static bool classifyImage(VNCoreMLModel *model,
                              const cf::DataPtr &image,
                              const CGRect &regionOfInterest,
                              VNConfidence minimumConfidence,
                              std::string &identifier,
                              VNConfidence &confidence);
    static bool detectObjects(VNCoreMLModel *model,
                              const cf::DataPtr &image,
                              const CGRect &regionOfInterest,
                              VNConfidence minimumConfidence,
                              DetectedObjects &objects);
    static void removeCompiledModel(NSURL *compiledModelURL);
    
    NSURL *compiledModelURL;
    VNCoreMLModel *model;
    
};


END_NAMESPACE_MW


#endif /* ModelManager_hpp */
