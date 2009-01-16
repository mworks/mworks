#ifndef _SCARAB_SERVICES_H
#define _SCARAB_SERVICES_H

/**
 * ScarabServices.h
 *
 * Description: Common scarab functions.
 *
 * History:
 * David Cox on 10/21/04 - Created.
 * Paul Jankunas on 01/27/05 - Added datum creation function.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#include "Scarab/scarab.h"
#include "Scarab/scarab_list.h"
#include "Scarab/scarab_dict.h"
#include "Scarab/scarab_datatypes.h"
#include "Scarab/scarab_utilities.h"

namespace mw {

/**
 * Prints out a scarab datum.
 */
void printDatum(ScarabDatum * datum);

/**
 * Returns the error code of the scarab session or zero if no error happened.
 */
int getScarabError(ScarabSession *);

/**
 * Returns the OS error code from the scarab stream or zero if no error
 * occurred
 */
int getScarabOSError(ScarabSession *);

/**
 * Returns a short name of the error for number.  Returns NULL if invalid
 * error code
 */
const char * getScarabErrorName(int error);

/**
 * Returns a descriptive error string for the error sent.  Retruns NULL if
 * invalid error code
 */
const char * getScarabErrorDescription(int error);

/**
 * Returns the descriptive OS error string for the error number.  If 
 * error number is invalid then it will return the string Unknown Error:
 */
const char * getOSErrorDescription(int oserror);

/**
 * Combines the scarab error string messages into one.
 */
void logDescriptiveScarabMessage(ScarabSession *);
}
#endif
