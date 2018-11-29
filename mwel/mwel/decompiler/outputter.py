from __future__ import division, print_function, unicode_literals

from .converter import Comment, Assignment


class Outputter(object):

    _groups = {
        'io_devices': 'I/O Devices',
        'variables': 'Variables',
        'sounds': 'Sounds',
        'stimuli': 'Stimuli',
        'filters': 'Filters',
        'optimizers': 'Optimizers',
        'calibrators': 'Calibrators',
        'resources': 'Resources',
        'experiment': 'Protocols',
        }

    _tab = ' ' * 4

    def __init__(self, error_logger):
        self.error_logger = error_logger

    def output(self, items, output_stream, indent=0):
        for item in items:
            if isinstance(item, Comment):
                self._output_comment(item, output_stream, indent)
            elif isinstance(item, Assignment):
                self._output_assignment(item, output_stream, indent)
            elif item.signature in self._groups:
                self._output_group(item, output_stream, indent)
            else:
                self._output_declaration(item, output_stream, indent)

    def _output_comment(self, item, output_stream, indent):
        text = item.text
        if '\n' not in text:
            # Single-line comment
            line = '//' + text
        else:
            # Multi-line comment
            line = '/*' + text + '*/'
        self._output_line(line, output_stream, indent)

    def _output_assignment(self, item, output_stream, indent):
        self._output_line('%s = %s' % (item.variable, item.value),
                          output_stream,
                          indent)

    def _output_group(self, item, output_stream, indent):
        self._output_line('', output_stream, 0)
        self._output_line('//', output_stream, indent)
        self._output_line('// ' + self._groups[item.signature],
                          output_stream,
                          indent)
        self._output_line('//', output_stream, indent)
        self._output_line('', output_stream, 0)

        self.output(item.children, output_stream, indent)

    def _output_declaration(self, item, output_stream, indent):
        line = item.signature
        if item.tag:
            line += ' ' + item.tag
        if item.value:
            line += ' = ' + item.value
        if not (item.value or item.params or item.children):
            line += ' ()'
        else:
            if item.params:
                if isinstance(item.params, type('')):
                    # Single param, name omitted
                    line += ' (%s)' % item.params
                elif len(item.params) == 1:
                    # Single param, name included
                    line += ' (%s = %s)' % tuple(item.params.items())[0]
                else:
                    # Multiple params
                    self._output_line(line + ' (', output_stream, indent)
                    for name_value in item.params.items():
                        self._output_line('%s = %s' % name_value,
                                          output_stream,
                                          indent+1)
                    line = self._tab + ')'
            if item.children:
                self._output_line(line + ' {', output_stream, indent)
                self.output(item.children, output_stream, indent+1)
                line = '}'
        self._output_line(line, output_stream, indent)

    def _output_line(self, text, output_stream, indent):
        output_stream.write(self._tab * indent)
        output_stream.write(text)
        output_stream.write('\n')
