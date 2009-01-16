#ifndef SCARAB_H
#define	SCARAB_H

#ifndef BUILD_SCARAB_FRAMEWORK
#ifdef __cplusplus
extern "C" {
#endif
#endif //BUILD_SCARAB_FRAMEWORK

#include <stdlib.h>
#include <Scarab/scarab_datatypes.h>
#include <Scarab/scarab_os.h>
#include <Scarab/scarab_utilities.h>
//#include <Scarab/encoding_ldo.h>

#define	SCARAB_API_VERSION				200007250
#define	SCARAB_ENCODER_HEADER(nerr)		SCARAB_API_VERSION,0,nerr,0
#define	SCARAB_STREAM_HEADER(nerr)		SCARAB_API_VERSION,0,nerr,0

#define FORCE_BUFFERING_OFF 0
#define FORCE_BUFFERING_ON 0

extern int SCARAB_SELECT_TIMEDOUT;
extern int READ_FUBAR_FLAG;


FWD(struct, ScarabEncoderEngine);
FWD(struct, ScarabStreamEngine);
FWD(struct, ScarabStream);
FWD(struct, ScarabSession);

// e.g. ldobinary
// An "EncoderEngine" deals with how data is encoded when it is sent out
// onto the stream
DEF(struct, ScarabEncoderEngine)
{
	/*
	 * integrity check ;-)
	 */
	int             api_version;
	int             err_code_base;
	int             nerr_code;
	int             _pad1;

	/*
	 * setup, cleanup
	 */
	int             (*lib_init) (void);
	int             (*session_accept) (ScarabSession * s);
	int             (*session_connect) (ScarabSession * s);
	int             (*session_close) (ScarabSession * s);

	/*
	 * writers
	 */
	int             (*write_null) (ScarabSession * s);
	int             (*write_integer) (ScarabSession * s, long long value);
	int             (*write_float) (ScarabSession * s, double value);
	int             (*write_float_inf) (ScarabSession * s);
	int             (*write_float_nan) (ScarabSession * s);
	int             (*write_opaque) (ScarabSession * s, const char *value, int len);
	int             (*write_list) (ScarabSession * s, ScarabList * list);
	int             (*write_dict) (ScarabSession * s, ScarabDict * dict);

	/*
	 * readers
	 */
	ScarabDatum    *(*read) (ScarabSession * s);

	int				(*seek) (ScarabSession *s, long int offset, int origin);
	long int		(*tell) (ScarabSession *s);
	/*
	 * module error strings
	 */
	char           *(*strerr) (int code);

	/*
	 * module info
	 */
	char            uri_scheme[64];
	char            engine_author[64];
	char            engine_name[64];
};


// A "StreamEngine" defines how you connect, talk, close etc. over a given sort
// of stream 
// e.g. tcp, file 
DEF(struct, ScarabStreamEngine)
{
	int             api_version;
	int             err_code_base;
	int             nerr_code;
	int             _pad1;

	/*
	 * setup, cleanup
	 */
	int             (*lib_init) (void);
	ScarabStream   *(*stream_listen) (ScarabSession * s, const char *uri);
	ScarabStream   *(*stream_accept) (ScarabSession * session, const char *uri,
									  ScarabStream * binding);
	ScarabStream   *(*stream_connect) (ScarabSession * session, const char *uri);
	int             (*stream_close) (ScarabStream * self);

	/*
	 * output
	 */
	int             (*write) (ScarabStream * self, const void *data, int len);
	int				(*send) (ScarabStream * self);
	int             (*flush) (ScarabStream * self);

	/*
	 * input
	 */
	int             (*read) (ScarabStream * self, void *data, int len);
	
	/*
	 * navigation
	 */
	int				(*seek)(ScarabStream *self, long int offset, int origin);
	long int		(*tell)(ScarabStream *);

	/*
	 * module error strings
	 */
	char           *(*strerr) (int code);

	/*
	 * module info
	 */
	char            uri_scheme[64];
	char            engine_author[64];
	char            engine_name[64];
    
    /*
     * Inspection
     */
    int             (*local_port) (ScarabStream * self);
    char            *(*local_address) (ScarabStream * self);
    int             (*foreign_port) (ScarabStream * self);
    char            *(*foreign_address) (ScarabStream * self);
};


DEF(struct, ScarabStream)
{
	ScarabStream   *next;		// it's a linked list (but why?)
	ScarabSession  *session;	// reciprocal link?
	ScarabStreamEngine *engine; // how to do stuff on this kind of stream
};

#define	scarab_errcode(self,code)	( (self)->err_code_base + (code) )

#define	scarab_stream_close(stream)				((stream)->engine->stream_close((stream)))
#define	scarab_stream_write(stream, data, len)	((stream)->engine->write((stream),(data),(len)))
#define	scarab_stream_flush(stream)				((stream)->engine->flush((stream)))
#define	scarab_stream_send(stream)				((stream)->engine->send((stream)))
#define	scarab_stream_read(stream, data, len)	((stream)->engine->read((stream),(data),(len)))
#define scarab_stream_seek(stream, offset, origin)	((stream)->engine->seek((stream), (offset), (origin)))
#define scarab_stream_tell(stream)				((stream)->engine->tell((stream)))

#define SEND_BUFFER_SIZE	100000000
#define READ_BUFFER_SIZE	10000000

DEF(struct, ScarabSession)
{

	ScarabEncoderEngine *encoder_engine;   // this says which functions to call
										   // to encode data (these, in turn,
										   // ultimately call functions in the
										   // the StreamEngine to actually write
										   // bytes, manage connections, etc.
	void           *encoder;				// what the fuck is this?
	char           *url;
	ScarabStream   *stream_head;			// linked list of streams to interact with
											// why a list?
											
	int             errcode;				// what could possibly go wrong?
	int             os_errno;
	
	/*
	 * operating parameters
	 */
	int				force_buffering;
	int				send_recursion_counter;
	
	/*
	 * Buffers
	 */
	 
	int				send_buffer_position;// = 0;
	unsigned char	send_buffer[SEND_BUFFER_SIZE];
	

	unsigned char	read_buffer[READ_BUFFER_SIZE];
	int				read_buffer_fill_position;// = 0;
	int				read_buffer_read_position;// = 0;


};

/*
 * Initilize the Scarab library.
 */
extern int      scarab_init();

/*
 * Create a new scarab session.
 */
extern ScarabSession *scarab_session_connect(const char *url);

/*
 * Listen for new new scarab sessions as a server.
 */
extern ScarabSession *scarab_session_listen(const char *url);

/*
 * Accept a new new scarab sessions as a server.
 * Uses the listen session already made by the listener to build the
 * connection.
 */
extern ScarabSession *scarab_session_accept(ScarabSession * binding);

/*
 * Assign the encoder to a session.
 */
extern void     scarab_session_set_encoder(ScarabSession * session,
										   ScarabEncoderEngine * engine,
										   void *engine_data);

/*
 * Close a session.
 */
extern int      scarab_session_close(ScarabSession * session);

/*
 * Set the session error code data.
 */
extern int      scarab_session_seterr(ScarabSession * session, int code);

/*
 * Get the session error code.
 */
extern int      scarab_session_geterr(ScarabSession * session);

/*
 * Kind of a hack but we need some way to tell if the error code returned
 *  from scarab functions was because of a timeout on a select call.
 * returns true if it was a timeout no otherwise.
 * errorCode should be the error code returned from a scarab session
 */
extern int     scarab_did_select_timeout(int errorCode);

/*
 * Get the session os error code.
 */
extern int      scarab_session_getoserr(ScarabSession * session);

/*
 * Get the module name that handles this error code.
 */
extern const char *scarab_moderror(int errcode);

/*
 * Get the string version of an error code.
 */
extern const char *scarab_strerror(int errcode);


/*
 * Create a new stream.  This stream will be pushed onto the
 * Scarab stream stack under the stream given.  If the stream
 * given is NULL, it will be placed at the TOP of the stream
 * stack.  This is a binding only (ie server socket).
 */
extern int      scarab_session_push_listen(ScarabSession * session,
										 ScarabStream * stream, 
										 const char *uri);

/*
 * Push a new stream onto the stack by accepting a stream from
 * the binding stream and creating a new ScarabStream.
 */
extern int      scarab_session_push_accept(ScarabSession * session,
										   ScarabStream * stream,
										   char *uri, ScarabStream * binding);

/*
 * Create a new stream.  This stream will be pushed onto the
 * Scarab stream stack under the stream given.  If the stream
 * given is NULL, it will be placed at the TOP of the stream
 * stack.
 */
extern int      scarab_session_push_connect(ScarabSession * session,
											ScarabStream * stream, 
											const char *uri);

/*
 * Send data to the session.
 */
extern int      scarab_session_read(ScarabSession * session, void *buffer,

									int len);

extern int		scarab_session_seek(ScarabSession *session, long int offset, int origin);
extern long int	scarab_session_tell(ScarabSession *session);
extern int      scarab_session_write(ScarabSession * session, const void *buffer,

									 int len);
extern int      scarab_session_send(ScarabSession * session);
extern int      scarab_session_flush(ScarabSession * session);

/*
 * Mabbit Inspection Functions
 */
extern int      scarab_session_local_port(ScarabSession * session);
extern char *   scarab_session_local_address(ScarabSession * session);
extern int      scarab_session_foreign_port(ScarabSession * session);
extern char *   scarab_session_foreign_address(ScarabSession * session);
extern int     scarab_session_read_should_die(ScarabSession * session);

/*
 * Output to a session.
 * Always returns 0 on success,
 *				< 0 on failure.
 */
extern int      scarab_write(ScarabSession * session, ScarabDatum * value);
extern int      scarab_write_integer(ScarabSession * session, int value);
extern int      scarab_write_float(ScarabSession * session, double value);
extern int      scarab_write_string(ScarabSession * session, const char *value);
extern int      scarab_write_opaque(ScarabSession * session, const char *value,
									int size);
extern int      scarab_write_list(ScarabSession * session, ScarabList * list);
extern int      scarab_write_dict(ScarabSession * session, ScarabDict * dict);

/*
 * Read from a session.
 * Always returns the datum on success,
 *					NULL on failure.
 */
extern ScarabDatum *scarab_read(ScarabSession * session);
extern ScarabDatum *scarab_read_integer(ScarabSession * session);
extern ScarabDatum *scarab_read_float(ScarabSession * session);
extern ScarabDatum *scarab_read_opaque(ScarabSession * session);
extern ScarabDatum *scarab_read_list(ScarabSession * session);
extern ScarabDatum *scarab_read_dict(ScarabSession * session);


extern int scarab_seek(ScarabSession *session, long int offset, int origin);
extern long int scarab_tell(ScarabSession *session);
/*
 * Create a new molecular (has its own memory context).
 * Initially set to null value.
 */
extern ScarabDatum *scarab_new_molecular();

/*
 * Create a new atomic (uses callers memory context).
 * Initially set to null value.
 */
extern ScarabDatum *scarab_new_atomic();

/*
 * Create copies of a ScarabDatum
 */
extern ScarabDatum * scarab_copy_datum(ScarabDatum *datum);
//extern ScarabDatum * scarab_deep_copy_datum(ScarabDatum *datum);

/*
 * Release a ScarabDatum and its memory immediately.
 */
void
scarab_free_datum(ScarabDatum *d);


ScarabDatum *scarab_init_lock(ScarabDatum *datum);
ScarabDatum *scarab_lock_datum(ScarabDatum *datum);
ScarabDatum *scarab_unlock_datum(ScarabDatum *datum);
ScarabDatum *scarab_destroy_lock(ScarabDatum *datum);



/*
 * Constructs new ScarabDatums for standard C/C++ types.
 */
extern ScarabDatum *scarab_new_integer(long long value);
extern ScarabDatum *scarab_new_float(double value);
extern ScarabDatum *scarab_new_string(const char *value);
extern ScarabDatum *scarab_new_opaque(const char *data, int size);

/*
 * Returns hash value 0 <= h < modulo given the datum.
 */
extern int      scarab_hash(ScarabDatum * datum, int modulo, int atry);

/*
 * Return 0 if datum1!=datum2, 
 *        1 if datum1==datum2.
 */
extern int      scarab_equals(ScarabDatum * dataum1, ScarabDatum * datum2);

void scarab_force_buffering(ScarabSession *s, int value);

#ifndef BUILD_SCARAB_FRAMEWORK
#ifdef __cplusplus
}
#endif
#endif //BUILD_SCARAB_FRAMEWORK
		
		

#endif
