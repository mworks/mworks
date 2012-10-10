/*
 *  .cpp
 *  MWorksCore
 *
 *  Created by JJD on 8/25/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

// Note: from the XML spec point of view, we are managing things called "filters".
// However, under the hood, we are really dealing with VarTransformAdaptor objects
//  (objects that take vars as input and alter vars as output)

#include "FilterManager.h"


BEGIN_NAMESPACE_MW


FilterManager *GlobalFilterManager;

FilterManager::FilterManager(){}
	
FilterManager::~FilterManager(){}
	
void FilterManager::addFilter(shared_ptr<VarTransformAdaptor> _filter){
	filters.addReference(_filter);
}


END_NAMESPACE_MW
