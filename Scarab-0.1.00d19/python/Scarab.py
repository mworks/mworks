""" $Id: Scarab.py,v 1.3 2000/03/13 22:32:35 kmacleod Exp $

Scarab.py implements the basic user API for using Scarab as either a
client or a server.  It can create connections and proxies for remote
servers and run a server loop for implementing a server.

Scarab URLs are two-part URLs that contain an upper-level protocol
scheme (ldoxml, ldo (ldobinary), xmlrpc, soap, wddx, pickle) and a
lower-level protocol scheme (http, mailto, tcp [default]) like this:

  xmlrpc:http://casbah.org/listener

connect_to() accepts an Scarab URL and returns a connection object.

connect_to_root_at() accepts an Scarab URL and returns the root proxy
object of the server.

server_on() accepts an Scarab URL or an ordinary URL.  If given an
Scarab URL then it only supports the given upper-level protocol.  If
given an ordinary URL then it attempts to determine the upper-level
protocol on each connection.

AVAILABLE PROTOCOLS

  Serialization:

    soap: Simple Object Access Protocol
    ldoxml: The reference protocol for Scarab
    ldo, ldobinary: A binary implementation of the reference protocol
    pickle: Python's native pickle format, can marshal all types of
            Python data but can only be used when Python is on both
            ends of the link

  Transport:

    http: via CGI
    tcp: straight-sockets

`tcp' is also the default protocol and can be left off of Scarab URLs.
The following two are synonomous:

  ldo:tcp://anysite.com:4321/
  ldo://anysite.com:4321/

There are no default ports for tcp connections, a port must always be
specified.

"""

import urlparse
from urllib import splittype, splithost, splitport
import ScarabConnection

uppers = ['pickle', 'ldo', 'ldobinary', 'ldoxml', 'soap']

def connect_to(scarab_url, options = {}):
    """ Create a connection object for the server at
    'scarab_url' """

    options['upper'], lower = splittype(scarab_url)

    if options['upper'] == 'ANY':
        options['upper'] = None

    options['pickler_class'], options['unpickler_class'], options['protocol'] = _picklers(options['upper'])

    return ScarabConnection.connect_to(lower, options)

def server_on(scarab_url = None, options = {}):
    """ Create a server connection for 'scarab_url' """

    if scarab_url == None:
        options['upper'] = None
        lower = None
    else:
        options['upper'], lower = splittype(scarab_url)

    options['pickler_class'], options['unpickler_class'], options['protocol'] = _picklers(options['upper'])

    return ScarabConnection.server_on(lower, options)

def _picklers(protocol = None):
    """ _picklers accepts a protocol scheme and returns a picker and
    unpickler Class objects for that scheme.  If the scheme is not
    given or isn't an upper level protocol scheme, ldobinary is used
    by default. """

    if protocol == None:
	protocol = 'ldobinary'

    if protocol not in uppers:
	protocol = 'ldobinary'

    if protocol == 'pickle':
	import cPickle
	pickler = cPickle.Pickler
	unpickler = cPickle.Unpickler
    elif protocol == 'ldoxml':
        import LDOXML
        pickler = LDOXML.LDOXMLMarshaler
        unpickler = LDOXML.LDOXMLUnmarshaler
    elif protocol == 'soap':
        import SOAP
        pickler = SOAP.SOAPMarshaler
        unpickler = SOAP.SOAPUnmarshaler
    elif (protocol == 'ldobinary'
          or protocol == 'ldo'):
	import LDOBinary
	pickler = LDOBinary.LDOBinaryMarshaler
	unpickler = LDOBinary.LDOBinaryUnmarshaler

    return pickler, unpickler, protocol
