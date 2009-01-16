#include <Scarab/scarab.h>
#include "Scarab/encoding_ldo.h"
#include <Scarab/scarab_list.h>
#include <Scarab/scarab_dict.h>
#include <stdio.h>

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



char *scarab_extract_string(ScarabDatum *d){

    ScarabOpaque opaque;
    char *returnstring;
    int i;
    
    
    opaque = d->data.opaque;

    returnstring = (char *)calloc(opaque.size, sizeof(char));
    
    for(i = 0; i < opaque.size; i++){
        
        returnstring[i] = (char)opaque.data[i];
    }
    
    return returnstring;

}



