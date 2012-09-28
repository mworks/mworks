#include <stdio.h>
#include <math.h>
#include <scarab.h>
#include "encoding_ldo.h"


const unsigned char MAGIC[] = {
	0x89, 'C', 'B', 'F', VERSION, VER_MAJOR, VER_MINOR
};


/*
 * Prototypes of internal functions
 */
extern ScarabEncoderEngine scarab_ldo_encoder;

static int		ldo_init(void);
static int      ldo_session_accept(ScarabSession * session);
static int      ldo_session_connect(ScarabSession * session);
static int      ldo_session_close(ScarabSession * session);
static int      ldo_write_typecode(ScarabSession * session,

								   unsigned char type_code);
static unsigned char ldo_read_typecode(ScarabSession * session);
static long long     ldo_readber(ScarabSession * session);
static double   ldo_readfloat(ScarabSession * session, int sign, int esign);
static int      ldo_writeber(ScarabSession * session, long long val);
static int      ldo_write_opaque(ScarabSession * session, const char *value, int len);
static int      ldo_write_float(ScarabSession * session, double d);
static int      ldo_write_float_inf(ScarabSession * session);
static int      ldo_write_float_nan(ScarabSession * session);
static int      ldo_write_integer(ScarabSession * session, long long val);
static int      ldo_write_null(ScarabSession * session);
static int      ldo_write_dict(ScarabSession * session, ScarabDict * dict);
static int      ldo_write_list(ScarabSession * session, ScarabList * list);
static void     ldo_putref(ScarabSession * session, int reference,

						   ScarabDatum * obj);
static ScarabDatum *ldo_getref(ScarabSession * session, int reference);
static ScarabDatum *ldo_read(ScarabSession * session);
static ScarabDatum *ldo_read_dict(ScarabSession * session);
static ScarabDatum *ldo_read_list(ScarabSession * session);
static char    *ldo_strerr(int code);

static int ldo_seek(ScarabSession *session, long int offset, int origin);
static long int ldo_tell(ScarabSession *session);

static int
ldo_init(void)
{
	return 0;
}

static int
ldo_session_accept(ScarabSession * session)
{
	LDO_Session    *s;

	s = (LDO_Session*)scarab_mem_malloc(sizeof(LDO_Session));

	s->refs = scarab_dict_new(11, scarab_dict_times2);

	int write_size = scarab_session_write(session, (const void *)MAGIC, 7);
	scarab_session_send(session);
	if (write_size != 7 )
		return scarab_session_seterr(session, LDO_ERR_BAD_HEADER);



	scarab_session_set_encoder(session, &scarab_ldo_encoder, s);

	return 0;
}

static int
ldo_session_connect(ScarabSession * session)
{
	LDO_Session    *s;
	unsigned char  expected_magic[sizeof(MAGIC)];
	int             i;

	s = (LDO_Session*)scarab_mem_malloc(sizeof(LDO_Session));

	s->refs = scarab_dict_new(11, scarab_dict_times2);

	int magic_size = 0;
	if ((magic_size = scarab_session_read(session, expected_magic, sizeof(MAGIC)))
                                                        != sizeof(MAGIC)) {
		fprintf(stderr,"Wrong size magic (is %d, should be %lu)\n",
						magic_size, sizeof(MAGIC));fflush(stderr);
		return scarab_session_seterr(session, LDO_ERR_BAD_HEADER);
	}

	for (i = 0; i < sizeof(MAGIC); i++) {
		if (expected_magic[i] != MAGIC[i]) {
			fprintf(stderr,"Wrong magic (in pos %d:  is: %x, should be %x)\n",
							i, MAGIC[i], expected_magic[i]);
			fflush(stderr);
			return scarab_session_seterr(session, LDO_ERR_BAD_HEADER);
		}
	}

	
	scarab_session_set_encoder(session, &scarab_ldo_encoder, s);

	return 0;
}

static int
ldo_session_close(ScarabSession * session)
{
	return 0;
}

static int
ldo_write_typecode(ScarabSession * session, unsigned char type_code)
{
	if (scarab_session_write(session, &type_code, 1) != 1)
		return scarab_session_seterr(session, LDO_ERR_IO);
	return 0;
}

