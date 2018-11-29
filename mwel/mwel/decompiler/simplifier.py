from __future__ import division, print_function, unicode_literals


class Simplifier(object):

    _ignored = (
        'action_marker',
        'transition_marker',
        )

    _omit_tag = (
        'action',
        'list_replicator',
        'range_replicator',
        'transition',
        )

    _ignored_attrib = (
        '_id',
        '_line_number',
        '_location',
        'full_name',
        )

    def __init__(self, error_logger):
        self.error_logger = error_logger

    def simplify(self, elem):
        self._remove_omitted_tag(elem)
        self._remove_empty_attributes(elem)
        self._remove_ignored_attributes(elem)
        self._remove_ignored_subelements(elem)

        for child in elem.children:
            self.simplify(child)

    def _remove_omitted_tag(self, elem):
        if elem.tag in self._omit_tag:
            elem.attrib.pop('tag', None)

    def _remove_empty_attributes(self, elem):
        for key, value in tuple(elem.attrib.items()):
            if not value:
                del elem.attrib[key]

    def _remove_ignored_attributes(self, elem):
        for attrib in self._ignored_attrib:
            elem.attrib.pop(attrib, None)

    def _remove_ignored_subelements(self, elem):
        for child in tuple(elem.children):
            if child.tag in self._ignored:
                elem.children.remove(child)
                if child.children:
                    self.error_logger(("Ignored element '%s' has unexpected "
                                       'children that will also be ignored' %
                                       child.tag),
                                      lineno = child.lineno,
                                      colno = child.colno)
