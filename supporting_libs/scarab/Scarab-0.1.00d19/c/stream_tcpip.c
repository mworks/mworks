/**
 * History:
 * Paul Jankunas on Thu Nov 4 2004 - Started changing the Scarab code to handle
 *                              errors more gracefully.  All changes will be
 *                              preceded by <EDIT> and where the changes
 *                              stop a </EDIT> will be inserted.  Additional
 *                              may appear as needed.
 * Paul Jankunas on Fri Nov 5 2004 - Added function turnOnSocketOptions().  
 *                              Use this function to set some socket options
 *                              that will hopefully lead to a more stable 
 *                              connection.  The options are hard coded
 *                              for now to just get something to work but that
 *                              might want to get changed later.
 */
 
#include <scarab.h>
#include <errno.h>

#undef USE_TCP_NODELAY
//#define USE_TCP_NODELAY 1

#ifdef _WIN32

# include <windows.h>

static int
close(int sock) {
	return closesocket(sock);
}

#else
/*
 * Linux / BSD / Real OSes 
 */
#include <stdio.h>
#include <string.h>
# include <netdb.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#endif

/*
 * Accepts URIs of the format:
 *
 *		host[:port][/.*]
 *
 */

FWD(struct, tcpstr);
DEF(struct, tcpstr)
{
	ScarabStream    header;
	int             fd;
	unsigned int    packet_size;
};

#define	_tcpstr(x)	((tcpstr*)(x))

#ifndef min
#define	min(x,y)	((x) < (y) ? (x) : (y))
#endif

extern ScarabStreamEngine scarab_stream_tcpip;

#define	STREAM_SELF				&scarab_stream_tcpip

#define	STREAM_ERR_NONE			scarab_errcode(STREAM_SELF,	0)
#define	STREAM_ERR_WRITE		scarab_errcode(STREAM_SELF,	1)
#define	STREAM_ERR_READ			scarab_errcode(STREAM_SELF,	2)
#define	STREAM_ERR_INVALID_HOST	scarab_errcode(STREAM_SELF,	3)
#define	STREAM_ERR_INVALID_SOCK	scarab_errcode(STREAM_SELF,	4)
#define	STREAM_ERR_CONNECT		scarab_errcode(STREAM_SELF,	5)
#define	STREAM_ERR_BIND			scarab_errcode(STREAM_SELF,	6)
#define	STREAM_ERR_LISTEN		scarab_errcode(STREAM_SELF,	7)
#define	STREAM_ERR_WSASTARTUP	scarab_errcode(STREAM_SELF,	8)

#define	STREAM_ERR_MAX			100

static int      lib_init(void);
static ScarabStream *stream_bind(ScarabSession * session, const char *uri);
static ScarabStream *stream_accept(ScarabSession * session, const char *uri,
                                                    ScarabStream * binding);
static ScarabStream *stream_connect(ScarabSession * session, const char *uri);
static int      stream_close(ScarabStream * stream);
static int      stream_write(ScarabStream * stream, const void *buffer, int size);
static int      stream_send(ScarabStream * stream);
static int      stream_flush(ScarabStream * stream);
static int      stream_read(ScarabStream * stream, void *buffer, int size);
static int		stream_seek(ScarabStream *stream, long int offset, int origin);
static long int	stream_tell(ScarabStream *stream);
//<EDIT>
static int      setSocketOptionFlag(int fd, int option);
//int fillSocketStructure(ScarabStream * s, int peer, struct sockaddr_in * sock);
//</EDIT>

static int lib_init(void) {
#ifdef _WIN32
	WORD            wVersionRequested;
	WSADATA         wsaData;
	int             err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -STREAM_ERR_WSASTARTUP;
	}
#endif
	return 0;
}

//<EDIT>
// The option is not tested to see if it is defined.  We leave that test
// for the caller.  NOTE This function only works for socket options that
// are treated as flags.
static int setSocketOptionFlag(int fd, int option) {
    socklen_t length;
    int value = 1;
    length = sizeof(value);
    return setsockopt(fd, SOL_SOCKET, option, &value, length);
}

/**
 * This function handles the setup of a listening socket.  The uri passed
 * to this fucntion is of the form "//hostname:portnum" , no quotes.  The port
 * argument appears to be optional.  This function is not IPv6 compatible.
 */
