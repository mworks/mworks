/*
 *  ScarabDatumWrapper.cpp
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "ScarabDatumWrapper.h"

ScarabDatumWrapper::ScarabDatumWrapper(ScarabDatum *new_datum) {
	datum = scarab_copy_datum(new_datum);
}

ScarabDatumWrapper::ScarabDatumWrapper(const ScarabDatumWrapper &sdw) {
	datum = scarab_copy_datum(sdw.getDatum());
}


ScarabDatumWrapper::~ScarabDatumWrapper() {
	scarab_free_datum(datum);
}

ScarabDatum *ScarabDatumWrapper::getDatum() const {
	return datum;
}



