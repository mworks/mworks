"""  $Id: LDOXML.py,v 1.3 2000/03/10 23:07:52 kmacleod Exp $

LDOXML.py implements the ``Self-Describing XML Data
Representation'' (XML Serialization) at <http://casbah.org/Scarab/>.

"""

from StringIO import StringIO
from ScarabMarshal import *
from types import *
from xml.sax import saxlib, saxexts
import string
import base64

class LDOXMLMarshaler(Marshaler):

    def __init__(self, stream):
        self.written_stream_header = 0
	self.write = stream.write
        self.flush = stream.flush

    def m_init(self, dict):
        self.write('<?xml version="1.0"?>')

    def m_finish(self, dict):
        self.write("\n\f\n")

    def persistent_id(self, object):
        return None

    def m_reference(self, object, dict):
        self.write('<ref ref="' + dict[str(id(object))] + '"/>')

    def m_None(self, object, dict):
        self.write('<atom type="Null"/>')

    def m_int(self, object, dict):
        self.write('<atom>' + str(object) + '</atom>')

    def m_long(self, object, dict):
        self.write('<atom>' + str(object) + '</atom>')

    def m_float(self, object, dict):
        self.write('<atom>' + str(object) + '</atom>')

    def m_complex(self, object, dict):
        self.write('<atom>' + str(object) + '</atom>')

    def m_string(self, object, dict):
        self.write('<atom>' + str(object) + '</atom>')

    def m_list(self, object, dict):
        # FIXME missing ref ids
        self.write('<list>')
	n = len(object)
	for k in range(n):
	    self._marshal(object[k], dict)
        self.write('</list>')

    def m_tuple(self, object, dict):
        # FIXME missing ref ids
        # FIXME set type to tuple
        self.write('<list>')
	n = len(object)
	for k in range(n):
	    self._marshal(object[k], dict)
        self.write('</list>')

    def m_dictionary(self, object, dict):
        # FIXME missing ref ids
        self.write('<dictionary>')
	items = object.items()
	n = len(items)
	for k in range(n):
	    key, value = items[k]
	    self._marshal(key, dict)
	    self._marshal(value, dict)
        self.write('</dictionary>')

    def m_instance(self, object, dict):
        # FIXME missing ref ids
        cls = object.__class__
	module = whichmodule(cls)
	name = cls.__name__
        # FIXME support LDO-Types
        # FIXME support Python-Types
        self.write('<dictionary>')
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
	    self._marshal(key, dict)
	    self._marshal(value, dict)
        self.write('</dictionary>')

class LDOXMLUnmarshaler(Unmarshaler, saxlib.DocumentHandler):

    def __init__(self, stream):
        self.memo = {}
        self.stream = stream

    def um_init(self):
        return

    def _unmarshal(self):
        self.parse_value_stack = [[]]
        self.parse_type_stack = []
        self.parse_keep_text = 0
        self.parse_base64 = 0

        self.parser = saxexts.make_parser()
        self.parser.setDocumentHandler(self)
        self.parser.setErrorHandler(self)
        lines = []
        stream = self.stream
        line = stream.readline()
        while (line != "\f\n") and (line != ""):
            lines.append(line)
            line = stream.readline()
        if len(lines) == 0:
            raise EOFError
        stream = StringIO(string.join(lines))
        self.parser.parseFile(stream)
        self.parser.close()

        return self.parse_value_stack[0][0]

    def startElement(self, name, attrs):

        if name == 'atom':
            self.parse_keep_text = 1
            value = ""
            if attrs.getValue('encoding') == 'base64':
                self.parse_base64 = 1
        elif name == 'list' or name == 'dictionary':
            value = []

        # FIXME elif name == 'ref'

        self.parse_value_stack.append(value)
        self.parse_type_stack.append(attrs.getValue('type'))

    def endElement(self, name):
        value = self.parse_value_stack.pop()
        type = self.parse_type_stack.pop()
        
        if name == 'atom':
            item = value
            self.parse_keep_text = 0
            if self.parse_base64:
                self.parse_base64 = 0
                item = base64.decodestring(item)
        elif name == 'list':
            item = value
        elif name == 'dictionary':
            item = {}
            # FIXME is there a better way to copy a list to a dict?
            while len(value) > 0:
                k, v = value[0:2]
                del value[0:2]
                item[k] = v
        elif name == 'ref':
            item = self.parse_saved_item

        # FIXME set type

        self.parse_value_stack[-1].append(item)

    def characters(self, ch, start, length):
        if self.parse_keep_text:
            self.parse_value_stack[-1] = self.parse_value_stack[-1] + ch[start:start+length]

    def fatalError(self, exc):
        raise exc

# Shorthands (credits to and copied from pickle.py)

def dump(object, file):
    LDOXMLMarshaler(file).dump(object)

def dumps(object):
    file = StringIO()
    LDOXMLMarshaler(file).dump(object)
    return file.getvalue()

def load(file):
    return LDOXMLUnmarshaler(file).load()

def loads(str):
    file = StringIO(str)
    return LDOXMLUnmarshaler(file).load()

if __name__ == '__main__':
    runtests(load, loads, dump, dumps)
