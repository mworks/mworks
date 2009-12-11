/*
 *  SimpleCodecUtilities.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "SimpleCodecUtilities.h"
#include "GenericData.h"
#include <map>

using namespace mw;
using namespace std;


map<int, string> extract_simple_codec_map(Datum datum){
    
    map<int, string> result;
    vector<Datum> keys = datum.getKeys();
    vector<Datum>::iterator key_iter;
    for(key_iter = keys.begin(); key_iter != keys.end(); key_iter++){
        int key = (*key_iter).getInteger();
        
        Datum value_datum = datum.getElement(*key_iter);
        if(value_datum.isDictionary()){
            value_datum = value_datum.getElement("tagname");
        }
        
        result[key] = value_datum.getString();
    }
    
    return result;
}

map<int, string> extract_simple_codec_map(shared_ptr<Event> evt){
    return extract_simple_codec_map(evt->getData());
}

map<string, int> reverse_simple_codec_map(map<int, string> input){
    
    map<string, int> output;
    map<int, string>::iterator i;
    for(i = input.begin(); i != input.end(); ++i){
        
        output[(*i).second] = (*i).first;
    }
    
    return output;
}