static unsigned char
ldo_read_typecode(ScarabSession * session)
{
	unsigned char   type_code;

	if (scarab_session_read(session, &type_code, 1) != 1)
		return 255;
	return type_code;
}

static long long
ldo_readber(ScarabSession * session)
{
	long long           val;
	unsigned char   c;
    int             b_read;
    
	val = 0;
	b_read = scarab_session_read(session, &c, 1);
  //  fprintf(stderr, "bytes read from readber %d\n", b_read);
	while ((c & 0x80) != 0)
	{
		val = (val << 7) + (c & 0x7f);
		b_read = scarab_session_read(session, &c, 1);
  //      fprintf(stderr, "bytes read from readber %d\n", b_read);
	}
	val = (val << 7) + c;
	return val;
}

static double
ldo_readfloat(ScarabSession * session, int sign, int esign)
{
	long long            mantissa;
	double          exponent;

	mantissa = ldo_readber(session);
	exponent = pow(10, esign * ldo_readber(session));

	return mantissa * exponent;
}

static int
ldo_seek(ScarabSession *session, long int offset, int origin)
{
	return scarab_session_seek(session, offset, origin);
//	return -1;
}

static long int
ldo_tell(ScarabSession *session)
{
	return scarab_session_tell(session);
//	return -1;
}

static int
ldo_writeber(ScarabSession * session, long long val) //ddc edit
{
	unsigned char   cs[64];	//ddc edit
	int             p;

	/* 
	 * To prevent recursing I created a buffer that was more
	 * than large enough.  Therefore I set the 0x80 flag on
	 * _every_ byte and then just remove it from the last one
	 * in the stack before I write the stack over the network.
	 */

	p = sizeof(cs);
	while (val >= 128)  // ddc edit
 	{
		cs[--p] = (val & 0x7f) | 0x80;
		val >>= 7;
	}

	cs[--p] = (unsigned char)val | 0x80;
	cs[sizeof(cs) - 1] &= 0x7f;

	if (scarab_session_write(session, cs + p,
		sizeof(cs) - p) != (int)(sizeof(cs) - p))
	{
		return scarab_session_seterr(session, LDO_ERR_IO);
	}
	return 0;
}

static int
ldo_write_opaque(ScarabSession * session, const char *value, int len)
{
	if (ldo_write_typecode(session, OPAQUE) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);
	if (ldo_writeber(session, len) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);
	if (scarab_session_write(session, value, len) != len)
		return scarab_session_seterr(session, LDO_ERR_IO);
	
	//scarab_session_send(session);
	return 0;
}

static int
ldo_write_float(ScarabSession * session, double d) {
// DDC: the following sucks ass.  Someone needs to fix this
/********************************************************
*           UBER L33T HAXOR WARNING HAHAHAHAHAHA        *
*********************************************************/
// Sending doubles as byte streams because its really hard to figure out
// how to send doubles in a platform independent way with the scarab objects
// strongly type. The type here is spec as FLOAT_NAN....thats just a label
// so that we know we are using the hack.

// DDC: kludge on top of kludge. The pre-existing kludge is not platform
//		independent; therefore we must handle different byte orders
//		Ultimately, the underlying problem needs to be dealt with

	#if	__LITTLE_ENDIAN__
		char * haxxor = (char *)&d;
    #else
		char swap_bytes[sizeof(double)];
		char *double_bytes = (char *)(&d);
		int i;
		for(i = 0; i < sizeof(double); i++){
			swap_bytes[i] = double_bytes[sizeof(double) - i - 1];
		}
		
		char *haxxor = (char *)swap_bytes;
	#endif
	
	int len = sizeof(double);
	if (ldo_write_typecode(session, FLOAT_OPAQUE) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);
	if (ldo_writeber(session, len) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);
	if (scarab_session_write(session, haxxor, len) != len)
		return scarab_session_seterr(session, LDO_ERR_IO);

	return 0;
}

static int
ldo_write_float_inf(ScarabSession * session)
{
	return scarab_session_seterr(session, LDO_ERR_DBL_NS);
}

static int
ldo_write_float_nan(ScarabSession * session)
{
	return scarab_session_seterr(session, LDO_ERR_DBL_NS);
}