//</EDIT>
static ScarabStream * stream_bind(ScarabSession * session, const char *uri) {
	tcpstr         *ts; //PAJ a structure with a ScarabStream header a 
                        //PAJ file descriptor and packet size
	struct protoent *proto;
	struct hostent *host;

	struct sockaddr_in end_point;   //PAJ socket adress structure we created

	char           *host_name;
	char           *c;
	int             len;
	short           tcp_port = 0;

	/*
	 * Find the hostname and copy it into its own string.
	 */
	len = 0;

	if ('/' == *uri)
		uri++;
	if ('/' == *uri)
		uri++;

	for (c = (char *)uri; *c != 0 && *c != ':' && *c != '/'; c++) {
		len++;
	}

	host_name = (char*)scarab_mem_malloc(len + 1);
	if (!host_name)
		return NULL;
	memcpy(host_name, uri, len);
	host_name[len] = 0;

	/*
	 * Do we have a port?
	 */
	uri += len;
	if (':' == *uri) {
		tcp_port = atoi(uri + 1);
	}

	/*
	 * Locate TCP on the machine
	 */
	proto = getprotobyname("tcp");
	if (!proto)
		return NULL;

	/*
	 * Pack the end_point object.
	 */
	memset(&end_point, 0, sizeof(end_point));
	end_point.sin_family = AF_INET; //PAJ removed PF_INET;
	end_point.sin_port = htons(tcp_port);

	/*
	 * Turn the hostName string into an IP address _somehow_.
	 */
	host = gethostbyname(host_name);
	if (host) {
		memcpy(&end_point.sin_addr, host->h_addr_list[0], 
                                                 sizeof(end_point.sin_addr));
	} else {
		scarab_session_seterr(session, STREAM_ERR_INVALID_HOST);
        // inet_addr transforms the hostname in number and dots notation
        // into binary data in network byte order. inet_addr is obsolete to
        // inet_aton because inet_addr can return -1 which is a valid address
        // 255.255.255.255.
		end_point.sin_addr.s_addr = inet_addr(host_name);
		if (0 == end_point.sin_addr.s_addr)
            // htonl converts from host byte format to network byte order
			end_point.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	/*
	 * Make the session and the TCP/IP listener socket.
	 */
	ts = (tcpstr*)scarab_mem_malloc(sizeof(tcpstr));
	if (!ts)
		return NULL;

    //<EDIT>
    // the last argument can be zero but we went through the work of
    // getting the protocol number from the system so we might as well use it
    ts->fd = socket(AF_INET, SOCK_STREAM, proto->p_proto);
	// ts->fd = socket(PF_INET, SOCK_STREAM, proto->p_proto);

#if !LINUX
    if(setSocketOptionFlag(ts->fd, SO_REUSEPORT) < 0) {
        // will only return -1 if the socket is bogus
        if(scarab_os_geterr() != ENOPROTOOPT) {
            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
            close(ts->fd);
            return NULL;
        }
    }
#endif

    if(setSocketOptionFlag(ts->fd, SO_REUSEADDR) < 0) {
        // will only return -1 if the socket is bogus
        if(scarab_os_geterr() != ENOPROTOOPT) {
            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
            close(ts->fd);
            return NULL;
        }
    }
    //</EDIT>

        
	if (ts->fd < 0)	{
		scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
		return NULL;
	}
    
	if (bind(ts->fd, (struct sockaddr *)&end_point, sizeof(end_point)) < 0)	{
		scarab_session_seterr(session, STREAM_ERR_BIND);
		close(ts->fd);
		return NULL;
	}
       
    /*
	 * Lets use 5 for a good default.  Just because I like 5
	 * for a listener.
	 */
	if (listen(ts->fd, 5) != 0)	{
		scarab_session_seterr(session, STREAM_ERR_LISTEN);
		close(ts->fd);
		return NULL;
	}

    //<EDIT>
//    if(setSocketOptionFlag(ts->fd, SO_REUSEPORT) < 0) {
//        // will only return -1 if the socket is bogus
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
//    if(setSocketOptionFlag(ts->fd, SO_REUSEADDR) < 0) {
//        // will only return -1 if the socket is bogus
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
//    if(setSocketOptionFlag(ts->fd, SO_KEEPALIVE) < 0) {
//        // will only return -1 if the socket is bogus
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
    //</EDIT>
    
	return &ts->header;
}

static ScarabStream *
stream_accept(ScarabSession * session, 
			  const char *uri, ScarabStream * binding) {
	tcpstr         *ts;
	tcpstr         *bd;
	//struct sockaddr_in end_point;
	struct sockaddr end_point;     // DDC edit: in a second they cast it into
								   // this kind of struct, which I think may be
								   // larger (and this causes memory corruption)
								   // (joy)
	//int					len
	socklen_t             len;     // DDC edit: there seems to be a lot of 
								   // sloppy casting going on in there

	bd = _tcpstr(binding);

	memset(&end_point, 0, sizeof(end_point));
	len = sizeof(end_point);

	/*
	 * Make the session and the TCP/IP socket.
	 */
	ts = (tcpstr*)scarab_mem_malloc(sizeof(tcpstr));
	if (!ts)
		return NULL;

	ts->fd = accept(bd->fd, (struct sockaddr *)&end_point, (socklen_t*)(&len));

	if (ts->fd < 0)	{
		scarab_session_seterr(session, STREAM_ERR_CONNECT);
		return NULL;
	}
    
	/*
	 * Get the packet size and use that, because Windows Sockets
	 * suck harder than a rock.
	 */
#ifdef SO_MAX_MSG_SIZE
	len = sizeof(ts->packet_size);
	getsockopt(ts->fd, SOL_SOCKET, SO_MAX_MSG_SIZE,
			   &ts->packet_size, &len);
	if (ts->packet_size < 1) {
		len = sizeof(ts->packet_size);
		getsockopt(ts->fd, SOL_SOCKET, SO_SNDBUF,
				   &ts->packet_size, &len);
	}
#else
	ts->packet_size = (unsigned int)-1;
#endif

	/*
	 * Try to disable Nagle's algorithm for send grouping as
	 * this can delay LDO.  Besides, we'll be doing send buffering
	 * ourselves well above here (and most likely blowing the OS
	 * buffers _anyway_.
	 */
#ifdef USE_TCP_NODELAY
#ifdef TCP_NO_DELAY
	setsockopt(ts->fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&len,
                                                                sizeof(len));
#endif
#endif
    //<EDIT>
//    if(setSocketOptionFlag(ts->fd, SO_KEEPALIVE) < 0) {
//        // will only return -1 if the socket is bogus
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
//    // setting a timeout on receives and sends
//    struct timeval tv;
//    tv.tv_sec = 3;
//    tv.tv_usec = 0;
//    if(setsockopt(ts->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
//    if(setsockopt(ts->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
    //</EDIT>
	return &ts->header;
}

static ScarabStream * stream_connect(ScarabSession * session, const char *uri) {
	tcpstr         *ts;
	struct protoent *proto;
	struct hostent *host;

	struct sockaddr_in end_point;

	char           *host_name;
	char           *c;
	int             len;
	short           tcp_port = 0;

	/*
	 * Find the hostname and copy it into its own string.
	 */
	len = 0;

	if ('/' == *uri)
		uri++;
	if ('/' == *uri)
		uri++;

	for (c = (char *)uri; *c != 0 && *c != ':' && *c != '/'; c++)
	{
		len++;
	}

	host_name = (char *)scarab_mem_malloc(len + 1);
	if (!host_name)
		return NULL;
	memcpy(host_name, uri, len);
	host_name[len] = 0;

	/*
	 * Do we have a port?
	 */
	uri += len;
	if (':' == *uri)
	{
		tcp_port = atoi(uri + 1);
	}

	/*
	 * Locate TCP on the machine
	 */
	proto = getprotobyname("tcp");
	if (!proto)
		return NULL;

	/*
	 * Pack the end_point object.
	 */
	memset(&end_point, 0, sizeof(end_point));
	end_point.sin_family = PF_INET;
	end_point.sin_port = htons(tcp_port);

	/*
	 * Turn the hostName string into an IP address _somehow_.
	 */
	host = gethostbyname(host_name);
	if (host) {
		memcpy(&end_point.sin_addr, host->h_addr_list[0],
			   sizeof(end_point.sin_addr));
	} else {
		scarab_session_seterr(session, STREAM_ERR_INVALID_HOST);

		end_point.sin_addr.s_addr = inet_addr(host_name);
		if (0 == end_point.sin_addr.s_addr)
			return NULL;
	}

	/*
	 * Make the session and the TCP/IP stream to the server.
	 */
	ts = (tcpstr*)scarab_mem_malloc(sizeof(tcpstr));
	if (!ts)
		return NULL;

	ts->fd = socket(PF_INET, SOCK_STREAM, proto->p_proto);
	if (ts->fd < 0)	{
		scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
		return NULL;
	}

	if (connect(ts->fd, (struct sockaddr *)&end_point,
                                                    sizeof(end_point)) < 0) {
		scarab_session_seterr(session, STREAM_ERR_CONNECT);
		close(ts->fd);
		return NULL;
	}

    //<EDIT>
//    if(setSocketOptionFlag(ts->fd, SO_KEEPALIVE) < 0) {
//        // will only return -1 if the socket is bogus
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
//    // setting a timeout on receives and sends
//    struct timeval tv;
//    tv.tv_sec = 3;
//    tv.tv_usec = 0;
//    if(setsockopt(ts->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
//    if(setsockopt(ts->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
//        if(scarab_os_geterr() != ENOPROTOOPT) {
//            scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
//            close(ts->fd);
//            return NULL;
//        }
//    }
    //</EDIT>
    
	/*
	 * Get the packet size and use that, because Windows Sockets
	 * suck harder than a rock.
	 */
#ifdef SO_MAX_MSG_SIZE
	len = sizeof(ts->packet_size);
	getsockopt(ts->fd, SOL_SOCKET, SO_MAX_MSG_SIZE,
			   &ts->packet_size, &len);
	if (ts->packet_size < 1)
	{
		len = sizeof(ts->packet_size);
		getsockopt(ts->fd, SOL_SOCKET, SO_SNDBUF,
				   &ts->packet_size, &len);
	}
#else
	ts->packet_size = (unsigned int)-1;
#endif

	/*
	 * Try to disable Nagle's algorithm for send grouping as
	 * this can delay LDO.  Besides, we'll be doing send buffering
	 * ourselves well above here (and most likely blowing the OS
	 * buffers _anyway_.
	 */
#ifdef TCP_NODELAY
	setsockopt(ts->fd, IPPROTO_TCP,
			   TCP_NODELAY, (const char *)&len,
			   sizeof(len));
#endif

	return &ts->header;
}

static int stream_close(ScarabStream * stream) {
	tcpstr         *ts = _tcpstr(stream);

    // PAJ second argument to shutdown disallows send and receives
	shutdown(ts->fd, 2); 
	close(ts->fd);

	return 0;
}

static int stream_write(ScarabStream * stream, const void *buffer, int size) {
	tcpstr         *ts = _tcpstr(stream);
	unsigned char  *c;
	int             r;
	unsigned int    left;

	left = (unsigned int)size;
	c = (unsigned char *)buffer;

	do {
        r = send(ts->fd, c, (int)min(left, ts->packet_size), 0);
            if (r < 0) {
                return scarab_session_seterr(stream->session, STREAM_ERR_WRITE);
            }
		left -= r;
		c += r;
	} while (left);

	return size;
}

static int stream_send(ScarabStream * stream) {
	//tcpstr         *ts = _tcpstr(stream);
	return 0;
}


static int stream_flush(ScarabStream * stream) {
	//tcpstr         *ts = _tcpstr(stream);
	return 0;
}

static int stream_read(ScarabStream * stream, void *buffer, int size) {
	tcpstr         *ts = _tcpstr(stream);
	unsigned char  *c;
	int             r;
	int             tot;

	tot = size;
	c = (unsigned char *)buffer;
    //printf("stream_read() size = %d\n", size);
	do {
        // this call will block until we receive something.  It seems that
        // the first read will only want to read 1 byte.   This seems to be
        // a header for a scarab object. So the only chance recv has to block
        // is on a call to stream_read that is at the beginning of a datum
        // object.  all other calls to read are guarenteed to find data, the
        // rest of the object, unless there is an error.
        r = recv(ts->fd, c, size, 0);
        //printf("stream_read() - recv return code %d\n", r);
        //printf("stream_read() - buffer as c = %s\n", c);
        if (r < 0) {
			return scarab_session_seterr(stream->session, STREAM_ERR_READ);
		}
        //<EDIT>
        // Although 0 may be a posssible return value from the recv
        // function I am going to return at this point but not set an error
        // this will allow the calling thread time to react in case there is
        // impending doom on the connection it wont sit here in an endless
        // loop.  By returning 0 a NULL ScarabDatum will be returned farther
        // up the chain.
        if(r == 0) {
            return 0;
        }
        //</EDIT>
		size -= r;
		c += r;
	} while (size);

	return tot;
}

static int
stream_seek(ScarabStream *stream, long int offset, int origin)
{
	return -1;
}

static long int
stream_tell(ScarabStream *stream)
{
	return -1;
}

static char *error_text[] = {
	"No Error",
	"I/O Write Error",
	"I/O Read Error",
	"Invalid Hostname / IP Address",
	"Socket Error",
	"Connection Error",
	"Bind Error",
	"Listen Error",
	"Winsock Startup Error",
};

static char * stream_errstr(int code) {
	return error_text[code];
}

////////////////////////////////////////////////////
//int fillSocketStructure(ScarabStream * s, int peer,struct sockaddr_in * sock) {
//    struct sockaddr_storage addr;
//    int len = sizeof(addr);
//    tcpstr * ts = (tcpstr *)(s);
//    if(peer) {
//        if(getpeername(ts->fd, (struct sockaddr *)&addr, 
//                                                (socklen_t*)(&len)) < 0) {
//            // Error occurred
//            //printf("Error in Scarab::get_local_port()\n");
//            return -1;
//        }
//    } else {
//        if(getsockname(ts->fd, (struct sockaddr *)&addr, 
//                                                (socklen_t*)(&len)) < 0) {
//            // Error occurred
//            //printf("Error in Scarab::get_local_port()\n");
//            return -1;
//        }
//    if(addr.ss_family != AF_INET) {
//        //printf("Incorrect Family in Scarab::get_local_port()\n");
//        return -2;
//    }
//    sock = (struct sockaddr_in *)&addr;
//    return 0;
//}

static int stream_local_port(ScarabStream * s) {
//    struct sockaddr_in * sin;
//    if(fillSocketStructure(s, 0, sin)) {
//        return (sin->sin_port);
//    } else {
//        return -1;
//    }
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getsockname(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
            // Error occurred
            //printf("Error in Scarab::get_local_port()\n");
            return -1;
        }
    if(addr.ss_family != AF_INET) {
        //printf("Incorrect Family in Scarab::get_local_port()\n");
        return -2;
    }
    struct sockaddr_in * sock = (struct sockaddr_in *)&addr;
    return sock->sin_port;
}

static char * stream_local_address(ScarabStream * s) {
    static char str[128];
//    struct sockaddr_in * sin;
//    if(fillSocketStructure(s, 0, *sin)) {
//        if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
//            return(NULL);
//        }
//        return(str);
//    } else {
//        return NULL;
//    }
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getsockname(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
            // Error occurred
            //printf("Error in Scarab::get_local_port()\n");
            return NULL;
    }
    if(addr.ss_family != AF_INET) {
        //printf("Incorrect Family in Scarab::get_local_port()\n");
        return NULL;
    }
    struct sockaddr_in * sin = (struct sockaddr_in *)&addr;
    if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
        return(NULL);
    }
    return(str);
}

