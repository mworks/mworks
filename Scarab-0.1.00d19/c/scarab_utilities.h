/**
 * scarab_utilities.h
 *
 * Description: Use this file to add utility functions that will be used 
 * outside of the scarab library code to operate on Scarab objects.
 *
 * History:
 * Dave Cox ?? ?? ?? - Created with scarab_extract_string
 *
 * Copyright (c) 2004 MIT. All rights reserved.
 */

char * scarab_extract_string(ScarabDatum *d);

// DDC added 8/2005
// solve the chicken and egg problem encountered with starting a new
// file (i.e. no magic)
int scarab_create_file(const char *filename);

// Just what it sounds like
//ScarabDatum * scarab_deep_copy(void *memctx, ScarabDatum *datum);


void
scarab_recursive_free_children(ScarabDatum datum);

void
scarab_recursive_free(ScarabDatum *d);
