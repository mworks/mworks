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


#pragma GCC visibility push(default)

#ifdef __cplusplus
extern "C" {
#endif


bool MWorksPythonInit(bool initSignals);
int MWorksPythonMain(int argc, char **argv);


#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop


#endif /* MWorksPython_h */
