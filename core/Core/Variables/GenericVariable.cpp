/**
 * GenericVariable.cpp
 *
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 01/27/05 - Fixed spacing added codec code implementation.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "GenericVariable.h"
#include "ExpressionVariable.h"
#include "GlobalVariable.h"
#include "ScopedVariable.h"
#include "Experiment.h"
#include "Event.h"
#include "EventBuffer.h"
#include <boost/algorithm/string/case_conv.hpp>
#include "ComponentRegistry.h"
#include "EventBuffer.h"
#include "TrialBuildingBlocks.h"


BEGIN_NAMESPACE_MW


Variable::Variable(const VariableProperties &properties) :
    properties(properties),
    codec_code(-1),
    logging(properties.getLogging()),
    event_target(global_outgoing_event_buffer)
{
    setTag(properties.getTagName());
}


void Variable::addChild(std::map<std::string, std::string> parameters,
                        ComponentRegistryPtr reg,
                        ComponentPtr child)
{
    auto act = boost::dynamic_pointer_cast<Action>(child);
    if (!act) {
        throw SimpleException("Attempting to add illegal object (" + child->getTag() + ") to variable (" + getVariableName() + ")");
    }
    addNotification(boost::make_shared<ActionVariableNotification>(act));
}


void Variable::addNotification(const boost::shared_ptr<VariableNotification> &notif) {
    if (!notif) {
        mwarning(M_PARADIGM_MESSAGE_DOMAIN, "Attempt to add a null notification to a variable");
        return;
    }
    notifications.addToBack(notif);
}


void Variable::performNotifications(const Datum &value, MWTime when, bool silent) {
    {
        Locker locker(notifications);
        auto node = notifications.getFrontmost();
        while (node) {
            node->notify(value, when);
            node = node->getNext();
        }
    }
    
    if (!silent) {
        announce(value, when);
    }
}


void Variable::announce(const Datum &value, MWTime when) const {
    if (properties.getLogging() == M_WHEN_CHANGED && event_target) {
        event_target->putEvent(boost::make_shared<Event>(codec_code, when, value));
    }
}


const std::string BaseVariableFactory::GROUPS("groups");
const std::string BaseVariableFactory::DESCRIPTION("description");


ComponentInfo BaseVariableFactory::describeComponent() {
    ComponentInfo info;
    
    info.addParameter(Component::TAG);
    info.addParameter(GROUPS, false);
    
    info.addParameter(DESCRIPTION, false);
    info.addParameterAlias(DESCRIPTION, "desc");
    
    return info;
}


ComponentPtr BaseVariableFactory::createObject(StdStringMap parameters, ComponentRegistryPtr reg) {
    ParameterValueMap values;
    processParameters(parameters, reg, values);
    return createVariable(values);
}


std::string BaseVariableFactory::getGroups(const ParameterValue &paramValue) {
    const auto groups = boost::algorithm::trim_copy(paramValue.str());
    if (groups.empty()) {
        return EXPERIMENT_DEFINED_VARIABLES;
    }
    return (EXPERIMENT_DEFINED_VARIABLES ",") + groups;
}


std::string BaseVariableFactory::getDescription(const ParameterValue &paramValue) {
    if (auto description = optionalVariableOrText(paramValue)) {
        return description->getValue().getString();
    }
    return "";
}


const std::string VariableFactory::DEFAULT_VALUE("default_value");
const std::string VariableFactory::TYPE("type");
const std::string VariableFactory::SCOPE("scope");
const std::string VariableFactory::LOGGING("logging");
const std::string VariableFactory::PERSISTENT("persistent");
const std::string VariableFactory::EXCLUDE_FROM_DATA_FILE("exclude_from_data_file");


ComponentInfo VariableFactory::describeComponent() {
    auto info = BaseVariableFactory::describeComponent();
    
    info.setSignature("variable");
    
    info.addParameter(DEFAULT_VALUE);
    info.addParameter(TYPE, "any");
    info.addParameter(SCOPE, "global");
    info.addParameter(LOGGING, "when_changed");
    
    info.addParameter(PERSISTENT, "NO");
    info.addParameterAlias(PERSISTENT, "saved");
    // "persistant" is too embarrassing to document, but it was the one correct name
    // for well over a decade and will be present in experiments for a long time
    info.addParameterAlias(PERSISTENT, "persistant");
    
    info.addParameter(EXCLUDE_FROM_DATA_FILE, "NO");
    
    // "editable" is no longer used but is still present in old experiments
    info.addIgnoredParameter("editable");
    
    return info;
}


GenericDataType VariableFactory::getType(const ParameterValue &paramValue) {
    const auto type = boost::algorithm::to_lower_copy(paramValue.str());
    if (type.empty() || type == "any") {
        return M_UNDEFINED;
    } else if (type == "integer") {
        return M_INTEGER;
    } else if (type == "float" || type == "double") {
        return M_FLOAT;
    } else if (type == "boolean") {
        return M_BOOLEAN;
    } else if (type == "string") {
        return M_STRING;
    } else if (type == "list") {
        return M_LIST;
    } else {
        throw InvalidAttributeException(TYPE, type);
    }
}


Datum VariableFactory::getDefaultValue(const ParameterValue &paramValue, GenericDataType type) {
    auto &defaultValue = paramValue.str();
    switch (type) {
        case M_UNDEFINED:
            // Type is "any", so let the expression parser infer the type
            return ParsedExpressionVariable::evaluateExpression(defaultValue);
            
        case M_INTEGER:
        case M_FLOAT:
        case M_BOOLEAN:
            return paramValue.getRegistry()->getNumber(defaultValue, type);
            
        case M_STRING:
            return Datum(defaultValue);
            
        case M_LIST: {
            std::vector<Datum> values;
            ParsedExpressionVariable::evaluateExpressionList(defaultValue, values);
            return Datum(std::move(values));
        }
            
        default:
            throw InvalidAttributeException(DEFAULT_VALUE, defaultValue);
    }
}


WhenType VariableFactory::getLogging(const ParameterValue &paramValue) {
    const auto logging = boost::algorithm::to_lower_copy(paramValue.str());
    if (logging == "when_changed") {
        return M_WHEN_CHANGED;
    } else if (logging == "never") {
        return M_NEVER;
    } else {
        throw InvalidAttributeException(LOGGING, logging);
    }
}


auto VariableFactory::getScope(const ParameterValue &paramValue) -> Scope {
    const auto scope = boost::algorithm::to_lower_copy(paramValue.str());
    if (scope == "global") {
        return Scope::Global;
    } else if (scope == "local") {
        return Scope::Local;
    } else {
        throw InvalidAttributeException(SCOPE, scope);
    }
}


ComponentPtr VariableFactory::createVariable(const ParameterValueMap &parameters) const {
    VariableProperties props(getDefaultValue(parameters[DEFAULT_VALUE], getType(parameters[TYPE])),
                             parameters[Component::TAG].str(),
                             getLogging(parameters[LOGGING]),
                             bool(parameters[PERSISTENT]),
                             getGroups(parameters[GROUPS]),
                             bool(parameters[EXCLUDE_FROM_DATA_FILE]),
                             getDescription(parameters[DESCRIPTION]));
    
    if (getScope(parameters[SCOPE]) == Scope::Local) {
        return global_variable_registry->createScopedVariable(GlobalCurrentExperiment, props);
    }
    return global_variable_registry->createGlobalVariable(props);
}


END_NAMESPACE_MW
