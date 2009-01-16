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

#include "ScarabConnection.h"
#include "EventBuffer.h"
namespace mw {
class ScarabWriteConnection : public ScarabConnection {
    private:
	shared_ptr<EventBufferReader> buffer_reader;
        
    public:
        /** 
         * Constructor delegates all responsibilities to the parent clas
         */
        ScarabWriteConnection(shared_ptr<BufferManager> _buffer_manager, std::string uri);
                
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
}
#endif
