/*
 *  SwigBridges.h
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on 10/11/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "VariableProperties.h"

#include "Experiment.h"
#include "BlockAndProtocol.h"
#include "TrialBuildingBlocks.h"
#include "ConstantVariable.h"
#include "ExpressionVariable.h"
#include "SelectionVariable.h"
#include "FilterManager.h"
#include "VariableTransformAdaptors.h"
#include "StandardSounds.h"
//#include "FunctionVariable.h"
#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"
#include "ScheduledActions.h"
namespace mw {
using namespace boost;

#define	DEF_CAST_SHARED_PTR(X,Y,NAME)	shared_ptr<X> NAME(shared_ptr<Y> ptr){ return dynamic_pointer_cast<X,Y>(ptr); }
#define DEF_GET_RAW(X)	X *get_raw_pointer(shared_ptr<X> ptr){ return ptr.get(); }
#define DEF_SHARED_PTR(X)	shared_ptr<X> create_shared_ptr(X *arg){ return shared_ptr<X>(arg); }
#define DEF_SHARED_PTR_LITERAL(X,Y) shared_ptr<X> create_shared_ptr(Y *arg){ return shared_ptr<X>(arg); }
#define DEF_CAST_RAW_PTR(X, Y, NAME)  X * NAME(Y *arg){ return (X *)arg; }

VariableProperties *createNewVariableProperties(long defaultval, char *_tagname, char *_shortname, 
												 char *_longname, int _editable, bool _viewable, int _domain, int _logging, char *_groups);
							
							
Data createNewDataObject(long data);
Data createNewDataObject(double data);

std::string new_string(const char *the_string){
	if(the_string == NULL){
		return std::string("");
	}
	return std::string(the_string);
}

// yup, this fucker leaks
const char *const_char_string(const std::string &the_string) {
	char *ccs = new char[the_string.length()+1];
	memcpy(ccs, the_string.c_str(), the_string.length()+1);
	return ccs;
}



// ****************************************************************************
// Variable Casting
// ****************************************************************************

DEF_GET_RAW(Variable);
DEF_GET_RAW(ConstantVariable);
DEF_GET_RAW(GlobalVariable);
DEF_GET_RAW(ScopedVariable);
DEF_GET_RAW(ExpressionVariable);
DEF_GET_RAW(SelectionVariable);
DEF_GET_RAW(VariableRegistry);

DEF_SHARED_PTR(Variable);
DEF_SHARED_PTR(VariableNotification);
DEF_SHARED_PTR(ExpressionVariable);

// "Ordinary" casts
DEF_CAST_SHARED_PTR(Variable,	ConstantVariable,		castAsVariablePtr);
DEF_CAST_SHARED_PTR(Variable,	GlobalVariable,		castAsVariablePtr);
DEF_CAST_SHARED_PTR(Variable,	ScopedVariable,		castAsVariablePtr);
DEF_CAST_SHARED_PTR(Variable,	ExpressionVariable,	castAsVariablePtr);
DEF_CAST_SHARED_PTR(Variable,	SelectionVariable,		castAsVariablePtr);
DEF_CAST_SHARED_PTR(Variable,	Timer,					castAsVariablePtr);


// Swig-required gratuitous casts
DEF_CAST_RAW_PTR(Variable,	ConstantVariable, castAsVariable);
DEF_CAST_RAW_PTR(Variable,	GlobalVariable, castAsVariable);
DEF_CAST_RAW_PTR(Variable,	ScopedVariable, castAsVariable);
DEF_CAST_RAW_PTR(Variable,	ExpressionVariable, castAsVariable);
//DEF_CAST_RAW_PTR(Variable,	FunctionVariable, castAsVariable);
DEF_CAST_RAW_PTR(Variable,	SelectionVariable, castAsVariable);
DEF_CAST_RAW_PTR(Variable,	Timer, castAsVariable);
DEF_CAST_RAW_PTR(Variable,	Variable, castAsVariable);

// "Up" casts
DEF_CAST_RAW_PTR(ExpressionVariable, Variable, castVariableAsExpressionVariable);
DEF_CAST_RAW_PTR(SelectionVariable,	Variable, castAsSelectionVariable);
DEF_CAST_RAW_PTR(SelectionVariable,	SelectionVariable, castAsSelectionVariable);
DEF_CAST_RAW_PTR(ScopedVariable, Variable, castAsScopedVariable);
DEF_CAST_SHARED_PTR(ScopedVariable, ScopedVariable, castAsScopedVariablePtr);


shared_ptr<Variable> get_variable(char *tag){
	return GlobalVariableRegistry->getVariable(tag);
}



// ****************************************************************************
// VarTransformAdaptor casting
// ****************************************************************************

DEF_CAST_SHARED_PTR(VarTransformAdaptor,	VarTransformAdaptor,	castAsVarTransformAdaptorPtr);
DEF_CAST_SHARED_PTR(VarTransformAdaptor,	EyeStatusMonitor,	castAsVarTransformAdaptorPtr);
DEF_CAST_SHARED_PTR(VarTransformAdaptor,	EyeStatusMonitorVer1,	castAsVarTransformAdaptorPtr);
DEF_CAST_SHARED_PTR(VarTransformAdaptor,	EyeStatusMonitorVer2,	castAsVarTransformAdaptorPtr);
DEF_CAST_SHARED_PTR(VarTransformAdaptor,	AveragerUser,	castAsVarTransformAdaptorPtr);
DEF_CAST_SHARED_PTR(VarTransformAdaptor,	Filter_LinearFilter1D,	castAsVarTransformAdaptorPtr);
DEF_CAST_SHARED_PTR(VarTransformAdaptor,	Filter_BoxcarFilter1D,	castAsVarTransformAdaptorPtr);

DEF_SHARED_PTR(VarTransformAdaptor);
DEF_SHARED_PTR_LITERAL(VarTransformAdaptor, EyeStatusMonitor);
DEF_SHARED_PTR_LITERAL(VarTransformAdaptor, EyeStatusMonitorVer1);
DEF_SHARED_PTR_LITERAL(VarTransformAdaptor, EyeStatusMonitorVer2);
DEF_SHARED_PTR_LITERAL(VarTransformAdaptor, AveragerUser);
DEF_SHARED_PTR_LITERAL(VarTransformAdaptor, Filter_LinearFilter1D);
DEF_SHARED_PTR_LITERAL(VarTransformAdaptor, Filter_BoxcarFilter1D);



// ****************************************************************************
// IODevice casting
// ****************************************************************************


// ****************************************************************************
// Calibrator et al. casting
// ****************************************************************************

DEF_SHARED_PTR(Calibrator);
DEF_CAST_SHARED_PTR(GoldStandard, FixationPoint,		castAsGoldStandardPtr);
DEF_CAST_SHARED_PTR(FixationPoint, Stimulus,			castAsFixationPointPtr);


// ****************************************************************************
// Paradigm mw::Component casting
// ****************************************************************************

DEF_CAST_SHARED_PTR(State, Action,	castAsStatePtr);
DEF_CAST_SHARED_PTR(State, State,		castAsStatePtr);

DEF_SHARED_PTR(State);
DEF_GET_RAW(State);

DEF_GET_RAW(TaskSystemState);
DEF_GET_RAW(ContainerState);


// "Up" casts
DEF_CAST_RAW_PTR(ContainerState, State,		castStateAsContainer);
DEF_CAST_SHARED_PTR(ContainerState, State,	castStateAsContainerPtr);

DEF_CAST_RAW_PTR(mw::Protocol, State,		castStateAsProtocol);
DEF_CAST_SHARED_PTR(mw::Protocol, State,	castStateAsProtocolPtr);

DEF_CAST_RAW_PTR(Block, State,		castStateAsBlock);
DEF_CAST_SHARED_PTR(Block, State,	castStateAsBlockPtr);

DEF_CAST_RAW_PTR(TaskSystem, State,		castStateAsTaskSystem);
DEF_CAST_SHARED_PTR(TaskSystem, State,	castStateAsTaskSystemPtr);

DEF_CAST_RAW_PTR(TaskSystemState, State,		castStateAsTaskSystemState);
DEF_CAST_SHARED_PTR(TaskSystemState, State,	castStateAsTaskSystemStatePtr);

DEF_CAST_RAW_PTR(Trial, State,		castStateAsTrial);
DEF_CAST_SHARED_PTR(Trial, State,	castStateAsTrialPtr);

DEF_CAST_RAW_PTR(Experiment, State,		castStateAsExperiment);
DEF_CAST_SHARED_PTR(Experiment, State,	castStateAsExperimentPtr);

DEF_CAST_RAW_PTR(GenericListState, State,		castStateAsGenericListState);
DEF_CAST_SHARED_PTR(GenericListState, State,	castStateAsGenericListStatePtr);


DEF_SHARED_PTR(Action);

DEF_GET_RAW(If);
DEF_CAST_SHARED_PTR(If,	Action,	castAsIfAction);

DEF_SHARED_PTR(TransitionCondition);

DEF_GET_RAW(ScheduledActions);


DEF_GET_RAW(ListState)
DEF_GET_RAW(GenericListState)
DEF_GET_RAW(Trial)
DEF_GET_RAW(Block)


// ****************************************************************************
// Selection/Selectable casting
// ****************************************************************************

DEF_CAST_SHARED_PTR(SelectionVariable, Variable,		castAsSelectionVariable)

DEF_CAST_SHARED_PTR(Selectable,	ListState,				castAsSelectablePtr)
DEF_CAST_SHARED_PTR(Selectable,	SelectionVariable,		castAsSelectablePtr)
DEF_CAST_SHARED_PTR(Selectable,	Trial,					castAsSelectablePtr)
DEF_CAST_SHARED_PTR(Selectable,	Block,					castAsSelectablePtr)
DEF_CAST_SHARED_PTR(Selectable,	State,					castAsSelectablePtr)

DEF_GET_RAW(Selectable);
DEF_GET_RAW(Selection);

DEF_SHARED_PTR(Selectable);

DEF_SHARED_PTR(Selection);
DEF_SHARED_PTR(SequentialSelection);
DEF_SHARED_PTR(RandomWORSelection);
DEF_SHARED_PTR(RandomWithReplacementSelection);

DEF_CAST_SHARED_PTR(Selection, SequentialSelection,	castAsSelectionPtr)
DEF_CAST_SHARED_PTR(Selection, RandomWORSelection,	castAsSelectionPtr)
DEF_CAST_SHARED_PTR(Selection, RandomWithReplacementSelection,	castAsSelectionPtr)

// ****************************************************************************
// Timer casting
// ****************************************************************************


DEF_SHARED_PTR(Timer);
DEF_SHARED_PTR(TimeBase);

DEF_GET_RAW(TimeBase);

DEF_CAST_SHARED_PTR(Timer, Variable,	castAsTimerPtr);

// ****************************************************************************
// Stimulus casting
// ****************************************************************************


DEF_SHARED_PTR(Sound);

DEF_SHARED_PTR(StimulusNodeGroup);
DEF_SHARED_PTR(StimulusNode);
DEF_SHARED_PTR(StimulusGroup);
DEF_SHARED_PTR(Stimulus);

DEF_GET_RAW(StimulusGroup);
DEF_GET_RAW(Stimulus);





// ****************************************************************************
// Assorted SWIG failures
// ****************************************************************************

DEF_CAST_RAW_PTR(ScopedVariableEnvironment,  Experiment,  castAsScopedVariableEnvironment);

}






