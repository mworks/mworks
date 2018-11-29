from __future__ import division, print_function, unicode_literals
from collections import OrderedDict
from xml.etree import ElementTree
from xml.parsers import expat


class XMLElement(object):

    comment_tag = object()

    text = None
    tail = None

    lineno = None
    colno = None

    def __init__(self, tag, attrib):
        self.tag = tag
        self.attrib = attrib
        self.children = []

    def append(self, subelement):
        self.children.append(subelement)


class XMLParser(object):

    def __init__(self, error_logger):
        self.error_logger = error_logger

    def parse(self, src_data):
        tb = ElementTree.TreeBuilder(XMLElement)

        def xml_decl(version, encoding, standalone):
            pass

        def start_element(name, attrs):
            attrs = OrderedDict(attrs[i:i+2] for i in range(0, len(attrs), 2))
            elem = tb.start(name, attrs)
            elem.lineno = p.CurrentLineNumber
            elem.colno = p.CurrentColumnNumber

        def end_element(name):
            tb.end(name)

        def comment(data):
            tag = XMLElement.comment_tag
            start_element(tag, ())
            tb.data(data)
            end_element(tag)

        def default(data):
            if data.strip():
                self.error_logger('XML contains unexpected data',
                                  lineno = p.CurrentLineNumber,
                                  colno = p.CurrentColumnNumber)

        p = expat.ParserCreate()

        p.XmlDeclHandler = xml_decl
        p.StartElementHandler = start_element
        p.EndElementHandler = end_element
        p.CommentHandler = comment
        p.DefaultHandlerExpand = default

        p.buffer_text = True
        p.ordered_attributes = True

        try:
            p.Parse(src_data, True)
            root = tb.close()
            if root.tag == 'monkeyml':
                return root
            self.error_logger('XML does not contain an MWorks experiment '
                              '(root element is %s, not monkeyml)' % root.tag)
        except expat.ExpatError:
            self.error_logger(('Failed to parse XML: %s' %
                               expat.ErrorString(p.ErrorCode)),
                              lineno = p.ErrorLineNumber,
                              colno = p.ErrorColumnNumber)
