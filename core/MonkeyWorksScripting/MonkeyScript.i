%module MonkeyScript

%{
#include "MWorksCore/Selection.h"
#include "MWorksCore/Trigger.h"
#include "MWorksCore/Lockable.h"
#include "MWorksCore/ExpandableList.h"
#include "MWorksCore/GenericData.h"
#include "MWorksCore/ScopedVariableEnvironment.h"
#include "MWorksCore/ScopedVariableContext.h"
#include "MWorksCore/GenericVariable.h"
#include "MWorksCore/GlobalVariable.h"
#include "MWorksCore/ConstantVariable.h"
#include "MWorksCore/ScopedVariable.h"
#include "MWorksCore/ExpressionVariable.h"
#include "MWorksCore/FunctionVariable.h"
#include "MWorksCore/VariableReference.h"
#include "MWorksCore/Selectable.h"
#include "MWorksCore/SequentialSelection.h"
#include "MWorksCore/RandomWORSelection.h"
#include "MWorksCore/RandomWithReplacementSelection.h"
#include "MWorksCore/SelectionVariable.h"
#include "MWorksCore/Experiment.h"
#include "MWorksCore/States.h"

#include "MWorksCore/TrialBuildingBlocks.h"
#include "MWorksCore/VariableRegistry.h"
#include "MWorksCore/VariableProperties.h"
#include "MWorksCore/ScopedVariableContext.h"

#include "MWorksCore/StateSystem.h"
#include "MWorksCore/BlockAndProtocol.h"
#include "MWorksCore/Stimulus.h"
#include "MWorksCore/StimulusDisplay.h"
#include "MWorksCore/StimulusNode.h"
#include "MWorksCore/StandardStimuli.h"
#include "MWorksCore/LoadingUtilities.h"
#include "MWorksCore/SwigBridges.h"
#include "MWorksCore/IODevice.h"
#include "MWorksCore/ExperimentPackager.h"
#include "NIDAQmxBase.h"
#include "MWorksCore/PlatformDependentServices.h"


//#include "MWorksCore/OpenGLContextManager.h"

#include "MWorksCore/OpenALContextManager.h"
#include "MWorksCore/Sound.h"
#include "MWorksCore/StandardSounds.h"

#include "MWorksCore/ITC18_IODevice.h"
//#include "MWorksCore/NIDaqDevice.h"
//#include "MWorksCore/NI_USB6009.h
#include "MWorksCore/FakeMonkey.h"
#include "MWorksCore/Timer.h"
#include "MWorksCore/IODeviceManager.h"
#include "MWorksCore/FilterManager.h"
#include "MWorksCore/FilterTransforms.h"
#include "Averagers.h"
#include "MWorksCore/EyeMonitors.h"
#include "MWorksCore/EyeCalibrators.h"
#include "MWorksCore/Event.h"
#include "MWorksCore/SimpleStaircase.h"
#include "MWorksCore/ExpandableList.h"
#include <boost/shared_ptr.hpp>
%}


//%template(mListState_mProtocol) mListState<mProtocol>;


//%include <boost/shared_ptr.hpp>

%include "ExpandableList.h"

%include "GenericData.h"

%include "ScopedVariableEnvironment.h"
%include "ScopedVariableContext.h"

%include "GenericVariable.h"
%include "ScopedVariable.h"
%include "GlobalVariable.h"
%include "ConstantVariable.h"
%include "ExpressionVariable.h"
%include "VariableReference.h"
%include "FunctionVariable.h"



%include "Selectable.h"
%include "SelectionVariable.h"
%include "SequentialSelection.h"
%include "RandomWORSelection.h"
%include "RandomWithReplacementSelection.h"

%include "Stimulus.h"
%include "StimulusDisplay.h"
%include "StimulusNode.h"
%include "StandardStimuli.h"
%include "LoadingUtilities.h"

%include "Experiment.h"
%include "Selection.h"
%include "Utilities.h"
%include "ExpandableList.h"

%include "Scheduler.h"
%include "StateSystem.h"
%include "Clock.h"

%include "States.h"



//%template(mListStateBlock) mListState<mBlock>;
//%template(mListStateState) mListState<mState>;

%include "BlockAndProtocol.h"

%include "TrialBuildingBlocks.h"

%include "VariableProperties.h"
%include "ScopedVariableContext.h"

%include "VariableRegistry.h"

%include "StateSystem.h"

%include "PlatformDependentServices.h"

%include "SwigBridges.h"

%include "IODevice.h"
%include "FakeMonkey.h"

%include "ITC18_IODevice.h"
%include "ExperimentPackager.h"	

//%include "OpenGLContextManager.h"

%include "OpenALContextManager.h"
%include "Sound.h"
%include "StandardSounds.h"

//%include "NIDAQmxBase.h"
//%include "NIDaqDevice.h"
//%include "NI_USB6009.h"

%include "Timer.h"

%include "Trigger.h"

%include "IODeviceManager.h"
%include "FilterManager.h"
%include "FilterTransforms.h"
%include "Averagers.h"
%include "EyeMonitors.h"
%include "EyeCalibrators.h"

%include "Event.h"
%include "SimpleStaircase.h"


