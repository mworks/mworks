/*
 *  DataFileUtilities.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DataFileUtilities.h"
#include "MonkeyWorksCore.h"

unsigned int DataFileUtilities::getScarabEventCode(ScarabDatum *datum){	
	ScarabDatum *code_datum = scarab_list_get(datum, SCARAB_EVENT_CODEC_CODE_INDEX);
	return code_datum->data.integer;
}

MonkeyWorksTime DataFileUtilities::getScarabEventTime(ScarabDatum *datum) {
	ScarabDatum *time_datum = scarab_list_get(datum, SCARAB_EVENT_TIME_INDEX);
	return time_datum->data.integer;
}
