/*
 *  monkeyWorksStreamUtilities.h
 *  MonkeyWorksMatlab
 *
 *  Created by David Cox on 12/20/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */
 
#include <string>
#include <mex.h>
#include "Scarab/scarab.h"
#include "Scarab/scarab_utilities.h"
#include "dfindex/MonkeyWorksCore.h"


int insertDatumIntoEventList(mxArray *eventlist, const int index, ScarabDatum *datum);
int insertDatumIntoCodecList(mxArray *codeclist, const int index, ScarabDatum *datum);
MonkeyWorksTime getMonkeyWorksTime(const mxArray *time);
std::string getString(const mxArray *string_array_ptr);
int getScarabEventCode(ScarabDatum *datum);
long long getScarabEventTime(ScarabDatum *datum);
ScarabDatum *getScarabEventPayload(ScarabDatum *datum);
mxArray *getScarabEventData(ScarabDatum *datum);
mxArray *getCodec(ScarabDatum *system_payload);
double scarab_extract_float(ScarabDatum *datum);
mxArray *recursiveGetScarabList(ScarabDatum *datum);
mxArray *recursiveGetScarabDict(ScarabDatum *datum);

