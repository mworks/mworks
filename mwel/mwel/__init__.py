from __future__ import division, print_function, unicode_literals
import collections
from contextlib import contextmanager
import json
import os
import sys
import xml.etree.ElementTree as ET


def get_component_info():
    try:
        with open('/Library/Application Support/MWorks/Documentation/'
                  'components.json') as fp:
            return json.load(fp)
    except IOError:
        return {}


ErrorInfo = collections.namedtuple('ErrorInfo',
                                   'msg token lineno colno filename')


class ErrorLogger(object):

    def __init__(self):
        self.errors = collections.deque()
        self._filenames = ['']

    def filename(self, name):
        @contextmanager
        def cm():
            self._filenames.append(name)
            try:
                yield
            finally:
                self._filenames.pop()
        return cm()

    @property
    def current_filename(self):
        return self._filenames[-1]

    def __call__(self, msg, token=None, lineno=None, colno=None, filename=''):
        info = ErrorInfo(msg, token, lineno, colno,
                         filename or self.current_filename)
        self.errors.append(info)

    def print_errors(self, file):
        # Sort errors by filename, lineno, and colno.  For errors with a
        # backtrace, sort by the last location in the trace (i.e. the location
        # of the initial, provoking expression or statement).
        for e in sorted(self.errors, key=(lambda e: (
                            self._ensure_sortable(e.filename),
                            self._ensure_sortable(e.lineno),
                            self._ensure_sortable(e.colno),
                            ))):
            msg = e.msg
            location = self.format_location(e.lineno, e.colno, e.filename)
            if location:
                msg += ' [%s]' % location
            print(msg, file=file)

    @staticmethod
    def _ensure_sortable(value):
        if value is None:
            return sys.maxsize
        if isinstance(value, tuple):
            return value[-1]
        return value

    @classmethod
    def format_location(cls, lineno=None, colno=None, filename=''):
        if isinstance(lineno, tuple):
            location_iter = zip(lineno, colno, filename)
        else:
            location_iter = ((lineno, colno, filename),)
        return '; via '.join(cls._format_location(l, c, f) for l, c, f in
                             location_iter)

    @staticmethod
    def _format_location(lineno, colno, filename):
        location = (os.path.basename(filename) if filename else '')
        if (lineno and lineno > 0) and (colno and colno > 0):
            prefix = (': ' if location else '')
            location += '%sline %d, column %d' % (prefix, lineno, colno)
        return location


def readfile(filepath, error_logger, lineno=None, colno=None, decode=True):
    try:
        with open(filepath, 'rb') as fp:
            data = fp.read()
        if decode:
            data = data.decode('utf-8')
        return data
    except IOError as e:
        error_logger(("Failed to open file '%s': %s" %
                      (filepath, e.strerror)),
                     lineno = lineno,
                     colno = colno)
    except UnicodeDecodeError:
        error_logger(("File '%s' does not contain valid UTF-8 encoded text" %
                      filepath),
                     lineno = lineno,
                     colno = colno)


def toxml(argv=sys.argv, stdout=sys.stdout, stderr=sys.stderr):
    from .analyzer import Analyzer
    from .generator import XMLGenerator
    from .parser import Parser
    from .validator import Validator

    omit_metadata = False
    if len(argv) == 2:
        filepath = argv[1]
    elif len(argv) == 3 and argv[1] == '--omit-metadata':
        filepath = argv[2]
        omit_metadata = True
    else:
        print('Usage: %s [--omit-metadata] file' % os.path.basename(argv[0]),
              file = stderr)
        return 2

    error_logger = ErrorLogger()

    src = readfile(filepath, error_logger)
    if src is not None:
        parser = Parser(error_logger)
        ast = parser.parse(src, os.path.dirname(filepath))
        if ast:
            cmpts = Analyzer(error_logger).analyze(ast)
            cmpts = Validator(error_logger).validate(cmpts)

    if error_logger.errors:
        error_logger.print_errors(stderr)
        return 1

    generator = XMLGenerator()
    root = generator.generate(cmpts, omit_metadata)

    if not omit_metadata:
        # Store the sources for the experiment and all included files in an
        # assignment to #loadedExperiment
        sources = collections.OrderedDict()
        sources[filepath] = src
        sources.update(parser.included_files)
        va_node = ET.SubElement(root,
                                'variable_assignment',
                                variable = '#loadedExperiment')
        dict_node = ET.SubElement(va_node, 'dictionary')
        for path, src in sources.items():
            dict_elem_node = ET.SubElement(dict_node, 'dictionary_element')
            ET.SubElement(dict_elem_node, 'key').text = path
            ET.SubElement(dict_elem_node, 'value', type='string').text = src

    generator.format(root)
    try:
        output_stream = stdout.buffer
    except AttributeError:
        # Python 2.7
        output_stream = stdout
    generator.write(root, output_stream)

    return 0


def fromxml(argv=sys.argv, stdout=sys.stdout, stderr=sys.stderr):
    from .decompiler.beautifier import Beautifier
    from .decompiler.converter import Converter
    from .decompiler.outputter import Outputter
    from .decompiler.simplifier import Simplifier
    from .decompiler.xmlparser import XMLParser

    if len(argv) != 2:
        print('Usage: %s file' % os.path.basename(argv[0]), file=stderr)
        return 2

    filepath = argv[1]
    error_logger = ErrorLogger()

    src_data = readfile(filepath, error_logger, decode=False)
    if src_data is not None:
        root = XMLParser(error_logger).parse(src_data)
        if root:
            Simplifier(error_logger).simplify(root)
            items = Converter(error_logger).convert(root)
            Beautifier(error_logger).beautify(items)
            Outputter(error_logger).output(items, stdout)

    if error_logger.errors:
        error_logger.print_errors(stderr)
        return 1

    return 0
