/**
 * NetworkReturn.h
 *
 * Description: A return value for network functions that can store extended
 * information.
 *
 * History: 
 * Paul Jankunas on 8/19/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef _NETWORK_RETURN_H__
#define _NETWORK_RETURN_H__

#include <stdio.h>
#include <string>
namespace mw {
const int NR_SUCCESS = 0; // success with no message
const int NR_SUCCESS_WITH_MESSAGE = 1; //success with a message
const int NR_SUCCESS_NETWORK_MESSAGE = 2;
const int NR_FAILED = -1; // failed but not a big deal
const int NR_ERROR = -2; // failed and generated an merror message
const int NR_FATAL_ERROR = -3; // failed with mfatal_error message
const int NR_INTERRUPTED = 1;

const int NR_INVALID_PACKAGE_CODE = -65535;
const int NR_INVALID_OS_CODE = -65535;

class NetworkReturn {
    protected:
        int rvCode; // monkey works code
        int osCode; // errno
        int packageCode; // scarab errno
		std::string information; // good or bad
    
    public:
        /**
         * Creates a new NetworkReturn and initializes rvCode to be 
         * a successful value.  All other information is invalid.
         */
        NetworkReturn();
        
        /**
         * Creates a NetworkReturn with the values specified.
         */
        NetworkReturn(int rc, int oscode, int pkgcode, const std::string &msg);
        
        /**
         * Default destructor.
         */
        ~NetworkReturn();
        
        /**
         * Returns true if a message is available false otherwise.
         */
        bool hasInformation() { return (!information.empty()); }
        
        /**
         * True on success false otherwise.
         */
        bool wasSuccessful() { return (rvCode >= NR_SUCCESS); }
        
        /**
         * Returns the information string.
         */
		std::string getInformation() {
			return information;
        }
        
        /**
         * Return code accessors
         */
        int getOSErro() { return osCode; };
        int getPackageCode() { return packageCode; }
        int getMonkeyWorksCode()  { return rvCode; }
        
        void setMonkeyWorksCode(int code) { rvCode = code; }
        void setOSErrorCode(int code) { osCode = code; }
        void setPackageCode(int code) { packageCode = code; }
        // overwrites any old information in there
        void setInformation(const std::string &msg);
        void appendInformation(const std::string &msg);
        void appendInformationWithFormat(const std::string msg, ...);
};
}
#endif
