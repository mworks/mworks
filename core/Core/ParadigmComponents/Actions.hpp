//
//  Actions.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/20/23.
//

#ifndef Actions_hpp
#define Actions_hpp

#include "Experiment.h"
#include "IODevice.h"
#include "Timer.h"
#include "SelectionVariable.h"
#include "ExpressionVariable.h"
#include "EyeCalibrators.h"
#include "Sound.h"
#include "ComponentRegistry.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"
#include "VariableAssignment.hpp"


BEGIN_NAMESPACE_MW


class Action : public State {
    
public:
    Action() { }
    explicit Action(const ParameterValueMap &parameters);
    
    void action() override;
    
private:
    virtual bool execute() = 0;
    
};


class ContainerAction : public Action {
    
public:
    explicit ContainerAction(const ParameterValueMap &parameters);
    
    void action() override;
    
protected:
    void addAction(const boost::shared_ptr<Action> &action);
    boost::shared_ptr<Action> getNextAction();
    
private:
    // execute() is a no-op and can't be overridden.
    // Subclasses must override action() and/or next().
    bool execute() final { return false; }
    
    std::vector<boost::shared_ptr<Action>> actions;
    std::size_t currentActionIndex;
    
};


class ActionVariableNotification : public VariableNotification {
    
public:
    explicit ActionVariableNotification(const boost::shared_ptr<Action> &action);
    
    void notify(const Datum &data, MWTime time) override;
    
private:
    const boost::weak_ptr<Action> weakAction;
    
};


class Assignment : public Action {
    
public:
    static const std::string VARIABLE;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Assignment(const ParameterValueMap &parameters);
    Assignment(const VariablePtr &var, const VariablePtr &value);
    
    bool execute() override;
    
private:
    const VariableAssignment assignment;
    const VariablePtr value;
    
};


class ReportString : public Action {
    
public:
    static const std::string MESSAGE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ReportString(const Map<ParameterValue> &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr message;
    const boost::shared_ptr<Clock> clock;
    
};


class AssertionAction : public Action {
    
public:
    static const std::string CONDITION;
    static const std::string STOP_ON_FAILURE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AssertionAction(const Map<ParameterValue> &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr condition;
    VariablePtr message;
    const bool stopOnFailure;
    
};


class NextVariableSelection : public Action {
	
protected:
	
	shared_ptr<SelectionVariable> variable;
	
public:
	
	NextVariableSelection(shared_ptr<SelectionVariable> _var){
		variable = _var;
	}
	
	virtual bool execute(){
		//mprintf("Executing next selection");
		if(variable == NULL){
			merror(M_PARADIGM_MESSAGE_DOMAIN, 
				   "Attempt to update an invalid or nonexistent selection variable.");
			return false;
		}
		variable->nextValue();
		return true;
	}
};

class NextVariableSelectionFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class SetTimeBase : public Action {
	
protected:
	
	shared_ptr<TimeBase> timebase;
	
public:
	
	SetTimeBase(shared_ptr<TimeBase> _timebase);
	virtual bool execute();
};

class SetTimeBaseFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class TimerAction : public Action {
    
public:
    static const std::string DURATION;
    static const std::string DURATION_UNITS;
    static const std::string TIMEBASE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TimerAction(const ParameterValueMap &parameters);
    
protected:
    MWTime getExpirationTime() const;
    
private:
    VariablePtr duration;
    MWTime durationUnitsToUS;
    boost::shared_ptr<TimeBase> timebase;
    
};


class StartTimer : public TimerAction {
    
public:
    static const std::string TIMER;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit StartTimer(const ParameterValueMap &parameters);
    
    bool execute() override;
    
private:
    boost::shared_ptr<Timer> timer;
    
};


class Wait : public TimerAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit Wait(const ParameterValueMap &parameters);
    
    bool execute() override;
    weak_ptr<State> next() override;
    
private:
    MWTime expirationTime;
    
};


class WaitForCondition : public Action {
    
public:
    static const std::string CONDITION;
    static const std::string TIMEOUT;
    static const std::string TIMEOUT_MESSAGE;
    static const std::string STOP_ON_TIMEOUT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit WaitForCondition(const ParameterValueMap &parameters);
    
    bool execute() override;
    boost::weak_ptr<State> next() override;
    
private:
    const VariablePtr condition;
    const VariablePtr timeout;
    VariablePtr timeoutMessage;
    const bool stopOnTimeout;
    
    const boost::shared_ptr<Clock> clock;
    MWTime deadline;
    
};


class StimulusAction : public Action {
    
public:
    static const std::string STIMULUS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit StimulusAction(const ParameterValueMap &parameters);
    
    bool execute() override;
    
private:
    virtual void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) = 0;
    
    const StimulusNodePtr stimNode;
    
};


