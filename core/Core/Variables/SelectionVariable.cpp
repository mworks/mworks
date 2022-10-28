/*
 *  SelectionVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/1/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "SelectionVariable.h"

#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"
#include "ComponentRegistry.h"
#include "ExpressionVariable.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


SelectionVariable::SelectionVariable(const VariableProperties &props, const boost::shared_ptr<Selection> &sel) :
    ReadOnlyVariable(props),
    selected_index(NO_SELECTION),
    advanceOnAccept(false)
{
    if (sel) {
        attachSelection(sel);
    }
}


Datum SelectionVariable::getTentativeSelection(int index) {
    lock_guard lock(mutex);
    
    if (!selection) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Internal error: selection variable has no selection attached");
        return Datum(0L);
    }
    
    auto &tenativeSelections = selection->getTentativeSelections();
    
    if (tenativeSelections.empty()) {
        // Issue an error message only if the experiment is running.  Otherwise, all selection variable indexing
        // expressions will produce load-time errors (since ParsedExpressionVariable's constructors evaluate the
        // expression to test for validity).
        if (StateSystem::instance(false) && StateSystem::instance()->isRunning()) {
            merror(M_PARADIGM_MESSAGE_DOMAIN, "Selection variable has no tentative selections available.  Returning 0.");
        }
        return Datum(0L);
    }
    
    if ((index < 0) || (index >= tenativeSelections.size())) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Selection variable index (%d) is out of bounds.  Returning 0.", index);
        return Datum(0L);
    }
    
    return values.at(tenativeSelections.at(index));
}


void SelectionVariable::nextValue() {
    lock_guard lock(mutex);
    
    if (!selection) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempt to advance a selection variable with a NULL selection");
        return;
    }
    
    try {
        selected_index = selection->draw();
    } catch (std::exception &e) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "%s", e.what());
        return;
    }
    
    // Announce the new value so that the event stream contains
    // all information about what happened in the experiment
    announce();
    performNotifications(values.at(selected_index));
}


Datum SelectionVariable::getValue() {
    lock_guard lock(mutex);
    
    if (selected_index == NO_SELECTION) {
        nextValue();
    }
    
    if (selected_index == NO_SELECTION) {
        merror(M_PARADIGM_MESSAGE_DOMAIN,
               "Attempt to select a value from a selection variable with no values defined");
        return Datum(0L);
    }
    
    return values.at(selected_index);
}


int SelectionVariable::getNItems() {
    lock_guard lock(mutex);
    return values.size();
}


void SelectionVariable::resetSelections() {
    lock_guard lock(mutex);
    Selectable::resetSelections();
    selected_index = NO_SELECTION;
}


void SelectionVariable::rejectSelections() {
    lock_guard lock(mutex);
    Selectable::rejectSelections();
    nextValue();
}


void SelectionVariable::acceptSelections() {
    lock_guard lock(mutex);
    Selectable::acceptSelections();
    if (advanceOnAccept && (getNLeft() > 0)) {
        nextValue();
    }
}


const std::string SelectionVariableFactory::VALUES("values");
const std::string SelectionVariableFactory::ADVANCE_ON_ACCEPT("advance_on_accept");
const std::string SelectionVariableFactory::AUTORESET("autoreset");


ComponentInfo SelectionVariableFactory::describeComponent() {
    auto info = BaseVariableFactory::describeComponent();
    
    info.setSignature("variable/selection");
    
    info.addParameter(VALUES);
    info.addParameter(ADVANCE_ON_ACCEPT, "NO");
    info.addParameter(AUTORESET, "NO");
    
    info.addParameter(ListState::SELECTION, "sequential");
    info.addParameter(ListState::NSAMPLES, "1");
    info.addParameter(ListState::SAMPLING_METHOD, "cycles");
    
    return info;
}


std::vector<Datum> SelectionVariableFactory::getValues(const ParameterValue &paramValue) {
    std::vector<Datum> values;
    ParsedExpressionVariable::evaluateExpressionList(paramValue.str(), values);
    return values;
}


long SelectionVariableFactory::getNumSamples(const ParameterValue &paramValue,
                                             SampleType sampleType,
                                             const std::vector<Datum> &values)
{
    long numSamples(paramValue);
    if (numSamples < 1) {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "\"nsamples\" must be greater than or equal to 1");
    }
    if (sampleType == M_CYCLES) {
        numSamples *= values.size();
    }
    return numSamples;
}


boost::shared_ptr<Selection> SelectionVariableFactory::getSelection(SelectionType selectionType,
                                                                    long numSamples,
                                                                    bool autoreset)
{
    switch (selectionType) {
        case M_SEQUENTIAL:
        case M_SEQUENTIAL_ASCENDING:
            return boost::make_shared<SequentialSelection>(numSamples, true, autoreset);
            
        case M_SEQUENTIAL_DESCENDING:
            return boost::make_shared<SequentialSelection>(numSamples, false, autoreset);
            
        case M_RANDOM_WITH_REPLACEMENT:
            return boost::make_shared<RandomWithReplacementSelection>(numSamples, autoreset);
            
        case M_RANDOM_WOR:
            return boost::make_shared<RandomWORSelection>(numSamples, autoreset);
    }
}


ComponentPtr SelectionVariableFactory::createVariable(const ParameterValueMap &parameters) const {
    const auto values = getValues(parameters[VALUES]);
    const bool advanceOnAccept(parameters[ADVANCE_ON_ACCEPT]);
    const bool autoreset(parameters[AUTORESET]);
    
    const SelectionType selectionType(parameters[ListState::SELECTION]);
    const auto numSamples = getNumSamples(parameters[ListState::NSAMPLES],
                                          SampleType(parameters[ListState::SAMPLING_METHOD]),
                                          values);
    
    VariableProperties props(Datum(0L),
                             parameters[Component::TAG].str(),
                             M_WHEN_CHANGED,
                             false,
                             getGroups(parameters[GROUPS]),
                             false,
                             getDescription(parameters[DESCRIPTION]));
    
    auto selectionVar = global_variable_registry->createSelectionVariable(props);
    selectionVar->setAdvanceOnAccept(advanceOnAccept);
    selectionVar->attachSelection(getSelection(selectionType, numSamples, autoreset));
    selectionVar->addValues(values);
    return selectionVar;
}


END_NAMESPACE_MW
