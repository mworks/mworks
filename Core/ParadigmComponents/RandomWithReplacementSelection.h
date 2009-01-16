/*
 *  RandomWithReplacementSelection.h
 *  MonkeyWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */


#ifndef RANDOM_WITH_REPLACEMENT_SELECTION_H
#define RANDOM_WITH_REPLACEMENT_SELECTION_H

#include "Selection.h"

#ifdef seed
#undef seed
#endif

#include <boost/random.hpp>
namespace mw {

typedef	boost::mt19937	random_generator;

class RandomWithReplacementSelection : public Selection {

protected:

	random_generator rng;

public:
	RandomWithReplacementSelection(int _n_draws);
	
	virtual ~RandomWithReplacementSelection();
	virtual shared_ptr<Selection> clone();
	
	virtual int draw();
	virtual void reset(){  Selection::reset(); }
	
	virtual void rejectSelections();
};
}
#endif


