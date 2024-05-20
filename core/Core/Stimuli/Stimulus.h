/**
 * Stimulus.h
 *
 * Copyright 2005 MIT.  All rights reserved.
 */
 
#ifndef STIMULUS_H
#define STIMULUS_H

#include "ExpandableList.h"
#include "Announcers.h"
#include "FreezableVariableContainer.h"
#include "StimulusDisplay.h"


BEGIN_NAMESPACE_MW


class ComponentInfo;
class ParameterValue;


class Stimulus : public mw::Component, public FreezableCollection {
    
public:
    enum load_style { deferred_load, nondeferred_load, explicit_load  };
    
protected:
    bool loaded, visible;
    load_style deferred;
    const boost::weak_ptr<StimulusDisplay> weakDisplay;
    
public:
    static const std::string DEFERRED;
    static const std::string DISPLAY;
    
    static void describeComponent(ComponentInfo &info);
    
    /**
     * Default Constructor.  Sets all members to false.
     */
    explicit Stimulus(const std::string &_tag);
    
    /**
     * StandardComponentFactory-compatible Constructor.
     */
    explicit Stimulus(const Map<ParameterValue> &parameters);
    
    /**
     * Overload this function.  This is a shell that does nothing.  Used
     * to load a stimulus into memory.
     */
    virtual void load(shared_ptr<StimulusDisplay> display);
    
    /**
     * Overload this function.  This is a shell that does nothing.  Used
     * to unload a stimulus from memory.
     */
    virtual void unload(shared_ptr<StimulusDisplay> display);
    
    /**
     * Sets whether this stimulus is visible.
     */
    virtual void setVisible(bool newvis);
    
    /**
     * Does the stimulus need to be redrawn?  (This is probably useful
     * only for dynamic stimuli.)
     */
    virtual bool needDraw(shared_ptr<StimulusDisplay> display) { return false; }
    
    /**
     * Draws the stimulus on display 'display'
     */
    virtual void draw(shared_ptr<StimulusDisplay> display);
    
    /**
     * Inspection methods.
     */
    bool isLoaded();
    bool isVisible();
    int getDeferred(){ return deferred; }
    void setDeferred(load_style _deferred){ deferred = _deferred; }
    boost::shared_ptr<StimulusDisplay> getDisplay() const { return weakDisplay.lock(); }
    
    // these method should be overriden to provide more announcement detail       
    virtual Datum getCurrentAnnounceDrawData();  
    
};


typedef boost::shared_ptr<Stimulus> StimulusPtr;


/**
 * StimulusGroup provides a simple mechanism for creating groups of stimuli
 * as an organizational convenience.  Its use is not necessary
 */
class StimulusGroup : public ExpandableList<Stimulus>, public mw::Component {
private:
	std::vector <shared_ptr<StimulusGroup> > sub_groups;
	
	boost::weak_ptr<StimulusGroup> parent;

	void addSubGroup(const shared_ptr <StimulusGroup> stim_group);
	void addStimToParents(const shared_ptr <Stimulus> stim);
	
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit StimulusGroup(const Map<ParameterValue> &parameters);
	virtual ~StimulusGroup() { }
	
	unsigned int dimensionSize(const unsigned int dimension);
	unsigned int getNDimensions() const;

	shared_ptr<StimulusGroup> getSubGroup(const unsigned int sub_group_index) const;
	shared_ptr<StimulusGroup> getParent() const;
	
	void setParent(boost::weak_ptr <StimulusGroup> _parent);
	
	virtual void addChild(std::map<std::string, std::string> parameters,
						  ComponentRegistry *reg,
						  shared_ptr<mw::Component> child);
	
};


typedef boost::shared_ptr<StimulusGroup> StimulusGroupPtr;


END_NAMESPACE_MW


#endif





















