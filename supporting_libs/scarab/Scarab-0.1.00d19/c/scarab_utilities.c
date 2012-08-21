#include <Scarab/scarab.h>
#include "Scarab/encoding_ldo.h"
#include <Scarab/scarab_list.h>
#include <Scarab/scarab_dict.h>
#include <stdio.h>
#include <string.h>

// DDC kludge necessary to do linux compile, for some reason
/*#define S_MAGIC_SIZE  	7

const unsigned char S_MAGIC[] = {
	0x89, 'C', 'B', 'F', VERSION, VER_MAJOR, VER_MINOR
};*/

#define MAGIC_SIZE		7
#define S_MAGIC_SIZE	MAGIC_SIZE
#define S_MAGIC			MAGIC





int scarab_create_file(const char *filename){

	FILE *fd;
	//int i;
	
	fd = fopen(filename, "w");
	
	// TODO check that the filename is OKAY!!!!
	if(fd == NULL){ // no file
		return -1;
	}
	
	// DDC MAGIC_SIZE kludge necessary to do linux compile, for some reason
	fwrite(S_MAGIC, S_MAGIC_SIZE, 1, fd);
	
	fclose(fd);
	return 0;
}



int scarab_opaque_is_string(ScarabDatum *d) {
    unsigned char *data = d->data.opaque.data;
    int size = d->data.opaque.size;
    // data is a string if its last byte (and only its last byte) is zero
    return memchr(data, 0, size) == (data + (size-1));
}



char* scarab_extract_opaque(ScarabDatum *d, int *size) {
    char *returnstring;
    
    *size = d->data.opaque.size;
    returnstring = (char *)malloc(*size);
    memcpy(returnstring, d->data.opaque.data, *size);
    
    return returnstring;
}



char* scarab_extract_string(ScarabDatum *d) {
    int size;
    return scarab_extract_opaque(d, &size);
}



double scarab_extract_float(ScarabDatum *datum){
#if __LITTLE_ENDIAN__
	return *((double *)(datum->data.opaque.data));
#else
	int i;
	unsigned char swap_buffer[sizeof(double)];
	unsigned char *datum_bytes = (unsigned char *)datum->data.opaque.data;
	for(i = 0; i < sizeof(double); i++){
		swap_buffer[i] = datum_bytes[sizeof(double) - i - 1];
	}
	
	return *((double *)swap_buffer);
#endif
}
