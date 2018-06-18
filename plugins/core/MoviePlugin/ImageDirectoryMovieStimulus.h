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
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) MW_OVERRIDE;
    
    void load(shared_ptr<StimulusDisplay> display) MW_OVERRIDE;
    void unload(shared_ptr<StimulusDisplay> display) MW_OVERRIDE;
    
    Datum getCurrentAnnounceDrawData() MW_OVERRIDE;
    
private:
    const ParameterValueMap parameters;
    const VariablePtr directoryPath;
    std::string currentDirectoryPath;
    
};


END_NAMESPACE_MW


#endif /* !defined(__MovieStimulusPlugin__ImageDirectoryMovieStimulus__) */
