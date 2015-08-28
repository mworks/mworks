/**
 * ScarabWriteConnection.h
 * 
 * Description:
 *
 * History:
 * Paul Jankunas on 12/1/04 - Created.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#ifndef _SCARAB_WRITE_CONNECTION_H__
#define _SCARAB_WRITE_CONNECTION_H__

#include <unordered_set>

#include "ScarabConnection.h"
#include "EventBuffer.h"


BEGIN_NAMESPACE_MW


class ScarabWriteConnection : public ScarabConnection {
    private:
        shared_ptr<EventBufferReader> buffer_reader;
        const std::unordered_set<int> excluded_event_codes;
    
    public:
        /** 
         * Constructor delegates all responsibilities to the parent class
         */
        ScarabWriteConnection(const shared_ptr<EventBuffer> &_event_buffer,
                              const std::string &uri,
                              const std::unordered_set<int> &excluded_event_codes = {});
                
        /**
         * Services the connection, in this case writing data to it.
         */
        int service();
        
        /**
         * Starts the servicing thread, interval tells the scheduler how often
         * this thread should run.
         */
        void startThread(int interval);
        void stopThread();
};


END_NAMESPACE_MW


#endif
