#ifndef ENCODING_LDO_H
#define ENCODING_LDO_H

#include <Scarab/scarab.h>

/*
 * LDO binary (Layer0) wire constants.
 */

#define VER_MAJOR 0x00
#define VER_MINOR 0x00

#define VERSION				0x01
#define INTEGER_N			0x02
#define INTEGER_P			0x03
#define FLOAT_NN			0x04
#define FLOAT_NP			0x05
#define FLOAT_PN			0x06
#define FLOAT_PP			0x07
#define FLOAT_INF			0x08
#define FLOAT_NAN			0x09
#define OPAQUE				0x0A
#define LDO_NULL			0x0B
#define LIST			    0x0C
#define DICTIONARY			0x0D
#define DEFINE_REFERENCE	0x0E
#define REFERENCE			0x0F
#define ATTRIBUTES			0x10
#define FLOAT_OPAQUE		0x11

/* 
 * Error codes
 */
#define	LDO_SELF				&scarab_ldo_encoder
#define LDO_ERR_NONE			scarab_errcode(LDO_SELF,	0)
#define LDO_ERR_BAD_HEADER		scarab_errcode(LDO_SELF,	1)
#define	LDO_ERR_DBL_NS			scarab_errcode(LDO_SELF,	2)
#define LDO_ERR_IO				scarab_errcode(LDO_SELF,	3)

#define	LDO_ERR_MAX				100

FWD(struct, LDO_Session);

DEF(struct, LDO_Session)
{
	void           *memctx;
	ScarabDatum    *refs;
};


extern const unsigned char MAGIC[];

#endif
