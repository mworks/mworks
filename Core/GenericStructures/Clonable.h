/*
 *  Clonable.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 1/17/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
 
 
#ifndef		M_CLONABLE_H_
#define		M_CLONABLE_H_
namespace mw {
using namespace std;


class Clonable {

	protected:
	
		bool	is_a_clone;

	public:
		virtual ~Clonable() { }
		// This returns a void pointer to a fesh copy of this
		// object
		virtual void *clone();
		
		virtual bool isAClone(){ return is_a_clone; }
		virtual void setIsAClone(bool isit){  is_a_clone = isit; }
};
}

#endif