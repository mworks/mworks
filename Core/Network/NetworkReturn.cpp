/**
 * NetworkReturn.cpp
 *
 * History: 
 * Paul Jankunas on 8/19/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "NetworkReturn.h"
#include "Utilities.h"
#include <stdarg.h>
#include <string>
using namespace mw;

// buffer padding for variable length arguments.
const int BUFFER_PADDING = 256;

NetworkReturn::NetworkReturn() {
    rvCode = NR_SUCCESS;
    osCode = NR_INVALID_OS_CODE;
    packageCode = NR_INVALID_PACKAGE_CODE;
}

NetworkReturn::NetworkReturn(int rc, int oscode, int pkgcode, 
                                                        const std::string &msg) {
    rvCode = rc;
    osCode = oscode;
    packageCode = pkgcode;
    information = msg;
}

NetworkReturn::~NetworkReturn() {
}

void NetworkReturn::setInformation(const std::string &msg) { 
    information = msg;
}

void NetworkReturn::appendInformation(const std::string &msg) {
	information.append(msg);
}

void NetworkReturn::appendInformationWithFormat(const std::string MSG, ...) {
	const char *msg = MSG.c_str();
    va_list ap;
	va_start(ap, MSG);
    int len = strlen(msg) + BUFFER_PADDING;
    char * tmp = new char[len+1];
    int numWritten = vsnprintf(tmp, len+1, msg, ap);
    if(numWritten > (len+1)) {
        // try again
        delete [] tmp;
        tmp = new char[numWritten+1];
        vsnprintf(tmp, numWritten+1, msg, ap);
    }
    va_end(ap);
    appendInformation(tmp);
    delete [] tmp;
}