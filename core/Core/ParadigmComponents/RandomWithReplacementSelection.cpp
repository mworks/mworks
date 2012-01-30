/*
 *  RandomWithReplacementSelection.cpp
 *  MWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "RandomWithReplacementSelection.h"
#include "Clock.h"
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
using namespace mw;


RandomWithReplacementSelection::RandomWithReplacementSelection(int _n_draws, bool _autoreset) 
: Selection(_n_draws, _autoreset) {
	shared_ptr <Clock> clock = Clock::instance();
	rng.seed((int)clock->getSystemTimeNS());
}

RandomWithReplacementSelection::~RandomWithReplacementSelection() { }

shared_ptr <Selection> RandomWithReplacementSelection::clone(){
	shared_ptr <RandomWithReplacementSelection> newsel = shared_ptr<RandomWithReplacementSelection>(new RandomWithReplacementSelection(*this));
	return dynamic_pointer_cast<Selection, RandomWithReplacementSelection>(newsel);
}

int RandomWithReplacementSelection::draw() {

	if(done_so_far >= n_draws){
        if(autoreset){
            mwarning(M_PARADIGM_MESSAGE_DOMAIN, "Autoreseting selection object");
            reset();
        } else {
            SelectionOffEdgeException e;
            throw e;
        }
	}
	
	// Make a random selection
	boost::uniform_int<> our_range(0, getNItems()-1);
	boost::variate_generator<random_generator&, boost::uniform_int<> > dice(rng, our_range);
	
	int returnval = dice();
	//int returnval = (int)(getNItems() * (float)rand()/(float)RAND_MAX);
	
	// Save the selection on the tentative selections list
	tentative_selections.push_back(returnval);

	done_so_far++;
	
	return returnval;
}



void RandomWithReplacementSelection::rejectSelections() {
	
	done_so_far -= tentative_selections.size();
	tentative_selections = std::vector<int>();
}




