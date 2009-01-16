/*
 *  Test.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "dfindex.h"
#include "boost/filesystem/path.hpp"
#include <iostream>
#include "ScarabDatumWrapper.h"
#include <sys/time.h>
#include <unistd.h>

using namespace::std;

int main( int argc, char *argv[])
{
	struct timeval tv;
	struct timezone tz;
	struct timeval tv2;
	struct timezone tz2;	
	cout << "start indexing" << endl;
	gettimeofday(&tv, &tz);
//	dfindex dfi("/Library/MonkeyWorks/DataFiles/trainingD1.mwk");
	dfindex dfi("/Users/bkennedy/Documents/sandbox/MonkeyWorks_trunk/MonkeyWorksTools/MatlabDataReader/simple4.mwk");
	gettimeofday(&tv2, &tz2);
	cout << "indexing ... took " << tv2.tv_sec-tv.tv_sec << " s" << endl;
	cout << "indexing ... took " << tv2.tv_usec-tv.tv_usec << " us" << endl;

	std::vector<unsigned int> event_codes;
	event_codes.push_back(0);
//	event_codes.push_back(1);
//	event_codes.push_back(2);
//	event_codes.push_back(3);
//	event_codes.push_back(4);
//	event_codes.push_back(5);
//	event_codes.push_back(6);
//	event_codes.push_back(7);
//	event_codes.push_back(8);
//	event_codes.push_back(9);
//	event_codes.push_back(10);
	gettimeofday(&tv, &tz);
//	MonkeyWorksTime spike_time = 3598592801LL;
	vector<ScarabDatumWrapper> events = dfi.events(event_codes);
	gettimeofday(&tv2, &tz2);
	
	cout << "number of events: " << events.size() << " ... took " << tv2.tv_sec-tv.tv_sec << " s" << endl;
	cout << "number of events: " << events.size() << " ... took " << tv2.tv_usec-tv.tv_usec << " us" << endl;
		
	return 0;
}

