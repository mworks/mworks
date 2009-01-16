/*
 *  DataFileUtilities.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "Scarab/scarab.h"
#include "MonkeyWorksCore.h"

class DataFileUtilities {
public:  
	static unsigned int getScarabEventCode(ScarabDatum *datum);
	static MonkeyWorksTime getScarabEventTime(ScarabDatum *datum);
};
