/*
 *  monkeyWorksStreamUtilities.h
 *  MonkeyWorksMatlab
 *
 *  Created by David Cox on 12/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
 
#include <string>
#include <mex.h>
#include "Scarab/scarab.h"
#include "Scarab/scarab_utilities.h"
#include "MonkeyWorksCore/EventConstants.h"

#define VERBOSE 0


double scarab_extract_float(ScarabDatum *datum);

int getScarabEventCode(ScarabDatum *datum);

long long getScarabEventTime(ScarabDatum *datum);

ScarabDatum *getScarabEventPayload(ScarabDatum *datum);


mxArray *recursiveGetScarabList(ScarabDatum *datum);

mxArray *recursiveGetScarabDict(ScarabDatum *datum);

mxArray *getScarabEventData(ScarabDatum *datum);

mxArray *getScarabEventDataArray(ScarabDatum *datum);

int getScarabSystemEventType(ScarabDatum *payload);

int getScarabSystemEventControlType(ScarabDatum *payload);
ScarabDatum *getScarabSystemEventPayload(ScarabDatum *payload);

mxArray *extractCodec(ScarabDatum *system_payload);

int scarabEventToDataStruct(mxArray *data_struct, int index, ScarabDatum *datum);

int getScarabError(ScarabSession * session);
int getScarabOSError(ScarabSession * session);
const char * getScarabErrorName(int error);
const char * getScarabErrorDescription(int error);
const char * getOSErrorDescription(int oserror);