// ScarabSession should be from a connected file
// descriptor after accept returns
static int stream_foreign_port(ScarabStream * s) {
//    struct sockaddr_in * sin;
//    if(fillSocketStructure(s, 1, *sin)) {
//        return (sin->sin_port);
//    } else {
//        return -1;
//    }
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getpeername(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
            // Error occurred
            //printf("Error in Scarab::get_local_port()\n");
            return -1;
        }
    if(addr.ss_family != AF_INET) {
        //printf("Incorrect Family in Scarab::get_local_port()\n");
        return -2;
    }
    struct sockaddr_in * sock = (struct sockaddr_in *)&addr;
    return sock->sin_port;
}

static char * stream_foreign_address(ScarabStream * s) {
    static char str[128];
//    struct sockaddr_in * sin;
//    if(fillSocketStructure(s, 1, *sin)) {
//        if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
//            return(NULL);
//        }
//        return(str);
//    } else {
//        return NULL;
//    }
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getpeername(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
            // Error occurred
            //printf("Error in Scarab::get_local_port()\n");
            return NULL;
    }
    if(addr.ss_family != AF_INET) {
        //printf("Incorrect Family in Scarab::get_local_port()\n");
        return NULL;
    }
    struct sockaddr_in * sin = (struct sockaddr_in *)&addr;
    if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
        return(NULL);
    }
    return(str);
}
/////////////////////////////////////////////////////////////

ScarabStreamEngine scarab_stream_tcpip = {
	SCARAB_STREAM_HEADER(STREAM_ERR_MAX),

	lib_init,
	stream_bind,
	stream_accept,
	stream_connect,
	stream_close,

	stream_write,
	stream_send,
	stream_flush,

	stream_read,
	stream_seek,
	stream_tell,

	stream_errstr,

	"tcp",
	"Shawn Pearce <spearce@spearce.org>",
	"TCP/IP",
    
    stream_local_port,
    stream_local_address,
    stream_foreign_port,
    stream_foreign_address
};
//remove comma after TCP/IP
