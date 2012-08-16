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


inline ScarabDatum* getScarabEventCodeDatum(ScarabDatum *datum) {
    return scarab_list_get(datum, SCARAB_EVENT_CODEC_CODE_INDEX);
}


inline unsigned int getScarabEventCode(ScarabDatum *datum) {
    return getScarabEventCodeDatum(datum)->data.integer;
}


inline ScarabDatum* getScarabEventTimeDatum(ScarabDatum *datum) {
    return scarab_list_get(datum, SCARAB_EVENT_TIME_INDEX);
}


inline MWTime getScarabEventTime(ScarabDatum *datum) {
    return getScarabEventTimeDatum(datum)->data.integer;
}


inline ScarabDatum* getScarabEventPayload(ScarabDatum *datum) {
    if (datum->data.list->size < SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS) {
        return NULL;
    }
    return scarab_list_get(datum, SCARAB_EVENT_PAYLOAD_INDEX);
}


END_NAMESPACE(DataFileUtilities)


#endif /* !defined(__DataFileIndexer__DataFileUtilities__) */
