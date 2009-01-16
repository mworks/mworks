/**
 * MonkeyWorksTypes.h
 *
 * Description: Core typedefs
 *
 * History:
 * David Cox on 6/25/05 - Created.
 * Paul Jankunas on 8/5/05 - Added NetworkStats type for connection stats.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef MONKEYWORKS_TYPES_H_
#define MONKEYWORKS_TYPES_H_
namespace mw {
// event time
typedef	long long MonkeyWorksTime;
// value of network statistics
typedef unsigned long long NetworkStats;
}
#endif