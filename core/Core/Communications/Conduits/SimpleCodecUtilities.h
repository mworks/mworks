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

#include "GenericData.h"
#include "Event.h"
#include <map>

using std::map;
using std::string;


BEGIN_NAMESPACE_MW

    
extern map<int, string> extract_simple_codec_map(Datum datum);
extern map<int, string> extract_simple_codec_map(shared_ptr<Event> evt);
extern map<string, int> reverse_simple_codec_map(map<int, string> input);
extern Datum construct_simple_codec_datum_from_map(map<int, string> input);


END_NAMESPACE_MW


#endif
