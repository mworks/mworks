/**
 * session.c
 *
 * Description: I believe this is some sort of adapter for a stream engine.
 *
 * History:
 * ?? on ??/??/?? - Created by some  bastards in Iowa who don't like comments
 * Paul Jankunas on 8/10/05 - Added a non-blocking tcpip stream engine.(JK)
 * Paul Jankunas on 08/10/05 - Added a strean that blocks in select not read.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include <scarab.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

/*
 * Import Encoders
 */
extern ScarabEncoderEngine scarab_ldo_encoder;

/*
 * Import Streams
 */
extern ScarabStreamEngine scarab_stream_tcpip;
extern ScarabStreamEngine scarab_stream_file;
extern ScarabStreamEngine scarab_stream_file_readonly;
//extern ScarabStreamEngine scarab_stream_tcpipnb; // non-blocking
extern ScarabStreamEngine scarab_stream_tcpip_select;
extern ScarabStreamEngine scarab_buffered_stream_tcpip;
/*
 * Define Encoder and Stream list.
 */

static ScarabEncoderEngine *g_encoder_engines[64] = {
	&scarab_ldo_encoder,
	NULL,
};

static ScarabStreamEngine *g_stream_engines[64] = {
	&scarab_stream_tcpip,
	&scarab_stream_file,
	&scarab_stream_file_readonly,
    &scarab_stream_tcpip_select,
	&scarab_buffered_stream_tcpip,
	NULL,
};

/*
 * Only for isnan() and isinf()
 */
#include <math.h>
#include <float.h>

#ifdef _WIN32
#define	isnan(x)			( _isnan(x) )
#define	isinf(x)			( !(_finite(x)) )
#define	strcasecmp(x,y)		( _stricmp((x),(y)) )
#endif

/*
 * Error codes
 */
#define	SCARAB_ERR_BAD_ENCODER_TYPE	1
#define	SCARAB_ERR_BAD_STREAM_TYPE	2
#define	SCARAB_ERR_API_VERSION		3

static int      g_errcnt = 1000;
static int      g_next_errcode = 1000;

static const char *g_error_strings[] = {
	"No Error",
	"Invalid Encoder Type",
	"Invalid Stream Type",
	"Driver Version Not Supported",
	"Unknown Error",
	"Unknown Error",
	NULL,
};

static ScarabEncoderEngine *find_encoder(const char *uri);
static ScarabStreamEngine *find_stream(const char *uri);

static ScarabEncoderEngine *
find_encoder(const char *uri)
{
	char            scheme[64];
	char           *c;
	int             len;
	ScarabEncoderEngine **engine;

	/*
	 * Make a string to match against.
	 */
	c = (char *)uri;
	len = 0;
	while (*c != ':' && *c != 0 && len < sizeof(scheme))
	{
		c++;
		len++;
	}
	if (*c != ':')
		return NULL;

	memcpy(scheme, uri, len);
	scheme[len] = 0;

	/*
	 * Look for it in the encoder list.
	 */
	for (engine = g_encoder_engines; *engine; engine++)
	{
		if (strcasecmp((*engine)->uri_scheme, scheme) == 0)
			return *engine;
	}

	return NULL;
}

static ScarabStreamEngine *
find_stream(const char *uri)
{
	char            scheme[64];
	char           *c;
	int             len;
	ScarabStreamEngine **engine;

	/*
	 * Make a string to match against.
	 */
	c = (char *)uri;
	len = 0;
	while (*c != ':' && *c != 0 && len < sizeof(scheme))
	{
		c++;
		len++;
	}
	if (*c != ':')
		return NULL;

	memcpy(scheme, uri, len);
	scheme[len] = 0;

	/*
	 * Look for it in the encoder list.
	 */

	for (engine = g_stream_engines; *engine; engine++)
	{
		if (strcasecmp((*engine)->uri_scheme, scheme) == 0)
			return *engine;
	}

	return NULL;
}

