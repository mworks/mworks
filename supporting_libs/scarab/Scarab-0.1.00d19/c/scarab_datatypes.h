/* Scarab types */
#ifndef SCARAB_DATATYPES_H
#define SCARAB_DATATYPES_H

#define SCARAB_NULL			0
#define SCARAB_INTEGER		1
#define SCARAB_FLOAT		2
#define SCARAB_FLOAT_INF	3
#define SCARAB_FLOAT_NAN	4
#define SCARAB_DICT			5
#define SCARAB_LIST			6
#define SCARAB_OPAQUE		7
#define SCARAB_MWTIME       8
#define SCARAB_FLOAT_OPAQUE 9

#define FWD(type,name)		type __##name;typedef type __##name name
#define	DEF(type,name)		type __##name

#include <Scarab/scarab_mem.h>
#include <pthread.h>

FWD(struct, ScarabOpaque);
FWD(struct, ScarabDatum);

#include <Scarab/scarab_list.h>
#include <Scarab/scarab_dict.h>

DEF(struct, ScarabOpaque) 
{
	int				size;
	unsigned char	*data;
};

//#undef THREAD_SAFE_SCARAB_DATUM
#define THREAD_SAFE_SCARAB_DATUM

DEF(struct, ScarabDatum)
{
	//void	*memctx;
	union
	{
		long long		integer;
		double			floatp;
		ScarabOpaque	opaque;
		ScarabDict		*dict;
		ScarabList		*list;
		void			*p;
	}
	data;
	ScarabDatum	*attributes;
	int			type;
	int			ref_count; // DDC added
	#ifdef THREAD_SAFE_SCARAB_DATUM
		pthread_mutex_t *mutex;
	#endif
};

#endif



