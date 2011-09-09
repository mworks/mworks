/**
 * stream_tcpip_select.c
 *
 * Description:
 *
 * History:
 * Paul Jankunas on 8/10/05 - Created.
 * Copyright 2005 MIT. All rights reserved.
 */

#include <scarab.h>
#include <errno.h>

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

FWD(struct, tcpstr);
DEF(struct, tcpstr) {
	ScarabStream    header;
	int             fd;
	unsigned int    packet_size;
};

#define	_tcpstr(x)	((tcpstr*)(x))

#ifndef min
#define	min(x,y)	((x) < (y) ? (x) : (y))
#endif

extern ScarabStreamEngine scarab_stream_tcpip_select;

#define	STREAM_SELF				&scarab_stream_tcpip_select

#define	STREAM_ERR_NONE			scarab_errcode(STREAM_SELF,	0)
#define	STREAM_ERR_WRITE		scarab_errcode(STREAM_SELF,	1)
#define	STREAM_ERR_READ			scarab_errcode(STREAM_SELF,	2)
#define	STREAM_ERR_INVALID_HOST	scarab_errcode(STREAM_SELF,	3)
#define	STREAM_ERR_INVALID_SOCK	scarab_errcode(STREAM_SELF,	4)
#define	STREAM_ERR_CONNECT		scarab_errcode(STREAM_SELF,	5)
#define	STREAM_ERR_BIND			scarab_errcode(STREAM_SELF,	6)
#define	STREAM_ERR_LISTEN		scarab_errcode(STREAM_SELF,	7)
#define	STREAM_ERR_WSASTARTUP	scarab_errcode(STREAM_SELF,	8)
#define STREAM_ERR_SELECT       scarab_errcode(STREAM_SELF, 9)
#define STREAM_ERR_TIMEDOUT     scarab_errcode(STREAM_SELF, 10)

#define	STREAM_ERR_MAX              100
#define ACCEPT_SELECT_TIMEOUT_SEC   5
#define READ_SELECT_TIMEOUT_SEC     5
#define READ_SOCKET_FUBAR           -99

int SCARAB_SELECT_TIMEDOUT;
int READ_FUBAR_FLAG;
	
static int      sel_lib_init(void);
static ScarabStream *selstr_bind(ScarabSession * session, const char *uri);
static ScarabStream *selstr_accept(ScarabSession * session, const char *uri,
                                                    ScarabStream * binding);
static ScarabStream *selstr_connect(ScarabSession * session, const char *uri);
static int      selstr_close(ScarabStream * stream);
static int      selstr_write(ScarabStream * stream, const void *buffer, int size);
static int      selstr_send(ScarabStream * stream);
static int      selstr_flush(ScarabStream * stream);
static int      selstr_read(ScarabStream * stream, void *buffer, int size);
static int		selstr_seek(ScarabStream *stream, long int offset, int origin);
static long int selstr_tell(ScarabStream *stream);
// sets the file descriptor to be non-blocking.
int setNonBlocking(int fd);
int turnOffNonBlocking(int fd);

