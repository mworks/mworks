//
//  MWorksPython.h
//  MWorksPython
//
//  Created by Christopher Stawarz on 10/31/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef MWorksPython_h
#define MWorksPython_h

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

 __attribute__((visibility("default")))
bool MWorksPythonInit(bool initSignals);

#ifdef __cplusplus
}
#endif


#endif /* MWorksPython_h */
