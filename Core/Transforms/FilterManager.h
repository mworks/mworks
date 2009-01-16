#ifndef FILTER_MANAGER_H_
#define FILTER_MANAGER_H_
/*
 *  FilterManager.h
 *  MonkeyWorksCore
 *
 *  Created by JJD on 8/25/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

// Note: from the XML spec point of view, we are managing things called "filters".
// However, under the hood, we are really dealing with VarTransformAdaptor objects
//  (objects that take vars as input and alter vars as output)

#include "ExpandableList.h"
#include "VariableTransformAdaptors.h"

namespace mw {
class FilterManager {

protected:

	ExpandableList<VarTransformAdaptor> filters;
	
public:

	FilterManager();
	~FilterManager();
	void addFilter(shared_ptr<VarTransformAdaptor> _filter);

};


extern FilterManager *GlobalFilterManager;
}
#endif

