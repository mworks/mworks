"""  $Id: ScarabConnection.py,v 1.3 2000/03/13 22:32:35 kmacleod Exp $

Scarab Connection class

The Scarab Connection class provides the housekeeping for setting up
connections, setting parameters, and creating the initial root proxy
for a server.
"""

from urllib import splittype, splithost, splitport
import types
import sys
from traceback import format_exception
import string
from string import find

ScarabConnectionError = "ScarabConnection.ScarabConnectionError"
ScarabConnectionRemoteError = "ScarabConnection.ScarabConnectionRemoteError"

class ScarabConnection:
    def __init__(self, options):
	self.globals = {}
	self.is_valid = 1
        self.debug = 0

        if options != None:
            if type(options) == types.InstanceType:
                options = options.__dict__

            for key in options.keys():
                setattr(self, key, options[key])

    def root_proxy(self):
	return ScarabProxy(self, 'root')

    def call(self, object, method, args):
	"""Forwards an object method and arguments to a remote
	server to be executed.  Exceptions in the remote
	server are propagated to the local caller."""

        if self.protocol == "soap":
            if len(args) != 1:
                raise ScarabConnectionError, \
                      "More than one argument to a SOAP method"
            # FIXME sanity checking on SOAP top-level
            self.pickler.encode_call(method, args[0])
        else:
            self.pickler.dump({ 'object' : object,
                                'method' : method,
                                'args' : args })
        try:
            self.pickler.flush()
        except AttributeError:
            self.stream.flush()

        result = self.unpickler.load()
        if self.protocol == "soap":
            # FIXME check for invalid data
            method = result['SOAP:Envelope']['SOAP:Body'].keys()[0]
            if method == "SOAP:Fault":
                result = { 'error' : result['SOAP:Envelope']['SOAP:Body']['SOAP:Fault']['message'] }
            else:
                result = { 'result' : result['SOAP:Envelope']['SOAP:Body'][method] }
	t = type(result)
	if t != types.DictType:
	    raise ScarabConnectionError, \
		"expected dictionary result, got %s" % `t.__name__`

	if result.has_key('error'):
	    raise ScarabConnectionRemoteError, result['error']

	return result['result'][0]

    def accept(self):
        self.log_info("checking connection protocol", 2)
        if not hasattr(self, 'unpickler'):
            stream = ScarabBufferedFile(self.stream)
            # allow exceptions to bubble up
            # FIXME should mark connection as invalid
            # FIXME could block on stream having less than 100 bytes
            head = stream.head(4)
            if head[0:4] == ("\x89" + "CBF"):
                import LDOBinary
                self.unpickler = LDOBinary.LDOBinaryUnmarshaler(stream)
                self.pickler = LDOBinary.LDOBinaryMarshaler(stream)
                self.protocol = 'ldobinary'
            elif head[0:2] == '<?':
                head = stream.head(100)
                if string.find(head, 'urn:schemas-xmlsoap-org:soap.v1') != -1:
                    import SOAP
                    self.unpickler = SOAP.SOAPUnmarshaler(stream)
                    self.pickler = SOAP.SOAPMarshaler(stream)
                    self.protocol = 'soap'
                else:
                    import LDOXML
                    self.unpickler = LDOXML.LDOXMLUnmarshaler(stream)
                    self.pickler = LDOXML.LDOXMLMarshaler(stream)
                    self.protocol = "ldoxml"
            elif head[0:1] != ' ' and head[0:1] != "\t" and head[0:1] != '<':
                import cPickle
                self.unpickler = cPickle.Unpickler(stream)
                self.pickler = cPickle.Pickler(stream)
                self.protocol = "pickle"
            else:
                self.log_info("accept: unrecognized serialization", 2)
                """FIXME LDO-XML or other XML format"""
                raise ScarabConnectionError

            self.log_info("accepted " + self.protocol + " connection")

	error = None
	try:
            request = self.unpickler.load()
            if self.protocol == "soap":
                # FIXME check for invalid data
                method = request['SOAP:Envelope']['SOAP:Body'].keys()[0]
                request = { 'object' : 'root',
                            'method' : method,
                            'args'   : [ request['SOAP:Envelope']['SOAP:Body'][method] ] }
            if self.debug >= 2:
                self.log_info("request: " + str(request), 3)
	except EOFError:
            self.log_info("EOF on connection", 2)
	    self.is_valid = 0
	    # FIXME close pickler (which'll close stream?)
	    # FIXME notify conn manager
	    return
	except:
            exc_type, value, traceback = sys.exc_info()
            # since we load picklers dynamically, we need to check their
            # errors in a dynamic way
            if str(exc_type) == "cPickle.UnpicklingError":
                self.log_info("EOF on connection", 2)
                self.is_valid = 0
                # FIXME close pickler (which'll close stream?)
                # FIXME notify conn manager
                return
                
	    error = "error unmarshaling, closing connection: " \
                    + string.join(format_exception(exc_type, value, traceback))
	    self.is_valid = 0
	    # we'll still try to send a failure response, ya never know
	else:
	    t = type(request)
	    if t != types.DictType:
		error = "expected dictionary for request, got %s" % `t.__name__`
	    else:
		if (not(request.has_key('object')
                        and request.has_key('method')
                        and request.has_key('args'))):
		    error = "missing object id, method name, or args in request"
		else:
		    object = request['object']
		    method_name = request['method']

		    if not self.globals.has_key(object):
			error = "no such object registered: %s" % `object`
		    else:
			try:
			    method = getattr(self.globals[object], method_name)
			except:
			    error = "no such method %s for object %s" \
                                    % (method_name, `object`,)

	if error != None:
            self.log_info(error)
	    result = { 'error' : error }
	else:
            call_str = object + "." + method_name
            self.log_info("calling `" + call_str + "'")
	    try:
		result = apply(method, tuple(request['args']))
	    except:
                exc_type, value, traceback = sys.exc_info()
                exc_str = string.join(format_exception(exc_type, value, traceback))
                self.log_info("exception raised in `" + call_str + "': " + exc_str)
		result = { 'error' : exc_str }
	    else:
                self.log_info("`" + call_str + "' returned successfully", 2)
		result = { 'result' : [ result ] }

	try:
            if self.debug >= 2:
                self.log_info("response: " + str(result), 2)
            if self.protocol == "soap":
                if result.has_key('error'):
                    self.pickler.encode_fault(100, result['error'], 1)
                else:
                    self.pickler.encode_response(method_name,
                                                 result['result'][0])
            else:
                self.pickler.dump(result)
            try:
                self.pickler.flush()
            except AttributeError:
                self.stream.flush()
	except:
            exc_type, value, traceback = sys.exc_info()
            self.log_info("error sending response"
                          + string.join(format_exception(exc_type,
                                                         value,
                                                         traceback)))
	    if self.is_valid:
		"""FIXME ignore, log, or reraise?"""
	    # else: ignore

	# FIXME if not self.is_valid: pickler.close

    def process_string(self, message):
        import StringIO
        self.stream = StringIO.StringIO(message)
        self.accept()
        delattr(self.stream)
        delattr(self.pickler)
        delattr(self.unpickler)

    def run_loop(self):
	# FIXME this is socket specific, see FIXMEs in Scarab.py
        while self.is_valid:
            self.log_info("awaiting connection", 2)
            socket, addr = self.socket.accept()
            self.caller = str(addr)
            stream = socket.makefile('r+')
            server = ScarabConnection(self)
            if self.debug >= 4:
                server.stream = ScarabDebugFile(stream, self)
            else:
                server.stream = stream
            while server.is_valid:
                server.accept()

    def log_info(self, message, level = 1):
        if self.debug >= level:
            if hasattr(self, 'caller'):
                print self.caller + ": " + message
            else:
                print message

