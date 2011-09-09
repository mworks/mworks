/**
 * NetworkWatcher.h
 *
 * Description: Styled after Java Listeners this class is meant to be a base
 * class for the ScarabServer so that the clients it controls can send 
 * messages to it.  Keeping the server hidden behind this class allows
 * a user to change who messages are sent to.  Any class can derive from
 * this one and register itself with a client to receive the messages.  This
 * is an abstract class and cannot be instantiated.
 *
 * History:
 * Paul Jankunas on 1/4/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef M_NETWORK_WATCHER_H__
#define M_NETWORK_WATCHER_H__
namespace mw {
/**
* Action Codes:
*
*       Name   Code #:      Desc:
*/
#define CONNDC    1          // Connection disconnected.
#define CLITERM   2          // Both Connections on client are DC'ed

class NetworkWatcher {
    public:
        /**
         * Default constructor does nothing.
         */
        NetworkWatcher() {}
        
        /**
         * Pure virtual function must be defined by subclasses.
         * Used to perform some action.
         */
        virtual void notifyWatcher(int actionCode) =0;
};
}
#endif
