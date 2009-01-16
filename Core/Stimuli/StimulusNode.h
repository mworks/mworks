/*
 *  StimulusNode.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 6/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef STIMULUS_NODE_H_
#define STIMULUS_NODE_H_
 
#include <boost/shared_ptr.hpp>
#include "StimulusDisplay.h"
#include "Stimulus.h"
#include "GenericVariable.h"
#include "Component.h"
namespace mw {
class StimulusGroup;

class StimulusNode : public Lockable, public LinkedListNode<StimulusNode>, public mw::Component {
    protected:
        /*StimulusNode *previous;
        StimulusNode *next;*/
        
		// A reference for the stimulus that this node refers to
	shared_ptr<Stimulus> stim;
		
		// A reference to a "frozen-in-time" (e.g. variables all evaluated and
		// not changing) version of the stimulus
		// The node "owns" the frozen copy and is responsible for freeing it
	shared_ptr<Stimulus> frozen_stim; 
		
		// The StimulusDisplay object that this node belongs to
        //mStimulusDisplay *stimulus_display;
		
		// Is the stimulus set to be shown on the next update?
        bool visible;
        bool visible_on_last_update;    // JJD added July 2006

		// Is the stimulus in a "frozen" state (e.g. do we want to the
		//  to use frozen_stim instead of stim -- frozen_stim is a variable
		//  locked version of stim, created using the frozenClone() method 
		//  on the stimulus
		bool frozen;

    public:
        
		StimulusNode();
        StimulusNode(shared_ptr<Stimulus> _stim);
		
		virtual ~StimulusNode(){}
        
		// DDC: I'd rather not have these if we don't use them
	    //virtual void setStimulus(Stimulus *_stim);
        virtual shared_ptr<Stimulus> getStimulus();
        
		virtual void addToDisplay(shared_ptr<StimulusNode> stimnode, shared_ptr<StimulusDisplay> display);
		
		// Set a flag to determine whether to draw this stimulus on an update
		virtual void setVisible(bool _vis);
        virtual bool isVisible();
        
		// Freeze the stimulus as needed, and set the frozen flag
		virtual void setFrozen(bool _frozen);
		virtual bool isFrozen();
		
		virtual void draw(StimulusDisplay* display);
                
		virtual void announceStimulusDraw(MonkeyWorksTime time);
		virtual Data getCurrentAnnounceDrawData();
    private:
        // this disables copying the object.
        StimulusNode(const StimulusNode&) { }
        void operator=(const StimulusNode& lval) { }
 };


// A group of stimulus nodes; WATCH the names of these things, they are
// admittedly tricky.
// This class (and it's word soup brethren) aren't gratuitous, they're needed
// to be able to parse and handle stimulus group references properly.
// A better, longer discussion how it all works will find its way into the 
// comments soon 
class StimulusNodeGroup : public ExpandableList<StimulusNode>, public mw::Component {

	public:

	StimulusNodeGroup(shared_ptr<StimulusGroup> _group);
};

class StimulusGroupReferenceNode : public StimulusNode {
    protected:
	shared_ptr<Variable> index;
	shared_ptr<StimulusNodeGroup> stimulus_nodes;
	
		int getIndexValue();

    public:

		StimulusGroupReferenceNode(shared_ptr<StimulusNodeGroup> _group,
										shared_ptr<Variable> _index);
		virtual ~StimulusGroupReferenceNode(){}							

		virtual void addToDisplay(shared_ptr<StimulusNode> stimnode, shared_ptr<StimulusDisplay> display);
	
		// set the "visible" state of the node
		virtual void setVisible(bool _vis);
        virtual bool isVisible();
		
		// set the "frozen" state of the node
		virtual void setFrozen(bool _frozen);
		virtual bool isFrozen();
		
        virtual void draw(StimulusDisplay * display);
        virtual void announceStimulusDraw(MonkeyWorksTime time);  // JJD add
		virtual Data getCurrentAnnounceDrawData();
		
		
		// LinkedListNode methods
		virtual void remove();
		virtual void bringToFront();
		virtual void sendToBack();
		virtual void bringForward();
		virtual void sendBackward();

};
 }
#endif