static int
ldo_write_integer(ScarabSession * session, long long val)
{
	unsigned char   type_code;

	if (val < 0)
	{
		type_code = INTEGER_N;
		val = abs(val);
	}
	else
	{
		type_code = INTEGER_P;
	}
//    printf("ldo_write_integer called value %d", val);
	if (ldo_write_typecode(session, type_code) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);
	if (ldo_writeber(session, val) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);

	
	//scarab_session_send(session);
	

	return 0;
}

static int
ldo_write_null(ScarabSession * session)
{
	if (ldo_write_typecode(session, LDO_NULL) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);
	return 0;
}

static int
ldo_write_dict(ScarabSession * session, ScarabDict * dict)
{
	int             l;
	int             i;
	int             r;

	if (ldo_write_typecode(session, DICTIONARY) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);

	if (ldo_writeber(session, dict->size) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);

	l = dict->tablesize;
	for (i = 0; i < l; i++)
	{
		if (dict->keys[i])
		{
			r = scarab_write(session, dict->keys[i]);
			if (r)
				return r;
			r = scarab_write(session, dict->values[i]);
			if (r)
				return r;
		}
	}
	return 0;
}

static int
ldo_write_list(ScarabSession * session, ScarabList * list)
{
	int             l;
	int             i;
	int             r;

	if (ldo_write_typecode(session, LIST) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);

	if (ldo_writeber(session, list->size) != 0)
		return scarab_session_seterr(session, LDO_ERR_IO);

	l = list->size;
	for (i = 0; i < l; i++)
	{
		r = scarab_write(session, list->values[i]);
		if (r)
			return r;
	}
	return 0;
}

static void
ldo_putref(ScarabSession * session, int reference, ScarabDatum * obj)
{
	LDO_Session    *s;
	ScarabDatum    *ref;

	s = (LDO_Session *) session->encoder;
	ref = scarab_new_integer(reference);

	scarab_dict_put(s->refs, ref, obj);
	scarab_free_datum(ref);
	scarab_free_datum(obj);
}

static ScarabDatum *
ldo_getref(ScarabSession * session, int reference)
{
	LDO_Session    *s;
	ScarabDatum    *ref;
	ScarabDatum    *obj;

	s = (LDO_Session *) session->encoder;
	ref = scarab_new_integer(reference);
	obj = scarab_dict_get(s->refs, ref);

	scarab_free_datum(ref);

	return obj;
}