def connect_to(url, options = {}):
    conn = ScarabConnection(options)
    # FIXME this should take the lower-level protocol and produce
    # a connection that can do either multiple or one-shot
    # connections.  For right now, we assume it's a TCP
    # connection, hence a socket stream

    type, path = splittype(url)
    host, dummy_path = splithost(path)
    host, port = splitport(host)

    if port == None:
	raise "no port defined and no defaults yet"

    port = int(port)

    from socket import *
    conn.socket = socket(AF_INET, SOCK_STREAM)
    conn.socket.connect(host, port)
    conn.stream = conn.socket.makefile('r+')

    # FIXME we assume one pickler instance, too, for that stream
    conn.pickler = conn.pickler_class(conn.stream)
    conn.unpickler = conn.unpickler_class(conn.stream)

    return conn

def connect_to_root_at(url, options):
    conn = server(url, options)
    return conn.root

def server_on(url = '', options = {}):
    conn = ScarabConnection(options)

    # url defaults to '', but may be passed as None by user
    if url == None:
        url = ''

    conn.lower = url
    # FIXME see FIXME in Scarab.server()

    host, dummy_path = splithost(url)
    if host == None:
        host = ''
        port = None
    else:
        host, port = splitport(host)

    if port == None:
	port = 0

    from socket import *
    conn.socket = socket(AF_INET, SOCK_STREAM)
    conn.socket.bind(host, port)
    conn.socket.listen(1)

    ip_addr, ip_port = conn.socket.getsockname()
    if ip_addr == '0.0.0.0':
        ip_host, dummy, dummy = gethostbyaddr(gethostname())
    else:
        ip_host, aliases, ip_addrs = gethostbyaddr(ip_addr)
    conn.url = 'ANY:tcp://' + ip_host + ':' + str(ip_port) + '/'

    conn.log_info("server opened on " + conn.url)

    return conn

