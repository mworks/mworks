/*
 *  CodecAwareConduit.h
 *  MWorksCore
 *
 *  Created by David Cox on 12/10/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef CODEC_AWARE_CONDUIT_H_
#define CODEC_AWARE_CONDUIT_H_

#include "SimpleConduit.h"


BEGIN_NAMESPACE_MW


#define name_defined_callback_key   "CodeAwareConduit::name_defined_callback_key"

#define CODEC_RESEND_TIMEOUT_MS 5000

// A conduit that maintains a local codec (code -> name mappings)
// that it transmits to the other side of the conduit so that it
// can understand event codes without needing a pre-agreed mapping
// Additionally, it receives codecs coming from the conduit so that
// it is possible to register for event callbacks by name rather than
// code
class CodecAwareConduit : public SimpleConduit, public boost::enable_shared_from_this<CodecAwareConduit> {
    
protected:
    boost::recursive_mutex conduit_mutex;
    typedef boost::recursive_mutex::scoped_lock scoped_lock;
    
    // A local codec mapping local codes to event names
    // when changed, this will get transmitted over the 
    map<int, string> local_codec;
    map<string, int> local_reverse_codec;
    
    map<int, string> remote_codec;
    map<string, int> remote_reverse_codec;
    boost::condition_variable_any remote_codec_cond;
    
    map<string, EventCallback> callbacks_by_name;
    
    void transmitCodecEvent();
    void addEventCallback(const std::string &name, EventCallback cb);
    void rebuildEventCallbacks();
    void waitForRemoteCodec(scoped_lock &lock);
    
public:
    
    CodecAwareConduit(shared_ptr<EventTransport> _transport, bool correct_incoming_timestamps=false) :
        SimpleConduit(_transport, correct_incoming_timestamps)
    { }
    
    virtual ~CodecAwareConduit(){ 
        // grab the lock, so that we can ensure that 
        // anyone else who had it is done
        scoped_lock lock(conduit_mutex);
    }
    
    
    virtual bool initialize();
    virtual void registerCallbackByName(string name, EventCallback cb);    
    virtual void registerLocalEventCode(int code, string name);
    
    void receiveCodecEvent(shared_ptr<Event> evt);   
    void receiveControlEvent(shared_ptr<Event> evt);
    
    static void codeTranslatedCallback(shared_ptr<Event> evt, EventCallback cb, int new_code);
    
    virtual void handleCallbacks(shared_ptr<Event> evt);
    
    //map<int, string> getLocalCodec(){ return local_codec; }
    //map<string, int> getLocalReverseCodec(){  return local_reverse_codec; }
    map<int, string> getRemoteCodec();
    map<string, int> getRemoteReverseCodec();

};


END_NAMESPACE_MW


#endif
