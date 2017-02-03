/**
 * Stimulus.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/23/05 - Added Copy constructor and destructor. Fixed
 *                          spacing.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "Stimulus.h"
#include "StandardVariables.h"
//#include "Utilities.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"

#include "ComponentRegistry.h"


BEGIN_NAMESPACE_MW


#define VERBOSE_STIMULI 0


void StimulusGroup::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.setSignature("stimulus_group");
    info.addParameter(TAG);  // Make tag required
}


StimulusGroup::StimulusGroup(const ParameterValueMap &parameters) :
    Component(parameters)
{ }


void StimulusGroup::addSubGroup(const shared_ptr <StimulusGroup> stim_group) {
	sub_groups.push_back(stim_group);
}

void StimulusGroup::setParent(weak_ptr <StimulusGroup> _parent) {
	parent = _parent;
}

unsigned int StimulusGroup::dimensionSize(const unsigned int dimension) {
	// this function assumes that the dimensionality of the group is perfect
	//  (each group has the same number of sub groups)
	
	if(dimension == 0) {
		if(sub_groups.size() > 0) {
			return sub_groups.size();
		} else {
			return getNElements();
		}	
	} else {
		shared_ptr<StimulusGroup> sub_group = this->getSubGroup(0);
		for(unsigned int i=0; i<dimension-1; ++i) {
			sub_group = sub_group->getSubGroup(0);
			if(sub_group == 0) {
				throw SimpleException("Illegal stimulus group dimensions");				
			}
		}
	
		if(sub_group->getNDimensions() > 1) {
			return sub_group->dimensionSize(0);
		} else {
			return sub_group->getNElements();
		}	
	}
}

shared_ptr<StimulusGroup> StimulusGroup::getSubGroup(const unsigned int sub_group_index) const {
	shared_ptr<StimulusGroup> return_sub_group = shared_ptr<StimulusGroup>();

	if(sub_group_index < sub_groups.size()) {
		return_sub_group = sub_groups.at(sub_group_index);
	}
	
	return return_sub_group;
}

void StimulusGroup::addStimToParents(const shared_ptr <Stimulus> stim) {
	// c'mon...
	if(parent.use_count() == 0){
		return;
	}
	
	shared_ptr <StimulusGroup> next_parent = shared_ptr<StimulusGroup>(parent);
	
	while(next_parent != 0) {
		next_parent->addReference(stim);
		next_parent = next_parent->getParent();
	}
}

// return a shared pointer to the parent...if there is no parent, return a null shared pointer
shared_ptr<StimulusGroup> StimulusGroup::getParent() const {
	shared_ptr <StimulusGroup> _parent = parent.lock();
	return _parent;
}

unsigned int StimulusGroup::getNDimensions() const {
	// this does some checking to make sure that all of the sub groups 
	// have the same number of dimensions as their siblings.  There should be 
	// no reason to call this during runtime.

	// this recursivley descends through the groups and counts the number of dimensions
	unsigned int number_of_dimensions = 1;

	// all of the subgroups should have the same dimensions
	vector<unsigned int> dimensions_for_each_sub_group;
	
	for(vector<shared_ptr<StimulusGroup> >::const_iterator i = sub_groups.begin();
		i != sub_groups.end();
		++i) {

		shared_ptr<StimulusGroup> sub_group = *i;
		dimensions_for_each_sub_group.push_back(sub_group->getNDimensions());
	}
	
	if(dimensions_for_each_sub_group.size() != sub_groups.size()) {
		throw SimpleException("Illegal stimulus group dimensions");
	}	
	
	for(vector<unsigned int>::const_iterator i = dimensions_for_each_sub_group.begin();
		i != dimensions_for_each_sub_group.end();
		++i) {
		if(*i != dimensions_for_each_sub_group.at(0)) {
			throw SimpleException("Illegal stimulus group dimensions");			
		}
		
		number_of_dimensions = *i + 1;
	}
	
	return number_of_dimensions;
}

void StimulusGroup::addChild(std::map<std::string, std::string> parameters,
							  ComponentRegistry *reg,
							  shared_ptr<mw::Component> child){
	shared_ptr<Stimulus> stim = boost::dynamic_pointer_cast<Stimulus, mw::Component>(child);
	shared_ptr<StimulusGroup> stim_group = boost::dynamic_pointer_cast<StimulusGroup, mw::Component>(child);
	
	if((stim == NULL && stim_group == NULL) || (stim != NULL && stim_group != NULL)){
		// TODO: better exception
		throw SimpleException("Attempt to add non-stimulus object to stimulus group");
	}

	// 2 cases, we're adding a stim group, or adding a stim
	
	if(stim != NULL) {
		addReference(stim);
		addStimToParents(stim);
	} else {
		addSubGroup(stim_group);

		shared_ptr<ComponentRegistry> component_registry = ComponentRegistry::getSharedRegistry();
		shared_ptr <StimulusGroup> stim_group_parent = component_registry->getObject<StimulusGroup>(getTag());
		stim_group->setParent(stim_group_parent);
	}
}


const std::string Stimulus::DEFERRED("deferred");


void Stimulus::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.addParameter(TAG);  // Make tag required
    info.addParameter(DEFERRED, "no");
}


template<>
Stimulus::load_style ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    std::string ds(boost::algorithm::to_lower_copy(s));
    
    if ((ds == "no") || (ds == "0") || (ds == "false")) {
        return Stimulus::nondeferred_load;
    } else if ((ds == "yes") || (ds == "1") || (ds == "true")) {
        return Stimulus::deferred_load;
    } else if (ds == "explicit") {
        return Stimulus::explicit_load;
    } else {
        throw SimpleException("invalid value for parameter \"deferred\"", s);
    }
}


Stimulus::Stimulus(const ParameterValueMap &parameters) :
    Announcable(ANNOUNCE_STIMULUS_TAGNAME),
    mw::Component(parameters),
    loaded(false),
    visible(false),
    deferred(parameters[DEFERRED])
{ }


Stimulus::Stimulus(const std::string &_tag) :
    Announcable(ANNOUNCE_STIMULUS_TAGNAME),
    mw::Component(_tag),
    loaded(false),
    visible(false),
    deferred(nondeferred_load)
{ }


void Stimulus::load(shared_ptr<StimulusDisplay> display) {
    loaded = true;
}

void Stimulus::unload(shared_ptr<StimulusDisplay> display) {
    loaded = false;
}

void Stimulus::setVisible(bool newvis) {
    visible = newvis;
}

void Stimulus::draw(shared_ptr<StimulusDisplay> display) {
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Attempt to draw an undefined stimulus");
}

bool Stimulus::isLoaded() {
    return loaded;
}

bool Stimulus::isVisible() {
    return visible;
}


Datum Stimulus::getCurrentAnnounceDrawData() {
    Datum announceData(M_DICTIONARY, 3);
    
    announceData.addElement(STIM_NAME,getTag());
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_GENERIC);  
    
    return announceData;
}


END_NAMESPACE_MW























