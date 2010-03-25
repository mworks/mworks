/*
 *  StimulusNode.h
 *  MWorksCore
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
        
		// A reference for the stimulus that this node refers to
        shared_ptr<Stimulus> stim;

        shared_ptr<Stimulus> live_stim; // the (potentially frozen) stimulus
                                           // to show the next time an explicit
                                           // update display is called (not an
                                           // internally-driven update call, as 
                                           // in dynamic stimulus
    
		
		// Is the stimulus set to be shown on the next update?
        
        bool visible;         // show whenever an update display (or any kind) is issued
    
        bool visible_on_last_update;    // JJD added July 2006
                                        // internal knowledge of whether this stimulus
                                        // was shown previously (to enable "stim off"
                                        // events

        
        bool pending;       // a flag indicating that some element of this 
                            // is pending a change to occur on the next
                            // "explicit" display update
    
        bool pending_visible; // queued, but not yet visible; set to visible
                                // when performing an explicit updateDisplay
                                // do not change state if display is updated 
                                // by a scheduled update (e.g. from a dynamic stimulus)
    
        bool pending_removal;   // a flag indicating that the node should be 
                                // removed from the chain on the next "explicit"
                                // display update
    
        // Is the stimulus in a "frozen" state (e.g. do we want to the
		//  to use frozen_stim instead of stim -- frozen_stim is a variable
		//  locked version of stim, created using the frozenClone() method 
		//  on the stimulus
		bool frozen;

    public:
        
		StimulusNode();
		StimulusNode(shared_ptr<Stimulus> _stim);
		
		virtual ~StimulusNode(){}
        
		virtual shared_ptr<Stimulus> getStimulus();
        
		virtual void addToDisplay(shared_ptr<StimulusNode> stimnode, shared_ptr<StimulusDisplay> display);
		
		
        virtual void setVisible(bool _vis);
        virtual bool isVisible();
        
        
        // Set a flag to determine whether to draw this stimulus on an update
        virtual void clearPending();
        virtual bool isPending();
    
        virtual void setPendingVisible(bool _vis);
        virtual bool isPendingVisible();
    
        virtual void setPendingRemoval();
        virtual void clearPendingRemoval();
        virtual bool isPendingRemoval();
    
		// "Freeze" the stimulus (copy with fixed variable values), and query the frozen flag
		virtual void freeze();
        virtual void thaw();
		virtual bool isFrozen();
		
		virtual void draw(StimulusDisplay* display);
                
		virtual void announceStimulusDraw(MWTime time);
		virtual Datum getCurrentAnnounceDrawData();
	
        virtual void load(shared_ptr<StimulusDisplay> display);
        
        virtual bool isLoaded();
        virtual int getDeferred();
	
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
        virtual void setFrozen(bool _frozen);


    public:

		StimulusGroupReferenceNode(shared_ptr<StimulusNodeGroup> _group,
										shared_ptr<Variable> _index);
		virtual ~StimulusGroupReferenceNode(){}							

		virtual void addToDisplay(shared_ptr<StimulusNode> stimnode, shared_ptr<StimulusDisplay> display);
	
		// set the "visible" state of the node
		virtual void setVisible(bool _vis);
        virtual bool isVisible();

        // Set a flag to determine whether to draw this stimulus on an update
        virtual void clearPending();
        virtual bool isPending();
        
        virtual void setPendingVisible(bool _vis);
        virtual bool isPendingVisible();
        
        virtual void setPendingRemoval();
        virtual void clearPendingRemoval();
        virtual bool isPendingRemoval();
        
    
		// set the "frozen" state of the node
        virtual void freeze();
        virtual void thaw();
		virtual bool isFrozen();
		
        virtual void draw(StimulusDisplay * display);
        virtual void announceStimulusDraw(MWTime time);  // JJD add
		virtual Datum getCurrentAnnounceDrawData();
		
  
        virtual void load(shared_ptr<StimulusDisplay> display);
        
        virtual bool isLoaded();
            
        virtual int getDeferred();
  
		// LinkedListNode methods
		virtual void remove();
		virtual void bringToFront();
		virtual void sendToBack();
		virtual void bringForward();
		virtual void sendBackward();

};
 }
#endif
