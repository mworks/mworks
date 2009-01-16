/*
 * Datatype comparison and hashing.
 */

#include <stdlib.h>
#include <scarab.h>

#ifndef MIN
#define MIN(x, y) ( x>y ? y : x)
#endif

/*
 * Returns hash value 0 <= h < modulo given the datum.
 */
int
scarab_hash(ScarabDatum *datum, int modulo, int atry) 
{ 
	int		hv = 0;
	int		i = 0;
	int		s = 0;

	switch (datum->type) 
    {
    case SCARAB_FLOAT_INF:
		hv = atry*atry;
		break;
    case SCARAB_FLOAT_NAN:
		hv = atry;
		break;
    case SCARAB_INTEGER:
    case SCARAB_FLOAT:
		hv = (int)datum->data.integer;
		break;
    case SCARAB_OPAQUE:
		s = datum->data.opaque.size;
		for (i = 4*atry; i< ((s > (4*atry+4)) ? (4*atry+4) : s); i++)
		{
			hv <<= 8;
			hv += datum->data.opaque.data[i];
		} 
		break;
    default:
		hv = datum->data.integer * (atry+1);
    }    
	return hv % modulo;
}

/*
 * Returns	0 if datum1 != datum2,
 *			1 if datum1 == datum2.
 */
int
scarab_equals(ScarabDatum *datum1, ScarabDatum *datum2) 
{
	int	i;
	int	s1;
	int	s2;
	ScarabOpaque	*o1;
	ScarabOpaque	*o2;

	if (datum1 != NULL
		&& datum2 != NULL
		&& datum1->type == datum2->type)
    {
		switch (datum1->type) 
		{
		case SCARAB_NULL:
		case SCARAB_FLOAT_NAN:
		case SCARAB_FLOAT_INF:
			return 1;
		case SCARAB_OPAQUE:
			o1 = &datum1->data.opaque;
			o2 = &datum2->data.opaque;
			s1 = o1->size;
			s2 = o2->size;
			if (s1 != s2)
				return 0;
			for (i = 0; i < s1; i++) 
			{
				if (o1->data[i] != o2->data[i])
					return 0;
			}
			return 1;
			break;
		case SCARAB_INTEGER:
			return datum1->data.integer == datum2->data.integer;
		case SCARAB_FLOAT:
			return datum1->data.floatp == datum2->data.floatp;
		default:
			return datum1->data.p == datum2->data.p;
		}
    }
	return 0;
}

