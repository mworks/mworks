/*
 *  alut.c
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/16/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <memory.h>
//#include "OpenAL32/Include/alMain.h"
#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#include "alut.h"

#if defined _MSC_VER
	#pragma pack (push,1) 							/* Turn off alignment */
#elif defined __GNUC__
	#define PADOFF_VAR __attribute__((packed))
#endif

#ifndef PADOFF_VAR
	#define PADOFF_VAR
#endif

typedef struct                                  /* WAV File-header */
{
  ALubyte  Id[4]			PADOFF_VAR;
  ALsizei  Size				PADOFF_VAR;
  ALubyte  Type[4]			PADOFF_VAR;
} WAVFileHdr_Struct;

typedef struct                                  /* WAV Fmt-header */
{
  ALushort Format			PADOFF_VAR;
  ALushort Channels			PADOFF_VAR;
  ALuint   SamplesPerSec	PADOFF_VAR;
  ALuint   BytesPerSec		PADOFF_VAR;
  ALushort BlockAlign		PADOFF_VAR;
  ALushort BitsPerSample	PADOFF_VAR;
} WAVFmtHdr_Struct;

typedef struct									/* WAV FmtEx-header */
{
  ALushort Size				PADOFF_VAR;
  ALushort SamplesPerBlock	PADOFF_VAR;
} WAVFmtExHdr_Struct;

typedef struct                                  /* WAV Smpl-header */
{
  ALuint   Manufacturer		PADOFF_VAR;
  ALuint   Product			PADOFF_VAR;
  ALuint   SamplePeriod		PADOFF_VAR;
  ALuint   Note				PADOFF_VAR;
  ALuint   FineTune			PADOFF_VAR;
  ALuint   SMPTEFormat		PADOFF_VAR;
  ALuint   SMPTEOffest		PADOFF_VAR;
  ALuint   Loops			PADOFF_VAR;
  ALuint   SamplerData		PADOFF_VAR;
  struct
  {
    ALuint Identifier		PADOFF_VAR;
    ALuint Type				PADOFF_VAR;
    ALuint Start			PADOFF_VAR;
    ALuint End				PADOFF_VAR;
    ALuint Fraction			PADOFF_VAR;
    ALuint Count			PADOFF_VAR;
  }      Loop[1]			PADOFF_VAR;
} WAVSmplHdr_Struct;

typedef struct                                  /* WAV Chunk-header */
{
  ALubyte  Id[4]			PADOFF_VAR;
  ALuint   Size				PADOFF_VAR;
} WAVChunkHdr_Struct;


#ifdef PADOFF_VAR			    				/* Default alignment */
	#undef PADOFF_VAR
#endif


ALuint	_bs_ALuint(ALuint orig){

	#if __LITTLE_ENDIAN__
		return orig;
	#endif

	ALuint swapped;
	unsigned char *orig_pointer = (unsigned char *)&orig;
	unsigned char *swapped_pointer = (unsigned char *)&swapped;
	for(unsigned int i = 0; i < sizeof(ALuint); i++){
		swapped_pointer[i] = orig_pointer[sizeof(ALuint) - i - 1];
	}
	
	return swapped;
}

ALuint	_bs_ALushort(ALushort orig){

	#if	__LITTLE_ENDIAN__
		return orig;
	#endif

	ALushort swapped;
	unsigned char *orig_pointer = (unsigned char *)&orig;
	unsigned char *swapped_pointer = (unsigned char *)&swapped;
	for(unsigned int i = 0; i < sizeof(ALushort); i++){
		swapped_pointer[i] = orig_pointer[sizeof(ALushort) - i - 1];
	}
	
	return swapped;
}


