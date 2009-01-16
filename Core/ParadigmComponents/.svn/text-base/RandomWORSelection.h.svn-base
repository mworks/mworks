/*
 *  RandomWORSelection.h
 *  MonkeyWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef RANDOM_WOR_SELECTION_H
#define RANDOM_WOR_SELECTION_H

#include "Selection.h"
#include <list>

#ifdef seed
#undef seed
#endif

#include <boost/random.hpp>
namespace mw {
typedef	boost::mt19937	random_generator;


class RandomWORSelection : public Selection {
protected:

	std::list<int> draw_list;
	random_generator rng;
	
public:
	RandomWORSelection(int _n_draws);
	virtual ~RandomWORSelection(){}
	
	virtual shared_ptr<Selection> clone();
	
	virtual int draw();
	virtual void reset();
	
	virtual void rejectSelections();
};
}
#endif
