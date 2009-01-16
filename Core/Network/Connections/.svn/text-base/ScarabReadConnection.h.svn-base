/**
 * ScarabReadConnection.h
 * 
 * Description:
 *
 * History:
 * Paul Jankunas on 12/1/04 - Created.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#ifndef _SCARAB_READ_CONNECTION_H__
#define _SCARAB_READ_CONNECTION_H__

#include "ScarabConnection.h"
namespace mw {
class ScarabReadConnection : public ScarabConnection {
    public:
        /** 
         * Constructor delegates all responsibilities to the parent clas
         */
        ScarabReadConnection(shared_ptr<BufferManager> _buffer_manager, std::string uri);
        
        /**
         * Services the connection, in this case reading data from it.
         */
        int service();
        
        /**
         * Starts the servicing thread, interval tells the scheduler how often
         * this thread should run.
         */
        void startThread(int interval);
        
        /**
         * Parses the list into an event.
         */
        Event * generateEventFromList(ScarabDatum * listDatum);
};
}
#endif
