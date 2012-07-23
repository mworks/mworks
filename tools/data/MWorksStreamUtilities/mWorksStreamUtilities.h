/*
 *  mWorksStreamUtilities.h
 *  MWorksMatlab
 *
 *  Created by David Cox on 12/20/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */
 
#include <string>
#include <mex.h>
#include "Scarab/scarab.h"
#include "Scarab/scarab_utilities.h"
#include <MWorksCore/Utilities.h>
using namespace mw;


int insertDatumIntoEventList(mxArray *eventlist, const int index, ScarabDatum *datum);
int insertDatumIntoCodecList(mxArray *codeclist, const int index, ScarabDatum *datum);
MWorksTime getMWorksTime(const mxArray *time);
std::string getString(const mxArray *string_array_ptr);
int getScarabEventCode(ScarabDatum *datum);
long long getScarabEventTime(ScarabDatum *datum);
ScarabDatum *getScarabEventPayload(ScarabDatum *datum);
mxArray *getScarabEventData(ScarabDatum *datum);
mxArray *getCodec(ScarabDatum *system_payload);
mxArray *recursiveGetScarabList(ScarabDatum *datum);
mxArray *recursiveGetScarabDict(ScarabDatum *datum);
mxArray *createTopLevelCodecStruct(long ncodecs);
mxArray *createTopLevelEventStruct(long nevents);