int
scarab_init()
{
	int             r;
	ScarabEncoderEngine **encoder;
	ScarabStreamEngine **stream;

	scarab_os_init();
	scarab_mem_init();

	r = 0;
    
//    <EDIT>
    // DDC added PAJ moved
    // SIGPIPE is sent when a host connection is terminated and
    // data gets written to the socket.  The SIGPIPE signal will be
    // ignored and EPIPE will be generated.  This was removed from
    // inside the stream_write function to here because it is a process
    // wide thing to ignore the SIGPIPE message
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        //handle error
        fprintf(stderr, "Failed to Ignore SIGPIPE signal in scarab_init()");
        fflush(stderr);
    }
//  <END_EDIT>
	for (encoder = g_encoder_engines; *encoder; encoder++)
	{
		if (SCARAB_ERR_API_VERSION == (*encoder)->api_version)
			return SCARAB_ERR_API_VERSION;

		(*encoder)->err_code_base = g_next_errcode;
		g_next_errcode += (*encoder)->nerr_code;

		r = (*encoder)->lib_init();
		if (r)
			return r;
	}

	for (stream = g_stream_engines; *stream; stream++)
	{
		if (SCARAB_ERR_API_VERSION == (*stream)->api_version)
			return SCARAB_ERR_API_VERSION;

		(*stream)->err_code_base = g_next_errcode;
		g_next_errcode += (*stream)->nerr_code;

		r = (*stream)->lib_init();
		if (r)
			return r;
	}

	return r;
}

ScarabSession  *
scarab_session_connect(const char *url)
{
	ScarabSession  *session;
	ScarabEncoderEngine *enc;

	session = (ScarabSession*)scarab_mem_malloc(sizeof(ScarabSession));
	if (!session)
		return NULL;

	session->encoder_engine = NULL;
	session->encoder = NULL;
	session->stream_head = NULL;
	session->errcode = 0;
	session->os_errno = 0;
	session->force_buffering = FORCE_BUFFERING_OFF;
	session->send_recursion_counter = 0;
	session->send_buffer_position = 0;
	session->read_buffer_fill_position = 0;
	session->read_buffer_read_position = 0;
	
	session->url = (char *)scarab_mem_malloc(strlen(url)+1); // DDC edit
	strncpy(session->url, url, strlen(url)+1);  // Whoever wrote this is a fucker 

	enc = find_encoder(url);
	if (!enc)
	{
		scarab_session_seterr(session, SCARAB_ERR_BAD_ENCODER_TYPE);
		return session;
	}

	url += strlen(enc->uri_scheme) + 1;

	if (scarab_session_push_connect(session, NULL, url) != 0)
	{
		return session;
	}

	session->encoder_engine = enc;
	if (enc->session_connect(session) != 0)
	{
		scarab_stream_close(session->stream_head);
		return session;
	}

	return session;
}

ScarabSession  *
scarab_session_listen(const char *url)
{
	ScarabSession  *session;
	ScarabEncoderEngine *enc;

	session = (ScarabSession*)scarab_mem_malloc(sizeof(ScarabSession));
	if (!session)
		return NULL;

	session->encoder_engine = NULL;
	session->encoder = NULL;
	session->stream_head = NULL;
	session->errcode = 0;
	session->os_errno = 0;
	session->force_buffering = FORCE_BUFFERING_OFF;
	session->send_recursion_counter = 0;
	session->send_buffer_position = 0;
	session->read_buffer_fill_position = 0;
	session->read_buffer_read_position = 0;


	session->url = (char *)scarab_mem_malloc(strlen(url)+1); // DDC edit
																	// (+1)
	strncpy(session->url, url, strlen(url)+1);

	enc = find_encoder(url);
	if (!enc)
	{
		scarab_session_seterr(session, SCARAB_ERR_BAD_ENCODER_TYPE);
		return session;
	}

	url += strlen(enc->uri_scheme) + 1;
	if (scarab_session_push_listen(session, NULL, url) != 0)
	{
		return session;
	}

	return session;
}

