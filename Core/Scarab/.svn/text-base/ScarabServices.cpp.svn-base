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
using namespace mw;

namespace mw {
	void printDatum(ScarabDatum * datum) {
		switch(datum->type) {
			case 0://SCARAB_NULL
				mdebug("datum->type => SCARAB_NULL");
				break;
			case 1://SCARAB_INTEGER
				mdebug("datum->type => SCARAB_INTEGER");
				mdebug("datum->data.integer => %d", datum->data.integer);
				break;
			case 2://SCARAB_FLOAT
				mdebug("datum->type => SCARAB_FLOAT");
				mdebug("datum->data.floatp => %d", datum->data.floatp);
				break;
			case 3://SCARAB_FLOAT_INF
				mdebug("datum->type => SCARAB_FLOAT_INF");
				mdebug("datum->data.floatp => %d", datum->data.floatp);
				break;
			case 4://SCARAB_FLOAT+NAN
				mdebug("datum->type => SCARAB_FLOAT_NAN");
				mdebug("datum->data.floatp => %d", datum->data.floatp);
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
		mnetwork("ERROR: SCR-%5.5i: %s: %s", scCode, scarab_moderror(scCode),
				 scarab_strerror(scCode));
		if(oserr) {
			mnetwork("OSERR: %i: %s", oserr, scarab_os_strerror(oserr));
		} 
	}
	
}
