/*
 *  MWorksCore.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/20/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef MWorksCore_
#define MWorksCore_

#define SCARAB_EVENT_CODEC_CODE_INDEX	0
#define SCARAB_EVENT_TIME_INDEX			1
#define SCARAB_EVENT_PAYLOAD_INDEX		2

typedef long long MWorksTime;

static inline MWorksTime MIN_MONKEY_WORKS_TIME() {
	MWorksTime min = 1;
	for(unsigned int i=0; i<(sizeof(MWorksTime)*8)-1; ++i) {
		min = min << 1;
	}
	return min;
}

static inline MWorksTime MAX_MONKEY_WORKS_TIME() {
	MWorksTime max = 0;
	MWorksTime min = MIN_MONKEY_WORKS_TIME();
	max = ~max;
	max = max ^ min;
	return max;
}

#endif // MWorksCore_
