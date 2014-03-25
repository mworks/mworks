//
//  ImageDirectoryMovieStimulus.cpp
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#include "ImageDirectoryMovieStimulus.h"

#include <algorithm>


BEGIN_NAMESPACE_MW


const std::string ImageDirectoryMovieStimulus::DIRECTORY_PATH("directory_path");


void ImageDirectoryMovieStimulus::describeComponent(ComponentInfo &info) {
    BaseMovieStimulus::describeComponent(info);
    info.setSignature("stimulus/image_directory_movie");
    info.addParameter(DIRECTORY_PATH);
    
    // Add all ImageStimulus parameters except "path"
    ComponentInfo imageInfo;
    ImageStimulus::describeComponent(imageInfo);
    const ParameterInfoMap &imageParams = imageInfo.getParameters();
    for (ParameterInfoMap::const_iterator iter = imageParams.begin(); iter != imageParams.end(); iter++) {
        const std::string &name = (*iter).first;
        if (name != ImageStimulus::PATH) {
            info.addParameter(name, (*iter).second);
        }
    }
}


ImageDirectoryMovieStimulus::ImageDirectoryMovieStimulus(const ParameterValueMap &parameters) :
    BaseMovieStimulus(parameters),
    directoryPath(parameters[DIRECTORY_PATH].as<boost::filesystem::path>().string())
{
    std::vector<std::string> imageFilePaths;
    getFilePaths(directoryPath, imageFilePaths);
    std::sort(imageFilePaths.begin(), imageFilePaths.end());
    
    ComponentRegistryPtr reg = parameters[DIRECTORY_PATH].getRegistry();
    
    for (size_t i = 0; i < imageFilePaths.size(); i++) {
        std::string imageTag((boost::format("%1%_frame_%2%") % getTag() % i).str());
        
        ParameterValueMap imageParams(parameters);
        imageParams.at(TAG) = ParameterValue(imageTag, reg);
        imageParams.insert(std::make_pair(ImageStimulus::PATH, ParameterValue(imageFilePaths[i], reg)));
        
        images.push_back(shared_ptr<ImageStimulus>(new ImageStimulus(imageParams)));
    }
}


Datum ImageDirectoryMovieStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    Datum announceData = BaseMovieStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "image_directory_movie");
    announceData.addElement(DIRECTORY_PATH, directoryPath);
    
    return announceData;
}


END_NAMESPACE_MW


























