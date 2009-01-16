/*
 *  CollectionPoint.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/16/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "CollectionPoint.h"


Collectable::Collectable(){
	/* do nothing */
}


CollectionPoint::CollectionPoint(){
	collection_list = new ExpandableList<Collectable>();
}

CollectionPoint::~CollectionPoint(){
	prepareForDeletion();

	delete collection_list;
}

CollectionPoint *GlobalCollectionPoint;