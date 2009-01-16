/**
 * Selection.h
 *
 * History:
 * David Cox on 12/28/05 - Created.
 * Paul Jankunas on 05/13/04 - added getSelectionType to Selection type to  
 *      give information about what kind of selection an object is.
 * Paul Jankunas on 06/01/05 - Removed getSelectionType() because it is unused.
 *      fixed spacing.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef SELECTION_H
#define SELECTION_H

#include <math.h>
#include <stdlib.h>
#include <vector>

#include "Utilities.h"
#include "GenericVariable.h"
#include "ExpandableList.h"
#include "Clonable.h"
#include "Selectable.h"
namespace mw {

enum SelectionType { M_SEQUENTIAL, M_SEQUENTIAL_ASCENDING, 
                     M_SEQUENTIAL_DESCENDING, M_RANDOM_WITH_REPLACEMENT, 
                     M_RANDOM_WOR };

enum SampleType { M_SAMPLES, M_CYCLES };

class SelectionOffEdgeException : public std::exception{
virtual const char* what() const throw()
  {
    return "There are no more items left to draw";
  }
};

class InvalidSelectionException : public std::exception{
virtual const char* what() const throw()
  {
    return "Attempt to draw an invalid selection index";
  }
};

class Selection {
//	class Selection : public mw::Component{
	protected:
		Selectable *selectable; // the object from whom selection will be made
								 // weak reference
		
		int n_draws;
		int done_so_far;

		std::vector<int> tentative_selections; // to allow "undo" feature
		        
	public:
	
        Selection(int _n_draws);
        virtual ~Selection() { }
        
		// Delegation of things-to-be-selected
		virtual void setSelectable(Selectable *_selectable){
			selectable = _selectable;
			reset();
		}
		virtual int getNItems();
			
		// Draw the next selection - mandatory override
		virtual int draw() = 0;
		
		// Reset the object to start over
		virtual void reset();
		
		// Accept all of the tentative selections
		virtual void acceptSelections();
		
		// Reject all of the tentative selection (e.g. rewind those selections, 
		// they will be made available again for selection). Mandatory override.
		virtual void rejectSelections() = 0;
		
		// Interogation functions
		virtual bool isFinished(){  return (done_so_far >= n_draws); }
		virtual int  getNDone(){  return done_so_far; }
		virtual int  getNLeft(){ return n_draws - done_so_far; }
		
		virtual shared_ptr<Selection> clone() = 0;
		
		
		
};

}
#endif

