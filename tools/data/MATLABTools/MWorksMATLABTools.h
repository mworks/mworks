/*
 *  MWorksMATLABTools.h
 *  MATLABTools
 *
 *  Created by David Cox on 12/20/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */
 
#ifndef __MWorksMATLABTools__
#define __MWorksMATLABTools__

#include <matrix.h>
#include <Scarab/scarab.h>
#include <dfindex/DataFileUtilities.h>

using namespace DataFileUtilities;


mxArray* getScarabDatum(ScarabDatum *datum);
mxArray *getCodec(ScarabDatum *system_payload);
mxArray *createTopLevelCodecStruct(long ncodecs);
mxArray *createTopLevelEventStruct(long nevents);
int insertDatumIntoCodecList(mxArray *codeclist, const int index, ScarabDatum *datum);


class MATLABEventInfo {
    
public:
    MATLABEventInfo(ScarabDatum *datum) :
        code(mxCreateDoubleScalar(double(getScarabEventCode(datum)))),
        time(mxCreateDoubleScalar(double(getScarabEventTime(datum)))),
        data(getScarabDatum(getScarabEventPayload(datum)))
    { }
    
    mxArray* getCode() const { return code; }
    mxArray* getTime() const { return time; }
    mxArray* getData() const { return data; }
    
private:
    mxArray *code;
    mxArray *time;
    mxArray *data;
    
};


void insertEventIntoEventList(mxArray *eventlist, const int index, const MATLABEventInfo &event);


inline void insertDatumIntoEventList(mxArray *eventlist, const int index, ScarabDatum *datum) {
    MATLABEventInfo event(datum);
    insertEventIntoEventList(eventlist, index, event);
}


#endif /* !defined(__MWorksMATLABTools__) */
























