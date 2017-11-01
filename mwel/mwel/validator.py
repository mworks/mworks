from __future__ import division, print_function, unicode_literals

from . import get_component_info
from .analyzer import Component


class Validator(object):

    # Only type signatures with one of the following roots are handled in
    # MWParserTransformation.xsl.  Component declarations with any other
    # signature root are just stripped out by the XSLT transformation, which
    # can lead to confusing errors.  Therefore, we insist that all component
    # declarations have a type signature that starts with one of these roots.
    _valid_signature_roots = (
        'action',
        'bias_monitor',
        'block',
        'calibrator',
        'experiment',
        'filter',
        'folder',
        'iochannel',
        'iodevice',
        'list',
        'list_replicator',
        'protocol',
        'range_replicator',
        'resource',
        'sound',
        'staircase',
        'stimulus',
        'stimulus_group',
        'task_system',
        'task_system_state',
        'transition',
        'trial',
        'variable',
        'variable_assignment',
        )

    def __init__(self, error_logger):
        self.error_logger = error_logger
        self._component_info = dict((info['signature'], info)
                                    for info in get_component_info().values()
                                    if not info.get('abstract', False))
        self._parent_infos = []

    def validate(self, cmpts):
        experiment = None
        protocols = []
        others = []

        for c in cmpts:
            if c.name == 'experiment':
                if not experiment:
                    experiment = c
                else:
                    self.error_logger("Experiment cannot contain more than "
                                      "one 'experiment' component",
                                      lineno = c.lineno,
                                      colno = c.colno,
                                      filename = c.filename)
            elif c.name == 'protocol':
                protocols.append(c)
            else:
                others.append(c)

            self._validate(c)

        if not experiment:
            experiment = Component(name='experiment', tag='Experiment')

        if not (protocols or experiment.children):
            protocols.append(Component(name='protocol', tag='Empty Protocol'))
        experiment.children.extend(protocols)

        return others + [experiment]

    def _validate(self, c):
        signature = c.name + ('/' + c.type if c.type else '')
        if c.name not in self._valid_signature_roots:
            self.error_logger("'%s' is not a valid component type signature" %
                              signature,
                              lineno = c.lineno,
                              colno = c.colno,
                              filename = c.filename)

        info = self._component_info.get(signature, {})
        if info:
            for parent_info in reversed(self._parent_infos):
                if not parent_info:
                    # We don't have any info for the component's parent, so we
                    # can't validate its placement
                    break
                if not any ((n in parent_info['allowed_child'])
                            for n in info['isa']):
                    self.error_logger("Component '%s' is not allowed inside "
                                      "component '%s'" %
                                      (signature, parent_info['signature']),
                                      lineno = c.lineno,
                                      colno = c.colno,
                                      filename = c.filename)
                    break
                if not parent_info.get('transient', False):
                    break
            else:
                if signature == 'action/assignment':
                    # Convert top-level action/assignment into
                    # variable_assignment
                    c.name = 'variable_assignment'
                    c.type = 'any'
                elif not (signature == 'protocol' or
                          info.get('toplevel', False)):
                    # Protocols aren't really allowed at the top level, but we
                    # accept them there out of convenience, and move them in to
                    # the experiment component before completing validation
                    self.error_logger("Component '%s' is not allowed at "
                                      "the top level" % signature,
                                      lineno = c.lineno,
                                      colno = c.colno,
                                      filename = c.filename)
        else:
            # We don't have any info for the given component, so we can't
            # validate its placement
            pass

        self._parent_infos.append(info)
        try:
            for child in c.children:
                self._validate(child)
        finally:
            self._parent_infos.pop()
