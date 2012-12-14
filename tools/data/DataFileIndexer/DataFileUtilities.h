/*
 *  DataFileUtilities.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef __DataFileIndexer__DataFileUtilities__
#define __DataFileIndexer__DataFileUtilities__

#include <Scarab/scarab.h>

#include <MWorksCore/MWorksMacros.h>
#include <MWorksCore/MWorksTypes.h>
#include <MWorksCore/EventConstants.h>

using mw::MWTime;


BEGIN_NAMESPACE(DataFileUtilities)


inline int getScarabEventSize(const ScarabDatum *datum) {
    return datum->data.list->size;
}


inline ScarabDatum** getScarabEventElements(const ScarabDatum *datum) {
    return datum->data.list->values;
}


inline ScarabDatum* getScarabEventCodeDatum(const ScarabDatum *datum) {
    return getScarabEventElements(datum)[SCARAB_EVENT_CODEC_CODE_INDEX];
}


inline unsigned int getScarabEventCode(const ScarabDatum *datum) {
    return (unsigned int)(getScarabEventCodeDatum(datum)->data.integer);
}


inline ScarabDatum* getScarabEventTimeDatum(const ScarabDatum *datum) {
    return getScarabEventElements(datum)[SCARAB_EVENT_TIME_INDEX];
}


inline MWTime getScarabEventTime(const ScarabDatum *datum) {
    return getScarabEventTimeDatum(datum)->data.integer;
}


inline ScarabDatum* getScarabEventPayload(const ScarabDatum *datum) {
    if (getScarabEventSize(datum) < SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS) {
        return NULL;
    }
    return getScarabEventElements(datum)[SCARAB_EVENT_PAYLOAD_INDEX];
}


inline bool isScarabEvent(const ScarabDatum *datum) {
    return ((datum->type == SCARAB_LIST) &&
            ((getScarabEventSize(datum) == SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS) ||
             (getScarabEventSize(datum) == SCARAB_EVENT_N_TOPLEVEL_ELEMENTS)) &&
            (getScarabEventCodeDatum(datum)->type == SCARAB_INTEGER) &&
            (getScarabEventTimeDatum(datum)->type == SCARAB_INTEGER));
}


END_NAMESPACE(DataFileUtilities)


#endif /* !defined(__DataFileIndexer__DataFileUtilities__) */



























