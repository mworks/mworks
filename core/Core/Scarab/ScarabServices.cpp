/**
 * ScarabServices.cpp.
 *
 * History:
 * David Cox on 10/21/04 - Created.
 * Paul Jankunas on 01/27/05 - Added datum creation function.
 * Paul Jankunas on 04/29/05 - Changed out of memory strings from mprintf to
 *                              merror.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#include "ScarabServices.h"
#include "Utilities.h"

#include <boost/scope_exit.hpp>


BEGIN_NAMESPACE_MW


	void printDatum(ScarabDatum * datum) {
		switch(datum->type) {
			case 0://SCARAB_NULL
				mdebug("datum->type => SCARAB_NULL");
				break;
			case 1://SCARAB_INTEGER
				mdebug("datum->type => SCARAB_INTEGER");
				mdebug("datum->data.integer => %lld", datum->data.integer);
				break;
			case 2://SCARAB_FLOAT
				mdebug("datum->type => SCARAB_FLOAT");
				mdebug("datum->data.floatp => %g", datum->data.floatp);
				break;
			case 6://SCARAB_LIST
				mdebug("datum->type => SCARAB_LIST");
				break;
			case 7://SCARAB_OPAQUE
				mdebug("datum->type => SCARAB_OPAQUE");
				mdebug("datum->data.opaque.size => %d", datum->data.opaque.size);
				mdebug("datum->data.opaque.data => %s", datum->data.opaque.data);
				break;
			case 5://SCARAB_DICT
				mdebug("datum->type => SCARAB_DICT");
				break;
			default:
				mdebug("datum->type => UNDEFINED");
				break;
		}
	}
	
	int getScarabError(ScarabSession * session) {
		return scarab_session_geterr(session);
	}
	
	int getScarabOSError(ScarabSession * session) {
		if(getScarabError(session)) {
			return scarab_session_getoserr(session);
		} else {
			return 0;
		}
	}
	
	const char * getScarabErrorName(int error) {
		return scarab_moderror(error);
	}
	
	const char * getScarabErrorDescription(int error) {
		const char *error_description = scarab_strerror(error);
		if(error_description == NULL){
			return "Unknown error type";
		}
		
		return error_description;
	}
	
	const char * getOSErrorDescription(int oserror) {
		return scarab_os_strerror(oserror);
	}
	
	void logDescriptiveScarabMessage(ScarabSession * s) {
		int scCode = getScarabError(s); // scarab error code
		int oserr = getScarabOSError(s);
    const char *scarab_error_string = scarab_strerror(scCode);
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "ERROR: SCR-%5.5i: %s: %s", scCode, scarab_moderror(scCode),
				 scarab_error_string);
		if(oserr) {
      const char *scarab_os_error_string = scarab_os_strerror(oserr);
			mwarning(M_NETWORK_MESSAGE_DOMAIN, "OSERR: %i: %s", oserr, scarab_os_error_string);
		} 
	}


scarab_datum_ptr datumToScarabDatum(const Datum &d) {
    switch (d.getDataType()) {
        case M_INTEGER:
            return scarab_datum_ptr(scarab_new_integer(d.getInteger()), false);
            
        case M_FLOAT:
            return scarab_datum_ptr(scarab_new_float(d.getFloat()), false);
            
        case M_BOOLEAN:
            return scarab_datum_ptr(scarab_new_integer(d.getBool()), false);
            
        case M_STRING: {
            auto &sval = d.getString();
            const char *data = sval.c_str();
            std::size_t size = sval.size();
            if (d.stringIsCString()) {
                // Include the terminal NUL to indicate that the opaque is a C string
                size += 1;
            }
            return scarab_datum_ptr(scarab_new_opaque(data, size), false);
        }
            
        case M_LIST: {
            auto &lval = d.getList();
            scarab_datum_ptr list(scarab_list_new(lval.size()), false);
            
            for (std::size_t i = 0; i < lval.size(); i++) {
                scarab_list_put(list.get(), i, datumToScarabDatum(lval.at(i)).get());
            }
            
            return std::move(list);
        }
            
        case M_DICTIONARY: {
            auto &dval = d.getDict();
            scarab_datum_ptr dict(scarab_dict_new(dval.size(), &scarab_dict_times2), false);
            
            for (auto &item : dval) {
                scarab_dict_put(dict.get(), datumToScarabDatum(item.first).get(), datumToScarabDatum(item.second).get());
            }
            
            return std::move(dict);
        }
            
        default:
            return scarab_datum_ptr(scarab_new_atomic(), false);
    }
}


Datum scarabDatumToDatum(ScarabDatum *datum) {
    if (!datum) {
        return Datum();
    }
    
    switch (datum->type) {
        case SCARAB_INTEGER:
            return Datum(datum->data.integer);
            
        case SCARAB_FLOAT:
            return Datum(datum->data.floatp);
            
        case SCARAB_DICT: {
            Datum::dict_value_type d;
            
            scarab_lock_datum(datum);
            BOOST_SCOPE_EXIT(datum) {
                scarab_unlock_datum(datum);
            } BOOST_SCOPE_EXIT_END
            
            ScarabDict *dict = datum->data.dict;
            for (int i = 0; i < dict->tablesize; i++) {
                if (dict->keys[i]) {
                    d.emplace(scarabDatumToDatum(dict->keys[i]), scarabDatumToDatum(dict->values[i]));
                }
            }
            
            return Datum(std::move(d));
        }
            
        case SCARAB_LIST: {
            Datum::list_value_type l;
            
            scarab_lock_datum(datum);
            BOOST_SCOPE_EXIT(datum) {
                scarab_unlock_datum(datum);
            } BOOST_SCOPE_EXIT_END
            
            ScarabList *list = datum->data.list;
            for (int i = 0; i < list->size; i++) {
                l.emplace_back(scarabDatumToDatum(list->values[i]));
            }
            
            return Datum(std::move(l));
        }
            
        case SCARAB_OPAQUE: {
            const char *data = reinterpret_cast<char *>(datum->data.opaque.data);
            int size = datum->data.opaque.size;
            if (scarab_opaque_is_string(datum)) {
                // Don't include the terminal NUL
                size -= 1;
            }
            return Datum(data, size);
        }
            
        default:
            return Datum();
    }
}


scarab_datum_ptr eventToScarabEventDatum(const Event &e) {
    auto event_datum = scarab_datum_ptr(scarab_list_new(MWORKS_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS), false);
    
    scarab_list_put(event_datum.get(),
                    MWORKS_EVENT_CODEC_CODE_INDEX,
                    scarab_datum_ptr(scarab_new_integer(e.getEventCode()), false).get());
    scarab_list_put(event_datum.get(),
                    MWORKS_EVENT_TIME_INDEX,
                    scarab_datum_ptr(scarab_new_integer(e.getTime()), false).get());
    scarab_list_put(event_datum.get(),
                    MWORKS_EVENT_PAYLOAD_INDEX,
                    datumToScarabDatum(e.getData()).get());
    
    return std::move(event_datum);
}


Event scarabEventDatumToEvent(ScarabDatum *event_datum) {
    if (!event_datum || event_datum->type != SCARAB_LIST) {
        mwarning(M_NETWORK_MESSAGE_DOMAIN,
                 "Attempt to create an event object from an invalid ScarabDatum");
        return Event();
    }
    
    ScarabDatum *code_datum = scarab_list_get(event_datum, MWORKS_EVENT_CODEC_CODE_INDEX);
    if (code_datum->type != SCARAB_INTEGER) {
        mwarning(M_NETWORK_MESSAGE_DOMAIN,
                 "Attempt to create an event object with an invalid code datum (%d)",
                 code_datum->type);
        return Event();
    }
    
    int code = code_datum->data.integer;
    MWTime time = 0;
    
    ScarabDatum *time_datum = scarab_list_get(event_datum, MWORKS_EVENT_TIME_INDEX);
    if (!time_datum) {
        mwarning(M_NETWORK_MESSAGE_DOMAIN,
                 "Bad time datum received over the network (NULL)");
    } else if (time_datum->type != SCARAB_INTEGER) {
        mwarning(M_NETWORK_MESSAGE_DOMAIN,
                 "Bad time datum received over the network (invalid type: %d)",
                 time_datum->type);
    } else {
        time = time_datum->data.integer;
    }
    
    ScarabDatum *payload = scarab_list_get(event_datum, MWORKS_EVENT_PAYLOAD_INDEX);
    
    return Event(code, time, scarabDatumToDatum(payload));
}


END_NAMESPACE_MW



