ScarabSession  *
scarab_session_accept(ScarabSession * binding)
{
	ScarabSession  *session;
	ScarabEncoderEngine *enc;
	char           *url;

	session = (ScarabSession*)scarab_mem_malloc(sizeof(ScarabSession));
	if (!session)
		return NULL;

	session->encoder_engine = NULL;
	session->encoder = NULL;
	session->stream_head = NULL;
	session->errcode = 0;
	session->os_errno = 0;
	session->force_buffering = FORCE_BUFFERING_OFF;
	session->send_recursion_counter = 0;
	session->send_buffer_position = 0;
	session->read_buffer_fill_position = 0;
	session->read_buffer_read_position = 0;


	url = binding->url;
	session->url = (char *)scarab_mem_malloc(strlen(url)+1); // DDC edit
	strncpy(session->url, url, strlen(url)+1);	// FUCKERS!!!!

	enc = find_encoder(url);
	if (!enc)
	{
		scarab_session_seterr(session, SCARAB_ERR_BAD_ENCODER_TYPE);
		return session;
	}

	url += strlen(enc->uri_scheme) + 1;
	if (scarab_session_push_accept(session, NULL, url,
								   binding->stream_head) != 0)
	{
		return session;
	}

	session->encoder_engine = enc;
	if (enc->session_accept(session) != 0)
	{
		scarab_stream_close(session->stream_head);
		return session;
	}

	return session;
}

void
scarab_session_set_encoder(ScarabSession * session,
						   ScarabEncoderEngine * engine, void *engine_data)
{
	session->encoder_engine = engine;
	session->encoder = engine_data;
}

int
scarab_session_push_listen(ScarabSession * session,
						   ScarabStream * stream, 
						   const char *uri)
{
	ScarabStreamEngine *e;
	ScarabStream   *s;

	e = find_stream(uri);
	if (!e)
	{
		scarab_session_seterr(session, SCARAB_ERR_BAD_STREAM_TYPE);
		return -SCARAB_ERR_BAD_STREAM_TYPE;
	}

	s = e->stream_listen(session, uri + strlen(e->uri_scheme) + 1);
	if (!s)
	{
		return -session->errcode;
	}

	s->engine = e;
	s->session = session;

	if (stream)
	{
		while (stream->next)
		{
			stream = stream->next;
		}
		s->next = NULL;
		stream->next = s;
	} else
	{
		s->next = session->stream_head;
		session->stream_head = s;
	}

	return 0;
}

int
scarab_session_push_accept(ScarabSession * session,
						   ScarabStream * stream,
						   char *uri, ScarabStream * binding)
{
	ScarabStreamEngine *e;
	ScarabStream   *s;

	e = find_stream(uri);
	if (!e)
	{
		scarab_session_seterr(session, SCARAB_ERR_BAD_STREAM_TYPE);
		return -SCARAB_ERR_BAD_STREAM_TYPE;
	}

	s = e->stream_accept(session, uri + strlen(e->uri_scheme) + 1, binding);
	if (!s)
	{
		return -session->errcode;
	}

	s->engine = e;
	s->session = session;

	if (stream)
	{
		while (stream->next)
		{
			stream = stream->next;
		}
		s->next = NULL;
		stream->next = s;
	} else
	{
		s->next = session->stream_head;
		session->stream_head = s;
	}

	return 0;
}

int
scarab_session_push_connect(ScarabSession * session,
							ScarabStream * stream, 
							const char *uri)
{
	ScarabStreamEngine *e;
	ScarabStream   *s;

	e = find_stream(uri);
	if (!e)
	{
		scarab_session_seterr(session, SCARAB_ERR_BAD_STREAM_TYPE);
		return -SCARAB_ERR_BAD_STREAM_TYPE;
	}

	s = e->stream_connect(session, uri + strlen(e->uri_scheme) + 1);
	if (!s)
	{
		return -session->errcode;
	}

	s->engine = e;
	s->session = session;

	if (stream)
	{
		while (stream->next)
		{
			stream = stream->next;
		}
		s->next = NULL;
		stream->next = s;
	} else
	{
		s->next = session->stream_head;
		session->stream_head = s;
	}

	return 0;
}

