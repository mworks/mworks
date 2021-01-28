//
//  ImageFileStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/20/21.
//

#ifndef ImageFileStimulus_hpp
#define ImageFileStimulus_hpp

#include "BaseImageStimulus.hpp"


BEGIN_NAMESPACE_MW


class ImageFileStimulus : public BaseImageStimulus {
    
public:
    static const std::string PATH;
    static const std::string ANNOUNCE_LOAD;
    
    static cf::ObjectPtr<CGImageSourceRef> loadImageFile(const std::string &filename,
                                                         std::string &fileHash,
                                                         bool announce = true);
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ImageFileStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
private:
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    
    double getAspectRatio() const override;
    
    const VariablePtr path;
    const bool announceLoad;
    
    std::string filename;
    std::string fileHash;
    
    std::size_t width;
    std::size_t height;
    
};


END_NAMESPACE_MW


#endif /* ImageFileStimulus_hpp */
