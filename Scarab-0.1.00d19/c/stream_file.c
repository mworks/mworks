#include <scarab.h>
#include <stdio.h>

/*
 * Accepts URIs which are also file paths.
 */

FWD(struct, filestr);
DEF(struct, filestr)
{
	ScarabStream    header;
	FILE			*fd;
};

#define	_filestr(x)	((filestr*)(x))

#ifndef min
#define	min(x,y)	((x) < (y) ? (x) : (y))
#endif

#define	STREAM_ERR_NONE			0
#define	STREAM_ERR_WRITE		1
#define	STREAM_ERR_READ			2
#define	STREAM_ERR_MAX			100

static int      lib_init(void);
static ScarabStream *filestr_bind(ScarabSession * session, const char *uri);
static ScarabStream *filestr_accept(ScarabSession * session, 
									const char *uri, ScarabStream *);
static ScarabStream *filestr_connect(ScarabSession * session, const char *uri);
static ScarabStream *filestr_connect_readonly(ScarabSession * session, const char *uri);
static int      filestr_close(ScarabStream * stream);
static int      filestr_write(ScarabStream * stream, const void *buffer, int size);
static int      filestr_send(ScarabStream * stream);
static int      filestr_flush(ScarabStream * stream);
static int      filestr_read(ScarabStream * stream, void *buffer, int size);
static int		filestr_seek(ScarabStream *stream, long int offset, int origin);
static long int	filestr_tell(ScarabStream *stream);


static int
lib_init(void)
{
	return 0;
}

static ScarabStream *
filestr_bind(ScarabSession * session, const char *uri)
{
	return NULL;
}

static ScarabStream *
filestr_accept(ScarabSession * session, const char *uri, ScarabStream * binding)
{
	return NULL;
}

static ScarabStream *
filestr_connect(ScarabSession * session, const char *uri)
{
	filestr         *ts;
	int				skip;

	if ('/' == uri[0] && '/' == uri[1])
		skip = 2;
	else
		skip = 0;

	ts = (filestr*)scarab_mem_malloc(sizeof(filestr));
	if (!ts)
		return NULL;

	ts->fd = fopen(uri + skip, "r+");
	if (!ts->fd)
		return NULL;
		
	//setvbuf(ts->fd, NULL, _IOFBF, 4096*1000);

	return &ts->header;
}


static ScarabStream *
filestr_connect_readonly(ScarabSession * session, const char *uri)
{
	filestr         *ts;
	int				skip;

	if ('/' == uri[0] && '/' == uri[1])
		skip = 2;
	else
		skip = 0;

	ts = (filestr*)scarab_mem_malloc(sizeof(filestr));
	if (!ts)
		return NULL;

	ts->fd = fopen(uri + skip, "r");
	if (!ts->fd)
		return NULL;
		
	//setvbuf(ts->fd, NULL, _IOFBF, 4096*1000);

	return &ts->header;
}


static int
filestr_close(ScarabStream * stream)
{
	filestr         *ts = _filestr(stream);

	fclose(ts->fd);

	return 0;
}

static int
filestr_write(ScarabStream * stream, const void *buffer, int size)
{
	filestr         *ts = _filestr(stream);

	fwrite(buffer, 1, size, ts->fd);
	return size;
}

static int
filestr_send(ScarabStream * stream)
{
	return 0;
}

static int
filestr_flush(ScarabStream * stream)
{
	filestr         *ts = _filestr(stream);

	fflush(ts->fd);
	return 0;
}

static int
filestr_read(ScarabStream * stream, void *buffer, int size)
{
	filestr         *ts = _filestr(stream);
	
	// DDC 8/26/2005
	//** Was
	//fread(buffer, 1, size, ts->fd);
	//return size;
	
	//** Now is
	return fread(buffer, 1, size, ts->fd);
}

static int 
filestr_seek(ScarabStream *stream, long int offset, int origin)
{
	filestr *ts= _filestr(stream);
	return fseek(ts->fd, offset, origin);
}

static long int	
filestr_tell(ScarabStream *stream) {
	filestr *ts= _filestr(stream);
	return ftell(ts->fd);
}


static char    *error_text[] = {
	"No Error",
	"I/O Write Error",
	"I/O Read Error",
	NULL,
};

static char    *
filestr_errstr(int code)
{
	return error_text[code];
}

ScarabStreamEngine scarab_stream_file = {
	SCARAB_STREAM_HEADER(STREAM_ERR_MAX),

	lib_init,
	filestr_bind,
	filestr_accept,
	filestr_connect,
	filestr_close,

	filestr_write,
	filestr_send,
	filestr_flush,

	filestr_read,
	filestr_seek,
	filestr_tell,
	
	filestr_errstr,

	"file",
	"Shawn Pearce <spearce@spearce.org>",
	"File IO Driver"
};


ScarabStreamEngine scarab_stream_file_readonly = {
	SCARAB_STREAM_HEADER(STREAM_ERR_MAX),

	lib_init,
	filestr_bind,
	filestr_accept,
	filestr_connect_readonly,
	filestr_close,

	filestr_write,
	filestr_send,
	filestr_flush,

	filestr_read,
	filestr_seek,
	filestr_tell,

	filestr_errstr,

	"file_readonly",
	"Shawn Pearce <spearce@spearce.org>",
	"File IO Driver"
};