int
scarab_session_close(ScarabSession * session)
{
    //<EDIT>
    // Getting a bad access here when trying to close a listening socket.
    // This function is only returning 0 so i am going to comment it out.
	//session->encoder_engine->session_close(session);

	/*
	 * Make sure we flush before we close it.
	 */
    // This function is also only returning 0 but I am going to keep it because
    // it wont throw an error.
	//scarab_session_flush(session);
    // keeping this one because this one actually calls shutdown on the socket
	scarab_stream_close(session->stream_head);

	free(session);
    //<END_EDIT>
	return 0;
}

int
scarab_session_seterr(ScarabSession * session, int code)
{
	session->errcode = code;
	session->os_errno = scarab_os_geterr();

	return -code;
}

int
scarab_session_geterr(ScarabSession * session)
{
	return session->errcode;
}

int
scarab_did_select_timeout(int errorCode) {
    return (errorCode == SCARAB_SELECT_TIMEDOUT);
}

int
scarab_session_getoserr(ScarabSession * session)
{
	return session->os_errno;
}

const char     *
scarab_moderror(int errcode)
{
	int             base;
	int             cnt;

	ScarabEncoderEngine **encoder;
	ScarabStreamEngine **stream;

	if (errcode >= 1 && errcode < g_errcnt)
		return "SCARAB";

	for (encoder = g_encoder_engines; *encoder; encoder++)
	{
		base = (*encoder)->err_code_base;
		cnt = (*encoder)->nerr_code;

		if (errcode >= base && errcode < base + cnt)
			return (*encoder)->uri_scheme;
	}

	for (stream = g_stream_engines; *stream; stream++)
	{
		base = (*stream)->err_code_base;
		cnt = (*stream)->nerr_code;

		if (errcode >= base && errcode < base + cnt)
			return (*stream)->uri_scheme;
	}

	return NULL;
}

const char     *
scarab_strerror(int errcode)
{
	int             base;
	int             cnt;

	ScarabEncoderEngine **encoder;
	ScarabStreamEngine **stream;

	if (errcode >= 1 && errcode < g_errcnt)
		return g_error_strings[errcode];

	for (encoder = g_encoder_engines; *encoder; encoder++)
	{
		base = (*encoder)->err_code_base;
		cnt = (*encoder)->nerr_code;

		if (errcode >= base && errcode < base + cnt)
			return (*encoder)->strerr(errcode - base);
	}

	for (stream = g_stream_engines; *stream; stream++)
	{
		base = (*stream)->err_code_base;
		cnt = (*stream)->nerr_code;

		if (errcode >= base && errcode < base + cnt)
			return (*stream)->strerr(errcode - base);
	}

	return NULL;
}

int
scarab_session_read(ScarabSession * session, void *data, int len)
{
	return scarab_stream_read(session->stream_head, data, len);
}

int scarab_session_seek(ScarabSession *session, long int offset, int origin)
{
	return scarab_stream_seek(session->stream_head, offset, origin);
}

long int scarab_session_tell(ScarabSession *session) 
{
	return scarab_stream_tell(session->stream_head);
}

int
scarab_session_write(ScarabSession * session, const void *data, int len)
{
	return scarab_stream_write(session->stream_head, data, len);
}

int
scarab_session_send(ScarabSession * session)
{
	return scarab_stream_send(session->stream_head);
}

int
scarab_session_flush(ScarabSession * session)
{
	return scarab_stream_flush(session->stream_head);
}

/**
 * More Mabbit Functions
 */
int scarab_session_local_port(ScarabSession * session) {
    if(session == NULL) { return (int)""; }
    return (session->stream_head)->engine->local_port(session->stream_head);
}

char * scarab_session_local_address(ScarabSession * session) {
    if(session == NULL) { return ""; }
    return (session->stream_head)->engine->local_address(session->stream_head);
}

