#ifndef SELECTION_VARIABLE_H_
#define SELECTION_VARIABLE_H_

/*
 *  SelectionVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/1/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <vector>

#include "Selection.h"
#include "Selectable.h"
#include "GenericVariable.h"
#include "ComponentFactory.h"


BEGIN_NAMESPACE_MW


class SelectionVariable : public Selectable, public ReadOnlyVariable {
    
public:
    explicit SelectionVariable(const VariableProperties &props = VariableProperties(),
                               const boost::shared_ptr<Selection> &sel = boost::shared_ptr<Selection>());
    
    void setAdvanceOnAccept(bool val) {
        lock_guard lock(mutex);
        advanceOnAccept = val;
    }
    
    void addValues(const std::vector<Datum> &vals) {
        lock_guard lock(mutex);
        values.insert(values.end(), vals.begin(), vals.end());
        if (selection) {
            resetSelections();
        }
    }
    
    void addValue(const Datum &val) {
        lock_guard lock(mutex);
        values.push_back(val);
        if (selection) {
            resetSelections();
        }
    }
    
    void addValue(const shared_ptr<Variable> &var) {
        if (var) {
            addValue(var->getValue());
        }
    }
    
    Datum getTentativeSelection(int index);
    void nextValue();
    
    // Variable overrides
    Datum getValue() override;
    
    // Selectable overrides
    int getNItems() override;
    void resetSelections() override;
    void rejectSelections() override;
    void acceptSelections() override;
    
private:
    static constexpr int NO_SELECTION = -1;
    
    std::vector<Datum> values;
    int selected_index;
    bool advanceOnAccept;
    
    using lock_guard = std::lock_guard<std::recursive_mutex>;
    lock_guard::mutex_type mutex;
    
};


class SelectionVariableFactory : public BaseVariableFactory {
    
public:
    static const std::string VALUES;
    static const std::string ADVANCE_ON_ACCEPT;
    static const std::string AUTORESET;
    
    static ComponentInfo describeComponent();
    
    SelectionVariableFactory() : BaseVariableFactory(describeComponent()) { }
    
private:
    static std::vector<Datum> getValues(const ParameterValue &paramValue);
    static long getNumSamples(const ParameterValue &paramValue,
                              SampleType sampleType,
                              const std::vector<Datum> &values);
    static boost::shared_ptr<Selection> getSelection(SelectionType selectionType, long numSamples, bool autoreset);
    
    ComponentPtr createVariable(const Map<ParameterValue> &parameters) const override;
    
};


END_NAMESPACE_MW


#endif