class LoadStimulus : public StimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit LoadStimulus(const ParameterValueMap &parameters);
    
private:
    void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) override;
    
};


class UnloadStimulus : public StimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit UnloadStimulus(const ParameterValueMap &parameters);
    
private:
    void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) override;
    
};


class QueueStimulus : public StimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit QueueStimulus(const ParameterValueMap &parameters);
    
private:
    void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) override;
    
};


class LiveQueueStimulus : public StimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit LiveQueueStimulus(const ParameterValueMap &parameters);
    
private:
    void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) override;
    
};


class DequeueStimulus : public StimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit DequeueStimulus(const ParameterValueMap &parameters);
    
private:
    void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) override;
    
};


class BringStimulusToFront : public StimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit BringStimulusToFront(const ParameterValueMap &parameters);
    
private:
    void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) override;
    
};


class SendStimulusToBack : public StimulusAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit SendStimulusToBack(const ParameterValueMap &parameters);
    
private:
    void performAction(const StimulusNodePtr &stimNode, const StimulusDisplayPtr &display) override;
    
};


class StimulusDisplayAction : public Action {
    
public:
    static const std::string DISPLAY;
    static const std::string PREDICTED_OUTPUT_TIME;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit StimulusDisplayAction(const ParameterValueMap &parameters);
    
    bool execute() override;
    boost::weak_ptr<State> next() override;
    
protected:
    virtual boost::shared_ptr<StimulusDisplay::UpdateInfo> performAction() = 0;
    virtual const char * getActionName() const = 0;
    
    const boost::shared_ptr<StimulusDisplay> display;
    const VariablePtr predictedOutputTime;
    const boost::shared_ptr<Clock> clock;
    
    MWTime startTime;
    boost::shared_ptr<StimulusDisplay::UpdateInfo> updateInfo;
    
};


class UpdateStimulusDisplay : public StimulusDisplayAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit UpdateStimulusDisplay(const ParameterValueMap &parameters);
    
private:
    boost::shared_ptr<StimulusDisplay::UpdateInfo> performAction() override;
    const char * getActionName() const override;
    
};


class ClearStimulusDisplay : public StimulusDisplayAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit ClearStimulusDisplay(const ParameterValueMap &parameters);
    
private:
    boost::shared_ptr<StimulusDisplay::UpdateInfo> performAction() override;
    const char * getActionName() const override;
    
};


class SoundAction : public Action {
    
public:
    static const std::string SOUND;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit SoundAction(const ParameterValueMap &parameters);
    
protected:
    const SoundPtr sound;
    
};


class LoadSound : public SoundAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit LoadSound(const ParameterValueMap &parameters);
    
    bool execute() override;
    
};


class UnloadSound : public SoundAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit UnloadSound(const ParameterValueMap &parameters);
    
    bool execute() override;
    
};


class PlaySound : public SoundAction {
    
public:
    static const std::string START_TIME;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit PlaySound(const ParameterValueMap &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr startTime;
    
};


class PauseSound : public SoundAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit PauseSound(const ParameterValueMap &parameters);
    
    bool execute() override;
    
};


class StopSound : public SoundAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit StopSound(const ParameterValueMap &parameters);
    
    bool execute() override;
    
};


class StartDeviceIO : public Action {
protected:
	weak_ptr<IODevice> device;
public:
	StartDeviceIO(shared_ptr<IODevice> _device);
	virtual ~StartDeviceIO();
	virtual bool execute();
};

class StartDeviceIOFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

class StopDeviceIO : public Action {
protected:
	weak_ptr<IODevice> device;
public:
	StopDeviceIO(shared_ptr<IODevice> _device);
	virtual ~StopDeviceIO();
	virtual bool execute();
};

class StopDeviceIOFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class ResetSelection : public Action {
	
protected:
	boost::weak_ptr<Selectable> weakSelectable;
	
public:
	
	ResetSelection(shared_ptr<Selectable> sel){
		weakSelectable = sel;
	}
	
	
	virtual bool execute(){
		if (auto selectable = weakSelectable.lock()) {
            selectable->resetSelections();
		}
		return true;
	}
};

class ResetSelectionFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

class AcceptSelections : public Action {
	
protected:
    boost::weak_ptr<Selectable> weakSelectable;
	
public:
	
	AcceptSelections(shared_ptr<Selectable> sel){
		weakSelectable = sel;
	}
	
	
	virtual bool execute(){
		if (auto selectable = weakSelectable.lock()) {
            selectable->acceptSelections();
		}
		return true;
	}
};

class AcceptSelectionsFactory : public ComponentFactory{	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class RejectSelections : public Action {
	
protected:
	boost::weak_ptr<Selectable> weakSelectable;
	
public:
	
	RejectSelections(shared_ptr<Selectable> sel){
		weakSelectable = sel;
	}
	
