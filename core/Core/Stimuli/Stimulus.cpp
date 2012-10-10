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

#include "OpenGLContextManager.h"

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
    cached(false),
    has_thumbnail(false),
    thumbnail(NULL),
    deferred(parameters[DEFERRED]),
    frozen(false)
{ }


Stimulus::Stimulus(std::string _tag) :
    Announcable(ANNOUNCE_STIMULUS_TAGNAME),
    mw::Component(_tag)
{
    //mdebug("Stimulus constructor called %x", this);
    loaded = false;
    visible = false;
    cached = false;
    has_thumbnail = false;
    thumbnail = NULL;
    
    deferred = Stimulus::nondeferred_load;
	frozen = false;
}


Stimulus::Stimulus(const Stimulus& copy):
								Announcable((const Announcable&) copy),
								mw::Component((const mw::Component&) copy) {

    loaded = copy.loaded;
    visible = copy.visible;
    cached = copy.cached;
    has_thumbnail = copy.has_thumbnail;
    deferred = copy.deferred;
    if(has_thumbnail) {
        // this should prevent a loop if someone set the thumbnail to
        // be itself
        if(copy.thumbnail == &copy) {
            thumbnail = copy.thumbnail;
        } else {
            thumbnail = new Stimulus(*(copy.thumbnail));
        }
    } else {
        thumbnail = NULL;
    }

//    tag = copy.tag;  
    
	frozen = false;
}


Stimulus::~Stimulus() {
    if((thumbnail != NULL) && (thumbnail != this)) {
        delete thumbnail;
        thumbnail = NULL;
    }
	
}

void Stimulus::load(shared_ptr<StimulusDisplay> display) { }

void Stimulus::unload(shared_ptr<StimulusDisplay> display) { }

void Stimulus::setVisible(bool newvis) {
    visible = newvis;
}

void Stimulus::drawInUnitSquare(shared_ptr<StimulusDisplay> display) { 
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Attempt to draw an undefined stimulus");

}
            
void Stimulus::draw(shared_ptr<StimulusDisplay> display) {
    draw(display, 0,0,1.0, 1.0);
}
            
void Stimulus::draw(shared_ptr<StimulusDisplay> display, float x, float y) {
    draw(display, x,y,1.0, 1.0);
}
            
void Stimulus::draw(shared_ptr<StimulusDisplay> display, float x, float y, 
                                                float sizex, float sizey) {
    glPushMatrix();
    glTranslatef(x-.5, y-.5,0);
    glPushMatrix();
    glScalef(sizex, sizey, 1.0);
    drawInUnitSquare(display);
    glPopMatrix();
    glPopMatrix();                
}
            
void Stimulus::precache(){ }

void Stimulus::drawThumbnail(shared_ptr<StimulusDisplay> display, float x, 
                                        float y, float sizex, float sizey) {
    draw(display, x,y,sizex,sizey);
}

void Stimulus::drawThumbnail(shared_ptr<StimulusDisplay> display, float x, float y) {
    draw(display, x,y,1.0,1.0);
}

int * Stimulus::getBounds() { return NULL; }

bool Stimulus::isLoaded() {
    return loaded;
}

bool Stimulus::isVisible() {
    return visible;
}
        
bool Stimulus::isCached() {
    return cached;
}

bool Stimulus::hasThumbnail() {
    return has_thumbnail;
}

Stimulus * Stimulus::getThumbnail() {
    return thumbnail;
}



/* The announceStimulusDraw method is called during update of the stimulus display chain
 *  for each visible stimulus node object in the chain as its draw method is called.
 *  NOTE:  the time of this "announcement" is the time of the updateDisplay method.  As of July 2006
 *   that time can lead the true appearance of the stimulus by more than 1 frame.
 *  We use the term "draw" rather than "on" because the latter implies a physical stimulus apperance.
 *   JJD TODO:  This must be corrected for with an openGL fence and monitoring of the vert refresh.
*/
void Stimulus::announceStimulusDraw(MWTime now) {
    if (VERBOSE_STIMULI>0)  mprintf("Stim draw announce just triggered...");
    Datum announceData = getCurrentAnnounceDrawData();  // will override this
    announce(announceData, now);    // announce things here using method from Announcable
    
}

/*void Stimulus::announceStimulusDraw(char *groupName, int idx) {
    if (VERBOSE_STIMULI>0) mprintf("Stim draw announce just triggered from group...");
    Datum announceData = getCurrentAnnounceDrawData();  // will override this
    announceData.addElement(STIM_GROUP_NAME,groupName);
    announceData.addElement(STIM_GROUP_INDEX,(long)idx);
    announce(announceData);    // announce things here using method from Announcable
    
}

void Stimulus::announceStimulusErase() { 
    if (VERBOSE_STIMULI>0)  mprintf("Stim erase announce just triggered...");
    Datum announceData = getCurrentAnnounceEraseData();  // will override this
    announce(announceData);    // announce things here using method from Announcable
   
}

void Stimulus::announceStimulusErase(char *groupName, int idx) { 
    if (VERBOSE_STIMULI>0) mprintf("Stim erase announce just triggered from group...");
    Datum announceData = getCurrentAnnounceEraseData();  // will override this
    announceData.addElement(STIM_GROUP_NAME,groupName);
    announceData.addElement(STIM_GROUP_INDEX,(long)idx);
    announce(announceData);    // announce things here using method from Announcable
   
}*/



/* method to fill the fields of the announceData object (M_DICTIONARY) 
 *  This is the base class version.  It should be overriden for each new type of 
 *  stimulus object to add more info.
 *  The stimulus object should be able to uniquely describe itself in this variable.
 *  In the future, this should involve a codec and maybe just an index value here along
 *  with things that may change every presentation (position, scale, etc.)
*/
    
Datum Stimulus::getCurrentAnnounceDrawData() {
    
    Datum announceData(M_DICTIONARY, 3);
    announceData.addElement(STIM_NAME,getTag());        // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE,STIM_TYPE_GENERIC);  
    
    return (announceData);
}


/*Data Stimulus::getCurrentAnnounceEraseData() {

    Datum announceData(M_DICTIONARY, 2);
    announceData.addElement(STIM_NAME,tag);       // char
    announceData.addElement(STIM_ACTION,STIM_ACTION_ERASE);
    
    return (announceData);
}*/


END_NAMESPACE_MW























