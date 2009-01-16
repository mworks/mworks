/*
 *  DoneTableSelection.h
 *  MonkeyWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */


#ifndef DONE_TABLE_SELECTION_H
#define DONE_TABLE_SELECTION_H

#include "Selection.h"
namespace mw {
// Base Class for Selection with a "Done" table of some sort 
class DoneTableSelection : public Selection {
protected:
	ExpandableList<int> *done_table;
	int nelements;
	int nper;
	// how many samples are actually required to empty the done table
	int done_table_samples; 
	
public:
	DoneTableSelection(Selectable *_selectable);
	DoneTableSelection();
	DoneTableSelection(const DoneTableSelection& tocopy);
	
	virtual ~DoneTableSelection();
	
	// default increase in size routine. Just makes sure no memory problems
	virtual void incrementNItems();
	
	// probably needs overloading...
	virtual void resetDoneTable();
	virtual void reset();
	virtual bool advance();
	virtual void updateDoneTable();
	
	// TODO: funniness when reset the donetable?
	// Reject all of the tentative selection
	// (e.g. rewind those selections, they will be made
	// available again for selection)
	virtual void rejectSelections();
};

}


#endif
