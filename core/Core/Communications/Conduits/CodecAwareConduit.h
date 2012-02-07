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
using namespace mw;

#define name_defined_callback_key   "CodeAwareConduit::name_defined_callback_key"

#define CODEC_RESEND_TIMEOUT_MS 100

// A conduit that maintains a local codec (code -> name mappings)
// that it transmits to the other side of the conduit so that it
// can understand event codes without needing a pre-agreed mapping
// Additionally, it receives codecs coming from the conduit so that
// it is possible to register for event callbacks by name rather than
// code
class CodecAwareConduit : public SimpleConduit, public enable_shared_from_this<CodecAwareConduit> {
    
protected:
    
    // A local codec mapping local codes to event names
    // when changed, this will get transmitted over the 
    boost::mutex local_codec_lock;
    map<int, string> local_codec;
    map<string, int> local_reverse_codec;
    int local_codec_code_counter;
    
    boost::mutex remote_codec_lock;
    map<int, string> remote_codec;
    map<string, int> remote_reverse_codec;
    
    map<string, EventCallback> callbacks_by_name;
    
    void transmitCodecEvent();
    void rebuildEventCallbacks();
    
public:
    
    CodecAwareConduit(shared_ptr<EventTransport> _transport,
                      bool correct_incoming_timestamps=false);
    virtual ~CodecAwareConduit(){ 
        // grab these locks, so that we can ensure that 
        // anyone else who had them is done
        boost::mutex::scoped_lock(local_codec_lock);
        boost::mutex::scoped_lock(remote_codec_lock);
    }
    
    
    virtual bool initialize();
    virtual void registerCallbackByName(string name, EventCallback cb);    
    virtual void registerLocalEventCode(int code, string name);
    
    void receiveCodecEvent(shared_ptr<Event> evt);   
    void receiveControlEvent(shared_ptr<Event> evt);
    
    void codeTranslatedCallback(shared_ptr<Event> evt, EventCallback cb, int new_code);
    
    map<int, string> getLocalCodec(){ return local_codec; }
    map<string, int> getLocalReverseCodec(){  return local_reverse_codec; }
    map<int, string> getRemoteCodec();
    map<string, int> getRemoteReverseCodec();

};

#endif
