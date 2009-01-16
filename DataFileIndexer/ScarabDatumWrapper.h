/*
 *  ScarabDatumWrapper.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/19/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef SCARAB_DATUM_WRAPPER_H
#define SCARAB_DATUM_WRAPPER_H

#include "Scarab/scarab.h"

class ScarabDatumWrapper
	{
	protected:
		ScarabDatum *datum;		
	public:
		ScarabDatumWrapper(ScarabDatum *);
		ScarabDatumWrapper(const ScarabDatumWrapper &sdw);
		~ScarabDatumWrapper();
		ScarabDatum *getDatum() const;
	};


#endif //SCARAB_DATUM_WRAPPER_H
