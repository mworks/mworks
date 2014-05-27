//
//  ImageDirectoryMovieStimulus.h
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#ifndef __MovieStimulusPlugin__ImageDirectoryMovieStimulus__
#define __MovieStimulusPlugin__ImageDirectoryMovieStimulus__

#include "BaseMovieStimulus.h"


BEGIN_NAMESPACE_MW


class ImageDirectoryMovieStimulus : public BaseMovieStimulus {
    
public:
    static const std::string DIRECTORY_PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ImageDirectoryMovieStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() MW_OVERRIDE;
    
protected:
    int getNumFrames() MW_OVERRIDE {
        return images.size();
    }
    
    shared_ptr<Stimulus> getStimulusForFrame(int frameNumber) MW_OVERRIDE {
        return images[frameNumber];
    }
    
private:
    const std::string directoryPath;
    std::vector< shared_ptr<Stimulus> > images;
    
};


END_NAMESPACE_MW


#endif /* !defined(__MovieStimulusPlugin__ImageDirectoryMovieStimulus__) */
