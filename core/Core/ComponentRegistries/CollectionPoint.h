#ifndef	COLLECTION_POINT_H_
#define COLLECTION_POINT_H_

/*
 *  CollectionPoint.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/16/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExpandableList.h"


class Collectable {

public:
	Collectable();
};



class CollectionPoint {

protected:
	
	ExpandableList<Collectable> *collection_list;

public:
	CollectionPoint();
	
	virtual ~CollectionPoint();
	
	virtual void prepareForDeletion(){
		// This method provides a standard hook to do anything
		// necessary with the objects before they are freed
	}
	
	virtual void add(shared_ptr<Collectable> item){
		collection_list->addElement(item);
	}
		
};


extern CollectionPoint *GlobalCollectionPoint;

#endif