class ScarabBufferedFile:
    def __init__(self, stream):
        self.stream = stream
        self.str_read = stream.read
        self.str_readline = stream.readline
        self.write = stream.write
        self.flush = stream.flush
        self._head = ""
        self._head_bytes = 0

    def head(self, num_bytes):
        if self._head_bytes > 0:
            num_bytes = num_bytes - self._head_bytes
            if num_bytes <= 0:
                return self._head
        bytes_to_go = num_bytes
        bytes = ''
        while bytes_to_go > 0:
            partial_bytes = self.str_read(bytes_to_go)
            bytes_to_go = bytes_to_go - len(partial_bytes)
            bytes = bytes + partial_bytes

        self._head = self._head + bytes
        self._head_bytes = self._head_bytes + num_bytes
        return bytes

    def read(self, size = -1):
        if self._head_bytes > 0:
            if size < 0:
                # Return rest of file
                bytes = self._head
                bytes = bytes + self.str_read()
                self._head = ''
                self._head_bytes = 0
            elif size <= self._head_bytes:
                # read part of self._head_bytes
                bytes = self._head[0:size]
                self._head = self._head[size:]
                self._head_bytes = self._head_bytes - size
            else:
                # read all of self._head_bytes and then some
                bytes = self._head
                size = size - self._head_bytes
                bytes = bytes + self.str_read(size)
                self._head = ''
                self._head_bytes = 0
        else:
            # we have no head bytes, read completely from file
            return self.str_read(size)

        return bytes

    def readline(self, size = -1):
        if self._head_bytes > 0:
            if size < 0:
                nl_index = find(self._head, "\n") + 1
                if nl_index == 0:
                    # Return head + a real readline
                    bytes = self._head + self.str_readline()
                    self._head = ''
                    self._head_bytes = 0
                else:
                    # return only portion of head
                    bytes = self._head[0:nl_index]
                    self._head = self._head[nl_index:]
                    self._head_bytes = self._head_bytes - nl_index
            elif size <= self._head_bytes:
                nl_index = find(self._head, "\n", 0, size) + 1
                if nl_index == 0:
                    # Return only a `size' portion of head
                    bytes = self._head[0:size]
                    self._head = self._head[size:]
                    self._head_bytes = self._head_bytes - size
                else:
                    # return only an `nl_index' bytes of head
                    bytes = self._head[0:nl_index]
                    self._head = self._head[nl_index:]
                    self._head_bytes = self._head_bytes - nl_index
            else:
                nl_index = find(self._head, "\n") + 1
                if nl_index == 0:
                    # Return all of head and then some
                    bytes = self._head + self.str_readline(size
                                                          - self._head_bytes)
                    self._head = ''
                    self._head_bytes = 0
                else:
                    # return only an `nl_index' bytes of head
                    bytes = self._head[0:nl_index]
                    self._head = self._head[nl_index:]
                    self._head_bytes = self._head_bytes - nl_index
            return bytes
        else:
            # we have no head bytes, read completely from file
            if size == -1:
                # work-around for a bug in StreamIO, not accepting -1
                # as the same as no argument
                return self.str_readline()
            else:
                return self.str_readline(size)

class ScarabDebugFile:
    def __init__(self, stream, log):
        self.stream = stream
        self.str_read = stream.read
        self.str_readline = stream.readline
        self.str_write = stream.write
        self.str_flush = stream.flush
        self.log_info = log.log_info
        self.is_binary = 0

    def read(self, size = -1):
        if size == -1:
            bytes = selft.str_read()
        else:
            bytes = self.str_read(size)
        self._log('read: ', bytes)
        return (bytes)

    def readline(self, size = -1):
        if size == -1:
            bytes = self.str_readline()
        else:
            bytes = self.str_readline(size)
        self._log('readline: ', bytes)
        return (bytes)

    def write(self, bytes):
        self._log('write: ', bytes)
        self.str_write(bytes)

    def flush(self):
        self.log_info("flush(str)")
        self.str_flush()

    def _log(self, mode, strng):
        ii = 0
        while ii < len(strng):
            char = strng[ii]
            if ((char != "\n" and char != "\f")
                and (ord(char) < 32 or ord(char) > 127)):
                self.is_binary = 1
                break
            ii = ii + 1
        if self.is_binary:
            ii = 0
            while ii < len(strng):
                jj = 0
                line_str = ""
                while jj < 8 and ii < len(strng):
                    line_str = line_str + hex(ord(strng[ii])) + " "
                    jj = jj + 1
                    ii = ii + 1
                self.log_info(mode + line_str)
        else:
            self.log_info(mode + strng)
