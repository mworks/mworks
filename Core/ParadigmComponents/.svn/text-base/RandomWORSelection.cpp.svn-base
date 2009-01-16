/*
 *  RandomWORSelection.cpp
 *  MonkeyWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "RandomWORSelection.h"


#include <boost/random.hpp>
#include "Clock.h"
using namespace mw;


RandomWORSelection::RandomWORSelection(int _n_draws) : Selection(_n_draws){
	shared_ptr <Clock> clock = Clock::instance();
	rng.seed((int)clock->getSystemTimeNS());
	reset();
}

void RandomWORSelection::reset() {	
	
	draw_list = std::list<int>();
	
	if(getNItems() != 0){
	
		int nper = ceil((float)n_draws / (float)getNItems());

		for(int i = 0; i < getNItems(); i++) {
			for(int j = 0; j < nper; j++){
				draw_list.push_back(i);
			}
		}
	}
	
	Selection::reset();
}

shared_ptr <Selection> RandomWORSelection::clone(){
	shared_ptr <RandomWORSelection> newsel = shared_ptr<RandomWORSelection>(new RandomWORSelection(*this));
	return dynamic_pointer_cast<Selection, RandomWORSelection>(newsel);
}


int RandomWORSelection::draw() {
	
	if(done_so_far >= n_draws){
		SelectionOffEdgeException e;
		throw e;
	}

	if(draw_list.size() <= 0){
		InvalidSelectionException e;
		throw e;
	}
	
	
	int returnval;
	
	
	int the_size = draw_list.size();
	
	// Make a random selection
	boost::uniform_int<> our_range(0, draw_list.size()-1);
	boost::variate_generator<random_generator&, boost::uniform_int<> > dice(rng, our_range);
	
	int random = dice();
		
	// has to be a better way
	std::list<int>::iterator i = draw_list.begin();
	for(int k=0; k < random; k++){
		i++;
	}
	
	returnval = *i;
//	cerr << "Selected: " << returnval << endl;
				
	// Remove it from further use
	draw_list.erase(i);
	
	the_size = draw_list.size();
	// Save it on the tentative list
	tentative_selections.push_back(returnval);
	
	done_so_far++;
	return returnval;
}


void RandomWORSelection::rejectSelections(){

	std::vector<int>::iterator i;
	for(i = tentative_selections.begin(); i != tentative_selections.end(); i++){
		draw_list.push_back(*i); // put it back
		done_so_far--;
	}
	
	tentative_selections = std::vector<int>();
}

