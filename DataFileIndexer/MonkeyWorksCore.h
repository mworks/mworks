/*
 *  MonkeyWorksCore.h
 *  DataFileIndexer
 *
 *  Created by bkennedy on 2/20/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef MonkeyWorksCore_
#define MonkeyWorksCore_

#define SCARAB_EVENT_CODEC_CODE_INDEX	0
#define SCARAB_EVENT_TIME_INDEX			1
#define SCARAB_EVENT_PAYLOAD_INDEX		2

typedef long long MonkeyWorksTime;

static inline MonkeyWorksTime MIN_MONKEY_WORKS_TIME() {
	MonkeyWorksTime min = 1;
	for(unsigned int i=0; i<(sizeof(MonkeyWorksTime)*8)-1; ++i) {
		min = min << 1;
	}
	return min;
}

static inline MonkeyWorksTime MAX_MONKEY_WORKS_TIME() {
	MonkeyWorksTime max = 0;
	MonkeyWorksTime min = MIN_MONKEY_WORKS_TIME();
	max = ~max;
	max = max ^ min;
	return max;
}

#endif // MonkeyWorksCore_
