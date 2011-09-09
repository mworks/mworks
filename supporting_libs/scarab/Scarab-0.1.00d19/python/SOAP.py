"""  $Id: SOAP.py,v 1.2 2000/03/13 22:32:35 kmacleod Exp $

SOAP.py implements the Simple Object Access Protocol
<http://develop.com/SOAP/>

"""

from StringIO import StringIO
from ScarabMarshal import *
from xml.sax import saxlib, saxexts
from types import *
import string
import base64

# just constants
DICT = "dict"
ARRAY = "array"
CHAR = "char"

class SOAPMarshaler(Marshaler):

    def __init__(self, stream):
        self.written_stream_header = 0
	self.write = stream.write
        self.flush = stream.flush

    def encode_call(self, method, map):
        dict = { 'id':0 }
        self.m_init(dict)
        self.write('''<SOAP:Envelope
    xmlns:SOAP="urn:schemas-xmlsoap-org:soap.v1"
    xmlns:tbd="ToBeDetermined">
  <SOAP:Body>
''')

        self._marshal({ method : map }, dict)

        self.write('''  </SOAP:Body>
</SOAP:Envelope>
''')
        self.m_finish(dict)

    def encode_response(self, method, map):
        dict = { 'id':0 }
        self.m_init(dict)
        self.write('''<SOAP:Envelope
    xmlns:SOAP="urn:schemas-xmlsoap-org:soap.v1"
    xmlns:tbd="ToBeDetermined">
  <SOAP:Body>
''')

        self._marshal({ method + "Response" : map }, dict)

        self.write('''  </SOAP:Body>
</SOAP:Envelope>
''')
        self.m_finish(dict)

    def encode_fault(self, faultcode, message, runcode, detail = None):
        dict = { 'id':0 }
        self.m_init(dict)
        self.write('''<SOAP:Envelope
    xmlns:SOAP="urn:schemas-xmlsoap-org:soap.v1"
    xmlns:tbd="ToBeDetermined">
  <SOAP:Body>
''')

        if detail == None:
            self._marshal({ "SOAP:Fault" : { 'faultcode'   : faultcode,
                                             'faultstring' : message,
                                             'runcode'     : runcode } },
                          dict)
        else:
            self._marshal({ "SOAP:Fault" : { 'faultcode'   : faultcode,
                                             'faultstring' : message,
                                             'runcode'     : runcode,
                                             'detail'      : detail } },
                          dict)

        self.write('''  </SOAP:Body>
</SOAP:Envelope>
''')
        self.m_finish(dict)

    def m_init(self, dict):
        self.write('<?xml version="1.0"?>')

    def m_finish(self, dict):
        self.write("\n\f\n")

    def persistent_id(self, object):
        return None

    def m_reference(self, object, dict):
        """ already done """

    def m_None(self, object, dict):
        """ already done """

    def m_int(self, object, dict):
        self.write(str(object))

    def m_long(self, object, dict):
        self.write(str(object))

    def m_float(self, object, dict):
        self.write(str(object))

    def m_complex(self, object, dict):
        self.write(str(object))

    def m_string(self, object, dict):
        self.write(str(object))

    def m_list(self, object, dict):
        # FIXME missing ref ids
	n = len(object)
	for k in range(n):
            type = self._type(object[k])
            self.write('<tbd:urtype' + type + '>')
	    self._marshal(object[k], dict)
            self.write('</tbd:urtype>')

    def m_tuple(self, object, dict):
        # FIXME missing ref ids
        # FIXME set type to tuple
	n = len(object)
	for k in range(n):
            type = self._type(object[k])
            self.write('<tbd:urtype' + type + '>')
	    self._marshal(object[k], dict)
            self.write('</tbd:urtype>')

    def m_dictionary(self, object, dict):
        # FIXME missing ref ids
	items = object.items()
	n = len(items)
	for k in range(n):
	    key, value = items[k]
            type = self._type(value)
            self.write('<' + key + type + '>')
	    self._marshal(value, dict)
            self.write('</' + key + '>')

    def m_instance(self, object, dict):
        # FIXME missing ref ids
        cls = object.__class__
	module = whichmodule(cls)
	name = cls.__name__
	try:
	    getstate = object.__getstate__
	except AttributeError:
	    stuff = object.__dict__
        else:
	    stuff = getstate()
	items = stuff.items()
	n = len(items)
	for k in range(n):
	    key, value = items[k]
            type = self._type(value)
            self.write('<' + key + type + '>')
	    self._marshal(value, dict)
            self.write('</' + key + '>')

    def _type(self, object):
        t = type(object)
        if t == ListType:
            return ' SOAP:arrayType="tbd:urtype[]"'
        elif t == TupleType:
            return ' SOAP:arrayType="tbd:urtype[]"'
        elif t == NoneType:
            return ' xsi:null="1"'

        return ''