ALUTAPI ALvoid ALUTAPIENTRY alutInternalLoadWAVFile(ALbyte *file,ALenum *format,ALvoid **data,ALsizei *size,ALsizei *freq)
{
	WAVChunkHdr_Struct ChunkHdr;
	WAVFmtExHdr_Struct FmtExHdr;
	WAVFileHdr_Struct FileHdr;
	WAVSmplHdr_Struct SmplHdr;
	WAVFmtHdr_Struct FmtHdr;
	FILE *Stream;
	
	*format=AL_FORMAT_MONO16;
	*data=(ALvoid *)NULL;
	*size=0;
	*freq=22050;
	//*loop=AL_FALSE;
	
	
	if (file){
		Stream=fopen(file,"rb");
		
		if (Stream) {
			fread(&FileHdr,1,sizeof(WAVFileHdr_Struct),Stream);
			FileHdr.Size=((FileHdr.Size+1)&~1)-4;
			
			while ( (_bs_ALuint(FileHdr.Size) !=0) &&
					(fread(&ChunkHdr,1,sizeof(WAVChunkHdr_Struct),Stream))){
				
				if (!memcmp(ChunkHdr.Id,"fmt ",4)){
					fread(&FmtHdr,1,sizeof(WAVFmtHdr_Struct),Stream);
					if ((_bs_ALushort(FmtHdr.Format) == 0x0001)||(_bs_ALushort(FmtHdr.Format) == 0xFFFE)){
						if (_bs_ALushort(FmtHdr.Channels) == 1){
							*format=(_bs_ALushort(FmtHdr.BitsPerSample)==4 ? alGetEnumValue((const ALchar *)"AL_FORMAT_MONO_IMA4"):(_bs_ALushort(FmtHdr.BitsPerSample)==8 ? AL_FORMAT_MONO8:AL_FORMAT_MONO16));
						}else if (_bs_ALushort(FmtHdr.Channels)==2){
							*format=(_bs_ALushort(FmtHdr.BitsPerSample)==4?alGetEnumValue((const ALchar *)"AL_FORMAT_STEREO_IMA4"):(_bs_ALushort(FmtHdr.BitsPerSample)==8?AL_FORMAT_STEREO8:AL_FORMAT_STEREO16));
						}
						
						*freq=_bs_ALuint(FmtHdr.SamplesPerSec);
						fseek(Stream,_bs_ALuint(ChunkHdr.Size)-sizeof(WAVFmtHdr_Struct),SEEK_CUR);
					} else if (_bs_ALuint(FmtHdr.Format)==0x0011){
						if (_bs_ALushort(FmtHdr.Channels)==1){
							*format=alGetEnumValue((const ALchar *)"AL_FORMAT_MONO_IMA4");
						} else if (_bs_ALushort(FmtHdr.Channels)==2){
							*format=alGetEnumValue((const ALchar *)"AL_FORMAT_STEREO_IMA4");
						}
						
						*freq=_bs_ALuint(FmtHdr.SamplesPerSec);
						fseek(Stream,_bs_ALuint(ChunkHdr.Size)-sizeof(WAVFmtHdr_Struct),SEEK_CUR);
					} else {
						fread(&FmtExHdr,1,sizeof(WAVFmtExHdr_Struct),Stream);
						fseek(Stream,_bs_ALuint(ChunkHdr.Size)-sizeof(WAVFmtHdr_Struct)-sizeof(WAVFmtExHdr_Struct),SEEK_CUR);
					}
				} else if (!memcmp(ChunkHdr.Id,"data",4)){
					*size = _bs_ALuint(ChunkHdr.Size);
					*data = malloc(_bs_ALuint(ChunkHdr.Size)+31);
					if (*data){
						fread(*data,_bs_ALushort(FmtHdr.BlockAlign),_bs_ALuint(ChunkHdr.Size)/_bs_ALushort(FmtHdr.BlockAlign),Stream);
					}
					
					#if __BIG_ENDIAN__
						int bytes_per_sample = _bs_ALushort(FmtHdr.BitsPerSample)/8;
						
						fprintf(stderr, "bytes per sample: %d", bytes_per_sample);fflush(stderr);
						unsigned char *temp_pointer = (unsigned char *)malloc(bytes_per_sample);
						
						for(int i = 0; i < _bs_ALuint(ChunkHdr.Size); i+=bytes_per_sample){
							unsigned char *orig_pointer = (unsigned char *)(*data)+i; 
							for(int j = 0; j < bytes_per_sample; j++){
								temp_pointer[j] = orig_pointer[j];
							}
							
							for(int j = 0; j < bytes_per_sample; j++){
								orig_pointer[j] = temp_pointer[bytes_per_sample - j - 1];
							}
						}
						
						free(temp_pointer);
					#endif
					memset(((char *)*data)+_bs_ALuint(ChunkHdr.Size),0,31);
				} else if (!memcmp(ChunkHdr.Id,"smpl",4)) {
				
					fread(&SmplHdr,1,sizeof(WAVSmplHdr_Struct),Stream);
					//*loop = (SmplHdr.Loops ? AL_TRUE : AL_FALSE);
					fseek(Stream,_bs_ALuint(ChunkHdr.Size)-sizeof(WAVSmplHdr_Struct),SEEK_CUR);
				} else {
					fseek(Stream,_bs_ALuint(ChunkHdr.Size),SEEK_CUR);
				}
				
				fseek(Stream,_bs_ALuint(ChunkHdr.Size)&1,SEEK_CUR);
				FileHdr.Size-=(((_bs_ALuint(ChunkHdr.Size)+1)&~1)+8);
			}
			fclose(Stream);
		}
		
	}
}


ALUTAPI ALvoid ALUTAPIENTRY alutInternalUnloadWAV(ALenum format,ALvoid *data,ALsizei size,ALsizei freq)
{
	if (data){
		free(data);
	}
}
