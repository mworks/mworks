/*
 *  mem_simple.c
 *  Scarab
 *
 *  Created by David Cox on 7/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

//#include "mem_simple.h"

#include <scarab.h>
#include <stdlib.h>
//#include <malloc.h>


void scarab_mem_init(void)
{
}
  
void *scarab_mem_malloc(int bytes) 
{
	return malloc(bytes);
}


void *scarab_mem_calloc(int num, int size)
{
    return calloc(num, size);
}


/**
 * Frees a variable within a memory context
 */
void scarab_mem_free(void *ptr) 
{ 
	free(ptr);
}