	virtual bool execute(){
		if (auto selectable = weakSelectable.lock()) {
            selectable->rejectSelections();
		}
		return true;
	}
};

class RejectSelectionsFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class If : public ContainerAction {
    
public:
    static const std::string CONDITION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit If(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    void action() override;
    boost::weak_ptr<State> next() override;
    
    bool getDidExecute() const { return didExecute; }
    
private:
    const VariablePtr condition;
    bool shouldExecute;
    bool didExecute;
    
};


class Else : public ContainerAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit Else(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    boost::weak_ptr<State> next() override;
    
};


class IfElse : public ContainerAction {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit IfElse(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    void action() override;
    boost::weak_ptr<State> next() override;
    
private:
    bool haveElse;
    boost::shared_ptr<If> currentIf;
    
};


class While : public ContainerAction {
    
public:
    static const std::string CONDITION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit While(const ParameterValueMap &parameters);
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<Component> child) override;
    
    void action() override;
    boost::weak_ptr<State> next() override;
    
private:
    const VariablePtr condition;
    bool shouldExecute;
    
};


class StopExperiment : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit StopExperiment(const ParameterValueMap &parameters);
    virtual ~StopExperiment() { }
    
    virtual bool execute();
    
};


class PauseExperiment : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit PauseExperiment(const ParameterValueMap &parameters);
    virtual ~PauseExperiment() { }
    
    virtual bool execute();
    
};


class ResumeExperiment : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit ResumeExperiment(const ParameterValueMap &parameters);
    virtual ~ResumeExperiment() { }
    
    virtual bool execute();
    
};


// action to cause the calibrator object to take a calibration value immediately
// the calibrator class has methods to get values for registered varaibles and 
//  call goldStandard objects to get true values
class TakeCalibrationSampleNow : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
	shared_ptr<GoldStandard> goldStandard;     // object that contains the "true" values
public:
	TakeCalibrationSampleNow(shared_ptr<Calibrator> _calibrator, 
							  shared_ptr<GoldStandard> _goldStandard);
	virtual ~TakeCalibrationSampleNow();
	virtual bool execute();
};

class TakeCalibrationSampleNowFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


class StartAverageCalibrationSample : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
public:
	StartAverageCalibrationSample(shared_ptr<Calibrator> _calibrator);
	virtual ~StartAverageCalibrationSample();
	virtual bool execute();
};

class StartAverageCalibrationSampleFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

class EndAverageAndTakeCalibrationSample : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
	shared_ptr<GoldStandard> goldStandard;     // object that contains the "true" values
public:
	EndAverageAndTakeCalibrationSample(shared_ptr<Calibrator> _calibrator, 
										shared_ptr<GoldStandard> _goldStandard);
	virtual ~EndAverageAndTakeCalibrationSample();
	virtual bool execute();
};

class EndAverageAndTakeCalibrationSampleFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

class EndAverageAndIgnore : public Action {
protected:
	shared_ptr<Calibrator> calibrator;       // some kind of calibrator 
public:
	EndAverageAndIgnore(shared_ptr<Calibrator> _calibrator);
	virtual ~EndAverageAndIgnore();
	virtual bool execute();
};

class EndAverageAndIgnoreFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


// action to compute the calibration parameters and immediately begin using them
class CalibrateNow : public Action {
protected:
	shared_ptr<Calibrator> calibrator;
public:
	CalibrateNow(shared_ptr<Calibrator> _calibrator);
	virtual ~CalibrateNow();
	virtual bool execute();     // false if calibration could not be completed
};

class CalibrateNowFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

// action to compute the calibration parameters and immediately begin using them
class ClearCalibration : public Action {
protected:
	shared_ptr<Calibrator> calibrator;
public:
	ClearCalibration(shared_ptr<Calibrator> _calibrator);
	virtual ~ClearCalibration();
	virtual bool execute();
};

class ClearCalibrationFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};



// ==============================================
// averager controler actions -- Added May 7, 2006 JJD
// ==============================================
class StartAverager : public Action {
protected:
	shared_ptr<AveragerUser> averager;
public:
	StartAverager(shared_ptr<AveragerUser> _averager);
	virtual ~StartAverager();
	virtual bool execute();
};

class StartAveragerFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

class StopAverager : public Action {
protected:
	shared_ptr<AveragerUser> averager;
public:
	StopAverager(shared_ptr<AveragerUser> _averager);
	virtual ~StopAverager();
	virtual bool execute();
};

class StopAveragerFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

class ClearAverager : public Action {
protected:
	shared_ptr<AveragerUser> averager;
public:
	ClearAverager(shared_ptr<AveragerUser> _averager);
	virtual ~ClearAverager();
	virtual bool execute();
};

class ClearAveragerFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};


END_NAMESPACE_MW


#endif /* Actions_hpp */
