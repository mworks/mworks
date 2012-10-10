/*
 *  SimpleCodecUtilities.h
 *  MWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef SIMPLE_CODEC_UTILITIES_H_
#define SIMPLE_CODEC_UTILITIES_H_

#include "SimpleCodecUtilities.h"
#include "GenericData.h"
#include "Event.h"
#include <map>

using std::map;
using std::string;


namespace mw{
    
extern map<int, string> extract_simple_codec_map(Datum datum);
extern map<int, string> extract_simple_codec_map(shared_ptr<Event> evt);
extern map<string, int> reverse_simple_codec_map(map<int, string> input);
extern Datum construct_simple_codec_datum_from_map(map<int, string> input);

}
#endif
