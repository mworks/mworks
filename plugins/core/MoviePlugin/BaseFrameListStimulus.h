//
//  BaseFrameListStimulus.h
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#ifndef __MovieStimulusPlugin__BaseFrameListStimulus__
#define __MovieStimulusPlugin__BaseFrameListStimulus__


BEGIN_NAMESPACE_MW


class BaseFrameListStimulus : public StandardDynamicStimulus {
    
public:
    static const std::string ENDING;
    static const std::string ENDED;
    static const std::string LOOP;
    static const std::string REPEATS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit BaseFrameListStimulus(const ParameterValueMap &parameters);
    
    void freeze(bool shouldFreeze = true) MW_OVERRIDE;
    
    void load(shared_ptr<StimulusDisplay> display) MW_OVERRIDE;
    void unload(shared_ptr<StimulusDisplay> display) MW_OVERRIDE;
    
    void drawFrame(shared_ptr<StimulusDisplay> display) MW_OVERRIDE;
    Datum getCurrentAnnounceDrawData() MW_OVERRIDE;
    
protected:
    void startPlaying() MW_OVERRIDE;
    
    int getFrameNumber();
    int getLastFrameDrawn() const { return lastFrameDrawn; }
    
    virtual int getNumFrames() = 0;
    virtual int getNominalFrameNumber() = 0;
    virtual shared_ptr<Stimulus> getStimulusForFrame(int frameNumber) = 0;
    
private:
    shared_ptr<Variable> ending;
    shared_ptr<Variable> ended;
    shared_ptr<Variable> loop;
    shared_ptr<Variable> repeats;
    
    int lastFrameDrawn;
    
};


END_NAMESPACE_MW


#endif /* !defined(__MovieStimulusPlugin__BaseFrameListStimulus__) */




























