from __future__ import division, print_function, unicode_literals
import xml.etree.ElementTree as ET

from . import ErrorLogger


class XMLGenerator(object):

    def __init__(self, newline='\n', tab='  '):
        self.newline = newline
        self.tab = tab

    def generate(self, cmpts, omit_metadata=False):
        root = ET.Element('monkeyml', version='1.0')
        for c in cmpts:
            self._convert(c, root, omit_metadata)
        return root

    def _convert(self, cmpt, parent_node, omit_metadata):
        params = cmpt.params.copy()
        if cmpt.type:
            params['type'] = cmpt.type
        if cmpt.tag:
            params['tag'] = cmpt.tag

        if not omit_metadata:
            location = ErrorLogger.format_location(cmpt.lineno,
                                                   cmpt.colno,
                                                   cmpt.filename)
            if location:
                params['_location'] = location

        node = ET.SubElement(parent_node, cmpt.name, params)

        for child in cmpt.children:
            self._convert(child, node, omit_metadata)

    def format(self, node, depth=0, last_at_depth=True):
        children = list(node)
        if children:
            if not node.text:
                node.text = self.newline + (self.tab * (depth + 1))
            for c in children[:-1]:
                self.format(c, depth + 1, False)
            self.format(children[-1], depth + 1)
        if not node.tail:
            node.tail = (self.newline +
                         (self.tab * (depth - 1 if last_at_depth else depth)))

    def write(self, root, output_stream):
        tree = ET.ElementTree(root)
        tree.write(output_stream, encoding=str('UTF-8'), xml_declaration=True)
