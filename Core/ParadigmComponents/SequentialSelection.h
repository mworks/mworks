/*
 *  SequentialSelection.h
 *  MonkeyWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef _SEQENTIAL_SELECTION_H_
#define _SEQENTIAL_SELECTION_H_

#include "Selection.h"
namespace mw {
class SequentialSelection : public Selection {
protected:
	int current;
	bool ascending;
	
public:
	SequentialSelection(int _n_draws, bool ascendingp = 1);
	SequentialSelection(SequentialSelection& topcopy);	
	virtual ~SequentialSelection() { }

	virtual int draw();
	virtual void reset();
	
	virtual shared_ptr<Selection> clone(){
		shared_ptr<SequentialSelection> newsel = shared_ptr<SequentialSelection>(new SequentialSelection(*this));
		return dynamic_pointer_cast<Selection, SequentialSelection>(newsel);
	}

	
//	virtual Selection *clone(){
//		SequentialSelection *newsel = new SequentialSelection(*this);
//		return (Selection *)newsel;
//	}
	
	// Reject all of the tentative selection
	// (e.g. rewind those selections, they will be made
	// available again for selection)
	virtual void rejectSelections();
};
}
#endif