static ScarabDatum *
ldo_read(ScarabSession * session)
{
	LDO_Session    *s;
	ScarabDatum    *attrib;
	ScarabDatum    *value;
	long long             refid;
	long long             sign;
	long long             esign;
	long long             len;
	unsigned char   lastByte;

	s = (LDO_Session *) session->encoder;
	attrib = NULL;
	refid = -1;
	sign = 1;
	esign = 1;

    // PAJ really wish the guys who wrote this left some comments in here
    // this read calls the stream head read which calls stream_read in
    // stream_tcpip.c.  This is the place where all the socket comm stuff 
    // happens.  This is where recv is called....and will block
	if (scarab_session_read(session, &lastByte, 1) != 1)
		return NULL;

	if (DEFINE_REFERENCE == lastByte)
	{
		refid = ldo_readber(session);
		if (scarab_session_read(session, &lastByte, 1) != 1)
			return NULL;
	}

	if (ATTRIBUTES == lastByte)
	{
		if (scarab_session_read(session, &lastByte, 1) != 1)
			return NULL;

		if (REFERENCE == lastByte)
		{
			refid = ldo_readber(session);
		} else if (DEFINE_REFERENCE == lastByte)
		{
			refid = ldo_readber(session);
			ldo_putref(session, refid, ldo_read_dict(session));
		}
		attrib = ldo_getref(session, refid);
		if (scarab_session_read(session, &lastByte, 1) != 1)
			return NULL;
	}

	switch (lastByte)
	{
		/*
		 * integer
		 */
	case INTEGER_N:
		sign = -1;
	case INTEGER_P:
		value = scarab_new_integer(ldo_readber(session) * sign);
		break;

		/*
		 * float
		 */
	case FLOAT_NN:
		esign = -1;
	case FLOAT_NP:
		sign = -1;
		value = scarab_new_float(ldo_readfloat(session, sign, esign));
		break;
	case FLOAT_PN:
		esign = -1;
	case FLOAT_PP:
		value = scarab_new_float(ldo_readfloat(session, sign, esign));
		break;
	case FLOAT_INF:
		value = scarab_new_atomic();
		value->type = SCARAB_FLOAT_INF;
		break;
	case FLOAT_OPAQUE:
		//value = scarab_new_atomic();
		//value->type = SCARAB_FLOAT_NAN;
//        fprintf(stderr, "This is f***ing ridiculous!!!!!\n");
//        fprintf(stderr, "Reading a god damn float nan\n");
		len = ldo_readber(session);
		value = scarab_new_atomic();
		value->type = SCARAB_FLOAT_OPAQUE; //HAXXOR WARNING
		value->data.opaque.size = len;
		value->data.opaque.data = (unsigned char*)scarab_mem_malloc(
													sizeof(char) * len);
        
		scarab_session_read(session, value->data.opaque.data, len);
		break;

		/*
		 * opaque thingies
		 */
	case OPAQUE:
		len = ldo_readber(session);
		value = scarab_new_atomic();
		value->type = SCARAB_OPAQUE;
		value->data.opaque.size = len;
		value->data.opaque.data = (unsigned char*)scarab_mem_malloc(
													sizeof(char) * len);

		scarab_session_read(session, value->data.opaque.data, len);
		break;

		/*
		 * compound
		 */
	case DICTIONARY:
		value = ldo_read_dict(session);
		break;
	case LIST:
		value = ldo_read_list(session);
		break;
	case REFERENCE:
		value = ldo_getref(session, ldo_readber(session));
		break;
	case 0x0:  // DDC August 10, 2005, changed from NULL
		value = scarab_new_atomic();
		value->type = SCARAB_NULL;
		break;
	default:
		return NULL;
	}

	/*
	 * save the reference if there is one
	 */
	if (refid != -1)
		ldo_putref(session, refid, value);

	/*
	 * if attributes were used, add them to the value
	 */
	if (attrib != NULL)
		value->attributes = attrib;

	return value;
}

static ScarabDatum *
ldo_read_dict(ScarabSession * session)
{
	int             len;
	ScarabDatum    *dict;

	len = ldo_readber(session);
	dict = scarab_dict_new(len, scarab_dict_times2);

	for (; len > 0; len--)
	{
		ScarabDatum *key  = ldo_read(session);
		ScarabDatum *value = ldo_read(session);
		scarab_dict_put(dict,
						key,
						value);
		scarab_free_datum(key);
		scarab_free_datum(value);
	}
	return dict;
}

static ScarabDatum *
ldo_read_list(ScarabSession * session)
{
	int             i;
	int             len;
	ScarabDatum    *list;
	ScarabDatum	   *list_element;

	len = ldo_readber(session);
	list = scarab_list_new(len);

	for (i = 0; i < len; i++)
	{
		list_element = ldo_read(session);
		scarab_list_put(list, i, list_element);
		scarab_free_datum(list_element);
	}
	return list;
}

static char    *error_text[] = {
	"No Error",
	"LDO Header Not Recognized",
	"C Language 'double' Datatype Not Supported", "Stream I/O Error",
};

static char    *
ldo_strerr(int code)
{
	return error_text[code];
}


ScarabEncoderEngine scarab_ldo_encoder = {
	SCARAB_ENCODER_HEADER(LDO_ERR_MAX),

	/*
	 * session control
	 */
	ldo_init,
	ldo_session_accept,
	ldo_session_connect,
	ldo_session_close,

	/*
	 * writers
	 */
	ldo_write_null,
	ldo_write_integer,
	ldo_write_float,
	ldo_write_float_inf,
	ldo_write_float_nan,
	ldo_write_opaque,
	ldo_write_list,
	ldo_write_dict,

	/*
	 * readers
	 */
	ldo_read,
	ldo_seek,
	ldo_tell,

	/*
	 * error string handling
	 */
	ldo_strerr,

	/*
	 * module info
	 */
	"ldobinary",
	"Shawn Pearce",
	"LDO (Lightweight Distributed Objects - Binary Protocol)"
};

