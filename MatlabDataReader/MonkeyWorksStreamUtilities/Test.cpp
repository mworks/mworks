/*
 *  Test.cpp
 *  MonkeyWorksStreamUtilities
 *
 *  Created by bkennedy on 2/24/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include <iostream>
#include "MonkeyWorksStreamUtilities/monkeyWorksStreamUtilities.h"
#include "mat.h"

using namespace std;

int main() {
	char *mwkfile = "ldobinary:file:///Users/bkennedy/Documents/sandbox/MonkeyWorks_trunk/MonkeyWorksTools/MatlabDataReader/simple2.mwk/simple2.mwk";

	ScarabSession *session = scarab_session_connect(mwkfile);
	
	ScarabDatum *datum = NULL;
	int events = 0;
	while(datum = scarab_read(session)) {
		if(events % 1000 == 0) {
			cout << "event: " << events << endl;
		}
		
		if(getScarabEventCode(datum) == 0) {
			cout << "Codec! at event: " << events << endl;
			mxArray *codec = getScarabEventData(datum);
			MATFile *test = matOpen("/Users/bkennedy/Desktop/test.mat", "w");
			matPutVariable(test, "codec", codec);
			matClose(test);
			
			mxDestroyArray(codec);
		}
		scarab_free_datum(datum);
		events++;
	}
	
	return 0;
}


