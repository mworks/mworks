//
//  AudioEngineSound.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/21.
//

#ifndef AudioEngineSound_hpp
#define AudioEngineSound_hpp

#include <AVFAudio/AVAudioEngine.h>
#include <AVFAudio/AVAudioMixerNode.h>

#include <boost/noncopyable.hpp>

#include "Sound.h"


BEGIN_NAMESPACE_MW


class AudioEngineSound : public Sound, boost::noncopyable {
    
public:
    static const std::string VOLUME;
    static const std::string PAN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AudioEngineSound(const ParameterValueMap &parameters);
    ~AudioEngineSound();
    
    void load() override;
    void unload() override;
    
    void play() override;
    void play(MWTime startTime) override;
    void pause() override;
    void stop() override;
    
protected:
    using lock_guard = std::lock_guard<std::mutex>;
    
    lock_guard acquireLock() const { return lock_guard(mutex); }
    void didStopPlaying() { playing = paused = pausedWithStateSystem = false; }
    
    virtual id<AVAudioMixing> load(AVAudioEngine *engine) = 0;
    virtual void unload(AVAudioEngine *engine) = 0;
    
    virtual bool startPlaying(MWTime startTime) = 0;
    virtual bool stopPlaying() = 0;
    virtual bool beginPause() = 0;
    virtual bool endPause() = 0;
    
    virtual void setCurrentAnnounceData(Datum::dict_value_type &announceData) const { }
    
private:
    using mutex_type = lock_guard::mutex_type;
    using unique_lock = std::unique_lock<mutex_type>;
    
    void volumeCallback(const Datum &data, MWorksTime time);
    void panCallback(const Datum &data, MWorksTime time);
    void stateSystemModeCallback(const Datum &data, MWorksTime time);
    
    void setCurrentVolume(const Datum &data);
    void applyCurrentVolume() { mixer.volume = currentVolume; }
    void setCurrentPan(const Datum &data);
    void applyCurrentPan() { mixer.pan = currentPan; }
    
    enum class Action { Play, Pause, Resume, Stop };
    static const char * getActionName(Action action);
    void announceAction(Action action, MWTime startTime = 0) const;
    
    struct EngineManager : boost::noncopyable {
        EngineManager();
        ~EngineManager();
        AVAudioEngine * getEngine(unique_lock &lock) const {
            lock = unique_lock(mutex);
            return engine;
        }
    private:
        void stateSystemModeCallback(const Datum &data, MWorksTime time);
        AVAudioEngine *engine;
        AVAudioNode *dummyNode;
        boost::shared_ptr<VariableNotification> stateSystemModeNotification;
        mutable mutex_type mutex;
    };
    
    static boost::shared_ptr<EngineManager> getEngineManager();
    
    const VariablePtr volume;
    const VariablePtr pan;
    const boost::shared_ptr<EngineManager> engineManager;
    
    id<AVAudioMixing> mixer;
    double currentVolume;
    double currentPan;
    
    bool loaded;
    bool running;
    bool playing;
    bool paused;
    bool pausedWithStateSystem;
    
    boost::shared_ptr<VariableNotification> volumeNotification;
    boost::shared_ptr<VariableNotification> panNotification;
    boost::shared_ptr<VariableNotification> stateSystemModeNotification;
    
    mutable mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* AudioEngineSound_hpp */
