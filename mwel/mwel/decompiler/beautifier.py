from __future__ import division, print_function, unicode_literals
from collections import defaultdict, OrderedDict

from .converter import Declaration
from .. import get_component_info


class Beautifier(object):

    _preferred_aliases = {
        'folder': 'group',
        'task_system': 'task',
        'task_system_state': 'state',
        'variable': 'var',
        }

    def __init__(self, error_logger):
        self.error_logger = error_logger

        self._params = {}
        for info in get_component_info().values():
            if not info.get('abstract', False):
                signature = info['signature']
                params = dict((p['name'], p)
                              for p in info.get('parameters', []))
                self._params[signature] = params
                for alias in info.get('alias', []) + info.get('mwel_alias', []):
                    self._params[alias] = params

        short_type_counts = defaultdict(lambda: 0)
        for signature in self._params:
            short_type_counts[signature.split('/')[-1]] += 1
        self._unique_short_types = set(k for k, v in short_type_counts.items()
                                       if v == 1)

        self._single_param_names = {}
        self._default_param_values = {}
        for signature, params in self._params.items():
            required_params = [p for p in params.values()
                               if p.get('required', False)]
            if len(required_params) == 1:
                self._single_param_names[signature] = required_params[0]['name']

            # Some defaults are stored as int's or other non-string types, so
            # convert them to strings for comparison with parameter values
            defaults = dict((p['name'], type('')(p['default']))
                            for p in params.values() if 'default' in p)
            if defaults:
                self._default_param_values[signature] = defaults

    def beautify(self, items):
        for item in items:
            if isinstance(item, Declaration):
                # We can only beautify known component types
                if item.signature in self._params:
                    self._convert_transition_to_goto(item)
                    self._remove_default_valued_parameters(item)
                    self._remove_inferrable_parameter_names(item)
                    self._substitute_preferred_alias(item)
                    self._shorten_type_name(item)

                self.beautify(item.children)

    def _convert_transition_to_goto(self, item):
        if item.signature == 'transition/conditional':
            if set(item.params) == {'condition', 'target'}:
                item.signature = 'transition/goto'
                params = OrderedDict()
                params['target'] = item.params['target']
                params['when'] = item.params['condition']
                item.params = params
        elif item.signature == 'transition/direct':
            if set(item.params) == {'target'}:
                item.signature = 'transition/goto'
                params = OrderedDict()
                params['target'] = item.params['target']
                item.params = params

    def _remove_default_valued_parameters(self, item):
        # NOTE: This isn't the best place to do this, as the converter has
        # already quoted values that aren't valid expressions. However, the odds
        # of a default value *not* being a valid expression are very low, so it
        # probably doesn't matter.
        for name, value in tuple(item.params.items()):
            defaults = self._default_param_values.get(item.signature, {})
            if defaults.get(name, '') == value:
                del item.params[name]

    def _remove_inferrable_parameter_names(self, item):
        if len(item.params) == 1:
            single_param_name = self._single_param_names.get(item.signature)
            if list(item.params)[0] == single_param_name:
                item.params = item.params[single_param_name]

    def _substitute_preferred_alias(self, item):
        alias = self._preferred_aliases.get(item.signature)
        if alias:
            item.signature = alias

    def _shorten_type_name(self, item):
        short_type = item.signature.split('/')[-1]
        if short_type in self._unique_short_types:
            item.signature = short_type
