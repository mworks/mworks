/*
 *  SimpleCodecUtilities.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "SimpleCodecUtilities.h"
#include "GenericData.h"
#include <map>

namespace mw {


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

Datum construct_simple_codec_datum_from_map(map<int, string> input){
    
    int n_items = input.size();
    
    Datum codec_datum(M_DICTIONARY, n_items);
    
    map<int, string>::iterator iter;
    for(iter = input.begin(); iter != input.end(); ++iter){
        pair<int, string> keyval = *iter;
        int code = keyval.first;
        string tagname = keyval.second;
        
        Datum variable_info(M_DICTIONARY, 1);
        variable_info.addElement("tagname", tagname);
        codec_datum.addElement(Datum((long)code), variable_info);
    }
    
    return codec_datum;
}
}
