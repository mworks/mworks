#ifndef	SELECTABLE_H_
#define SELECTABLE_H_

/*
 *  Selectable.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 5/1/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

//#include "Selection.h"
#include <boost/shared_ptr.hpp>
namespace mw {
	using namespace boost;


class Selection;

class Selectable {

	protected:
	
		shared_ptr<Selection> selection;

	public:

		Selectable();
		virtual ~Selectable();
		
		void attachSelection(shared_ptr<Selection> _selection);
        shared_ptr<Selection> getSelectionClone();
		
		// Method to be overriden by subclasses
		virtual int getNItems() = 0;
		
		virtual void acceptSelections();
		virtual void rejectSelections();
		virtual void resetSelections();

		virtual int getNDone();
		virtual int getNLeft();
		
};
}
#endif