class SOAPUnmarshaler(Unmarshaler, saxlib.DocumentHandler):

    def __init__(self, stream):
        self.memo = {}
        self.stream = stream

    def _unmarshal(self):
        self.parse_value_stack = [ {} ]
        self.parse_utype_stack = [ DICT ]
        self.parse_type_stack = [ ]

        self.parser = saxexts.make_parser()
        self.parser.setDocumentHandler(self)
        self.parser.setErrorHandler(self)
        lines = []
        stream = self.stream
        # FIXME SAX parsers should support this on streams
        line = stream.readline()
        while (line != "\f\n") and (line != ""):
            lines.append(line)
            line = stream.readline()
        if len(lines) == 0:
            raise EOFError
        stream = StringIO(string.join(lines))
        self.parser.parseFile(stream)
        o = self.parse_value_stack[0]
        delattr(self, 'parse_value_stack')
        self.parser.close()
        return o

    def startElement(self, name, attrs):
        self.chars = ""

        xsi_type = None
        if attrs.has_key('xsi:type'):
            xsi_type = attrs['xsi:type']
        elif (attrs.has_key('xsi:null')
              and attrs['xsi:null'] == '1'):
            xsi_type = "None"
        self.parse_type_stack.append(xsi_type)

        # FIXME 'list' is a temp hack for a specific user
        if (attrs.has_key('SOAP:arrayType')
            or (attrs.has_key('type')
                and attrs['type'] == 'list')):
            self.parse_utype_stack.append(ARRAY)
            self.parse_value_stack.append( [ ] )
        else:
            # will be set to DICT if a sub-element is found
            self.parse_utype_stack.append(CHAR)

    def endElement(self, name):
        # FIXME do something with types
        xsi_type = self.parse_type_stack.pop()
        utype = self.parse_utype_stack.pop()

        if utype is CHAR:
            if xsi_type == "None":
                value = None
            else:
                value = self.chars
        else:
            value = self.parse_value_stack.pop()

        # if we're in an element, and our parent element was defaulted
        # to CHAR, then we're in a struct and we need to create that
        # dictionary.
        if self.parse_utype_stack[-1] is CHAR:
            self.parse_value_stack.append( { } )
            self.parse_utype_stack[-1] = DICT

        if self.parse_utype_stack[-1] is DICT:
            self.parse_value_stack[-1][name] = value
        else:
            self.parse_value_stack[-1].append(value)

    def characters(self, ch, start, length):
        self.chars = self.chars + ch[start:start + length]

    def fatalError(self, exc):
        raise exc

# Shorthands (credits to and most copied from pickle.py)

def encode_call(file, method, object):
    SOAPMarshaler(file).encode_call(method, object)

def encode_calls(method, object):
    file = StringIO()
    SOAPMarshaler(file).encode_call(method, object)
    return file.getvalue()

def encode_response(file, method, object):
    SOAPMarshaler(file).encode_response(method, object)

def encode_responses(method, object):
    file = StringIO()
    SOAPMarshaler(file).encode_response(method, object)
    return file.getvalue()

def encode_fault(file, faultcode, message, runcode, detail = None):
    SOAPMarshaler(file).encode_fault(faultcode, message, runcode, detail)

def encode_faults(faultcode, message, runcode, detail = None):
    file = StringIO()
    SOAPMarshaler(file).encode_fault(faultcode, message, runcode, detail)
    return file.getvalue()

def dump(object, file):
    SOAPMarshaler(file).dump(object)

def dumps(object):
    file = StringIO()
    SOAPMarshaler(file).dump(object)
    return file.getvalue()

def decode(file):
    return SOAPUnmarshaler(file).decode()

def decodes(str):
    file = StringIO(str)
    return SOAPUnmarshaler(file).decode()

def load(file):
    return SOAPUnmarshaler(file).load()

def loads(str):
    file = StringIO(str)
    return SOAPUnmarshaler(file).load()

if __name__ == '__main__':
    runtests(load, loads, dump, dumps)
