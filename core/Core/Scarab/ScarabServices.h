#ifndef _SCARAB_SERVICES_H
#define _SCARAB_SERVICES_H

/**
 * ScarabServices.h
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#include <boost/intrusive_ptr.hpp>
#include <Scarab/scarab.h>

#include "Event.h"


inline void intrusive_ptr_add_ref(ScarabDatum *d) { scarab_copy_datum(d); }
inline void intrusive_ptr_release(ScarabDatum *d) { scarab_free_datum(d); }


BEGIN_NAMESPACE_MW


using scarab_datum_ptr = boost::intrusive_ptr<ScarabDatum>;
scarab_datum_ptr datumToScarabDatum(const Datum &d);
Datum scarabDatumToDatum(ScarabDatum *datum);
scarab_datum_ptr eventToScarabEventDatum(const Event &e);
Event scarabEventDatumToEvent(ScarabDatum *event_datum);


END_NAMESPACE_MW


#endif