static int sel_lib_init(void) {
    SCARAB_SELECT_TIMEDOUT = STREAM_ERR_TIMEDOUT;
    READ_FUBAR_FLAG = 0;
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

static ScarabStream * selstr_bind(ScarabSession * session, const char *uri) {
	tcpstr         *ts; //PAJ a structure with a ScarabStream header a 
                        //PAJ file descriptor and packet size
	struct protoent *proto;
	struct hostent *host;

	struct sockaddr_in end_point;   //PAJ socket adress structure we created

	char           *host_name;
	char           *c;
	int             len;
    int             turn_on;
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
	if (!ts) return NULL;

    ts->fd = socket(AF_INET, SOCK_STREAM, proto->p_proto);

	if (ts->fd < 0)	{
		scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
		return NULL;
	}
    
    // make the socket non blocking so that calls to accept will not 
    // block if a client disconnects before the server has a chance
    // to call accept.
    if(setNonBlocking(ts->fd) < 0 ) {
        // just send an invalid sock error here
		scarab_session_seterr(session, STREAM_ERR_INVALID_SOCK);
		close(ts->fd);
		return NULL;
    }
    // we now have a non-blocking socket
    // allow the server socket to reuse the address
    turn_on = 1;
    setsockopt(ts->fd, SOL_SOCKET, SO_REUSEADDR, &turn_on, sizeof(int));
    
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
	return &ts->header;
}

static ScarabStream *
selstr_accept(ScarabSession * session, const char *uri, ScarabStream * binding) {
	tcpstr         *ts;
	tcpstr         *bd;
	struct sockaddr_in end_point;
	int             len;
    fd_set          read_set;
    int             maxfdplus1;
    struct timeval  selectTimeout;
    int             select_rc;
    select_rc = 0;
    // bd is a non-blocking socket!
	bd = _tcpstr(binding);

	memset(&end_point, 0, sizeof(end_point));
	len = sizeof(end_point);

	/*
	 * Make the session and the TCP/IP socket.
	 */
	ts = (tcpstr*)scarab_mem_malloc(sizeof(tcpstr));
	if (!ts)
		return NULL;
    
    // we must use select here to wait for valid connections.  
    // we set a timeout so that the calling program has a chance to decide 
    // if it still wants to accept clients.
    FD_ZERO(&read_set); // zero out the set
    // wait for the listen socket to become readable
    FD_SET(bd->fd, &read_set);
    maxfdplus1 = bd->fd + 1;
    
    // every minute it will give the calling program a chance to either
    // come back to accept more clients or perform some other task.
    selectTimeout.tv_sec = ACCEPT_SELECT_TIMEOUT_SEC;
    selectTimeout.tv_usec = 0;

    // this call will block
    select_rc = select(maxfdplus1, &read_set, NULL, NULL, &selectTimeout);

    if(select_rc == 0) {
        // a timeout occurred 
        scarab_session_seterr(session, STREAM_ERR_TIMEDOUT);
        return NULL;
    }
    
    if(select_rc < 0) {
        // an error occurred
        scarab_session_seterr(session, STREAM_ERR_SELECT);
        return NULL;
    }
    
    if(!FD_ISSET(bd->fd, &read_set)) {
        // the wrond descriptor is set which should be impossible
        scarab_session_seterr(session, STREAM_ERR_SELECT);
        return NULL;
    }
    // this call should not block any longer, but can return bad values
    // if the client closed while waiting for this call, although 
    // highly unlikely.
    ts->fd = accept(bd->fd, (struct sockaddr *)&end_point, (socklen_t*)(&len));
    // this will happen on cases where the accept would have blocked
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
    getsockopt(ts->fd, SOL_SOCKET, SO_MAX_MSG_SIZE, &ts->packet_size, &len);
    if (ts->packet_size < 1) {
        len = sizeof(ts->packet_size);
        getsockopt(ts->fd, SOL_SOCKET, SO_SNDBUF, &ts->packet_size, &len);
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
    setsockopt(ts->fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&len,
                                                                sizeof(len));
#endif
    
    return &ts->header;
}

static ScarabStream * selstr_connect(ScarabSession * session, const char *uri) {
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

static int selstr_close(ScarabStream * stream) {
	tcpstr         *ts = _tcpstr(stream);
    shutdown(ts->fd, 2); 
	close(ts->fd);

	return 0;
}

static int selstr_write(ScarabStream * stream, const void *buffer, int size) {
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

static int selstr_send(ScarabStream * stream) {
	return 0;
}

static int selstr_flush(ScarabStream * stream) {
	return 0;
}

static int selstr_read(ScarabStream * stream, void *buffer, int size) {
	tcpstr         *ts = _tcpstr(stream);
	unsigned char  *c;
	int             r;
	int             tot;
    fd_set          readSet; // read file descriptors
    struct timeval  selectTimeout;
    int             select_rc;
    
	tot = size;
	c = (unsigned char *)buffer;
    FD_ZERO(&readSet); // zero our read set
    
	do {
        // we are interested in reading from said socket
        // so mark it in the descriptor set
        FD_SET(ts->fd, &readSet);
        selectTimeout.tv_sec  = READ_SELECT_TIMEOUT_SEC;
        selectTimeout.tv_usec = 0;
        // we block on the select call until there is data to read.
        select_rc = select(ts->fd + 1, &readSet, NULL, NULL, &selectTimeout);
        if(select_rc < 0) {
            // a select error occured EINTR is probably most common
            // means a signal occured before data was ready or time expired
            return scarab_session_seterr(stream->session, STREAM_ERR_SELECT);
        }
        if(select_rc == 0) {
            // a timeout occurred give the program or thread to do something
            // useful during this time.
           // fprintf(stderr, "Read timeout....you guys are high!!\n");
            return 0;
        }
        if(FD_ISSET(ts->fd, &readSet)) {
            r = recv(ts->fd, c, size, 0);
            if (r < 0) {
                return scarab_session_seterr(stream->session, STREAM_ERR_READ);
            }
            // this happens if the other half of the socket has been closed
            if(r == 0) {
                //fprintf(stderr, "read read zero bytes oh noes!\n");
                READ_FUBAR_FLAG = 1;
                return -1;
            }
            size -= r;
            c += r;
        }
    } while (size);
	return tot;
}

static int
selstr_seek(ScarabStream *stream, long int offset, int origin)
{
	return -1;
}

static long int
selstr_tell(ScarabStream *stream)
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
    "Select Read Error",
    "Select Timed Out",
};

static char * selstr_errstr(int code) {
	return error_text[code];
}

static int selstr_local_port(ScarabStream * s) {
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getsockname(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
        // Error occurred
        return -1;
    }
    if(addr.ss_family != AF_INET) {
        return -2;
    }
    struct sockaddr_in * sock = (struct sockaddr_in *)&addr;
    return sock->sin_port;
}

static char * selstr_local_address(ScarabStream * s) {
    static char str[128];
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getsockname(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
        // Error occurred
        return NULL;
    }
    if(addr.ss_family != AF_INET) {
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
static int selstr_foreign_port(ScarabStream * s) {
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getpeername(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
        // Error occurred
        return -1;
    }
    if(addr.ss_family != AF_INET) {
        return -2;
    }
    struct sockaddr_in * sock = (struct sockaddr_in *)&addr;
    return sock->sin_port;
}

static char * selstr_foreign_address(ScarabStream * s) {
    static char str[128];
    struct sockaddr_storage addr;
    int len = sizeof(addr);
    tcpstr * ts = (tcpstr *)(s);
    if(getpeername(ts->fd, (struct sockaddr *)&addr, (socklen_t*)(&len)) < 0) {
        // Error occurred
        return NULL;
    }
    if(addr.ss_family != AF_INET) {
        return NULL;
    }
    struct sockaddr_in * sin = (struct sockaddr_in *)&addr;
    if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
        return(NULL);
    }
    return(str);
}

int setNonBlocking(int fd) {
    int flags;
    if((flags = fcntl(fd, F_GETFL, 0)) < 0) {
        return flags;
    }
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

int turnOffNonBlocking(int fd) {
    int flags;
    if((flags = fcntl(fd, F_GETFL, 0)) < 0) {
        return flags;
    }
    flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

ScarabStreamEngine scarab_stream_tcpip_select = {
	SCARAB_STREAM_HEADER(STREAM_ERR_MAX),

	sel_lib_init,
	selstr_bind,
	selstr_accept,
	selstr_connect,
	selstr_close,

	selstr_write,
	selstr_send,
	selstr_flush,

	selstr_read,
	selstr_seek,
	selstr_tell,

	selstr_errstr,

	"tcpselect",
	"Mabbit is l33t",
	"TCP/IP Select Blocks Read",
    
    selstr_local_port,
    selstr_local_address,
    selstr_foreign_port,
    selstr_foreign_address
};