int scarab_session_foreign_port(ScarabSession * session) {
    if(session == NULL) { return (int)""; }
    return (session->stream_head)->engine->foreign_port(session->stream_head);
}

char * scarab_session_foreign_address(ScarabSession * session) {
    if(session == NULL) { return ""; }
    return (session->stream_head)->engine->foreign_address(session->stream_head);
}

int scarab_session_read_should_die(ScarabSession * session) {
    if(READ_FUBAR_FLAG) {
        READ_FUBAR_FLAG = 0;
        return 1;
    } else {
        return 0;
    }
}

int
scarab_write(ScarabSession * session, ScarabDatum * value)
{
	ScarabEncoderEngine *enc;

	enc = session->encoder_engine;

	session->send_recursion_counter++;

	int returnval;


	if (!value)
	{
		session->send_recursion_counter--;
		return enc->write_null(session);
	}

	switch (value->type)
	{
	case SCARAB_NULL:
		returnval = enc->write_null(session);
		break;
	case SCARAB_INTEGER:
		returnval = enc->write_integer(session, value->data.integer);
		break;
	case SCARAB_FLOAT:
	case SCARAB_FLOAT_OPAQUE:
		returnval = enc->write_float(session, value->data.floatp);
		break;
	case SCARAB_FLOAT_INF:
		returnval = enc->write_float_inf(session);
		break;
	case SCARAB_FLOAT_NAN:
		returnval = enc->write_float_nan(session);
		break;
	case SCARAB_DICT:
		returnval = enc->write_dict(session, value->data.dict);
		break;
	case SCARAB_LIST:
		returnval = enc->write_list(session, value->data.list);
		break;
	case SCARAB_OPAQUE:
		returnval = enc->write_opaque(session, (const char *)value->data.opaque.data,
								 value->data.opaque.size);
		break;
	default:
		returnval = -1;
		break;
	}
	
	session->send_recursion_counter--;
	
	if(session->send_recursion_counter <= 0){
		scarab_session_send(session);
	}
	
	return returnval;
}

int
scarab_write_integer(ScarabSession * session, int value)
{
    //printf("Called scarab_write_integer");
	return session->encoder_engine->write_integer(session, value);
}

int
scarab_write_float(ScarabSession * session, double value)
{
	if (isnan(value))
		return session->encoder_engine->write_float_nan(session);
	if (isinf(value))
		return session->encoder_engine->write_float_inf(session);
	return session->encoder_engine->write_float(session, value);
}

int
scarab_write_string(ScarabSession * session, const char *value)
{
	int             len;

	for (len = 0; value[len]; len++);
	return session->encoder_engine->write_opaque(session, value, len);
}

int
scarab_write_opaque(ScarabSession * session, const char *value, int size)
{
	return session->encoder_engine->write_opaque(session, value, size);
}

int
scarab_write_list(ScarabSession * session, ScarabList * list)
{
	return session->encoder_engine->write_list(session, list);
}

int
scarab_write_dict(ScarabSession * session, ScarabDict * dict)
{
	return session->encoder_engine->write_dict(session, dict);
}

ScarabDatum    *
scarab_read(ScarabSession * session)
{
	return session->encoder_engine->read(session);
}

int
scarab_seek(ScarabSession *session, long int offset, int origin)
{
	return session->encoder_engine->seek(session, offset, origin);
}

long int
scarab_tell(ScarabSession *session) 
{
	return session->encoder_engine->tell(session);
}

ScarabDatum    *
scarab_read_integer(ScarabSession * session)
{
	return session->encoder_engine->read(session);
}
ScarabDatum    *
scarab_read_float(ScarabSession * session)
{
	return session->encoder_engine->read(session);
}
ScarabDatum    *
scarab_read_opaque(ScarabSession * session)
{
	return session->encoder_engine->read(session);
}
ScarabDatum    *
scarab_read_list(ScarabSession * session)
{
	return session->encoder_engine->read(session);
}
ScarabDatum    *
scarab_read_dict(ScarabSession * session)
{
	return session->encoder_engine->read(session);
}
