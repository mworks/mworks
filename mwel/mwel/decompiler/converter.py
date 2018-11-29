from __future__ import division, print_function, unicode_literals
from collections import OrderedDict

from .. import ast
from ..parser import ExpressionParser


class ExpressionListParser(ExpressionParser):

    def start(self):
        return self.expr_list()


class Comment(object):
    pass


class Assignment(object):
    pass


class Declaration(object):
    pass


class Converter(object):

    def __init__(self, error_logger):
        self.error_logger = error_logger

        self._expr_error_count = 0
        def expr_error_logger(*args, **kwargs):
            self._expr_error_count += 1
        self._expr_parser = ExpressionParser(expr_error_logger)
        self._expr_list_parser = ExpressionListParser(expr_error_logger)

    def convert(self, root):
        items = []

        for elem in root.children:
            if elem.tag is elem.comment_tag:
                item = self._convert_comment(elem)
            else:
                item = self._convert_declaration(elem)
            items.append(item)

        return items

    def _convert_comment(self, elem):
        item = Comment()
        item.text = elem.text
        return item

    def _convert_declaration(self, elem):
        item = Declaration()
        item.signature = elem.tag
        item.tag = None
        item.value = None
        item.params = OrderedDict()

        attrib = elem.attrib.copy()

        if ('type' in attrib) and (item.signature != 'variable' or
                                   attrib['type'] == 'selection'):
            # Lowercase the subtype, to ensure that the type info (if any)
            # can be found in components.json
            item.signature += '/' + attrib.pop('type').lower()

        if 'tag' in attrib:
            item.tag = self._convert_value(attrib.pop('tag'),
                                           ast.IdentifierExpr)

        if item.signature == 'variable':
            self._cleanup_variable_params(item, attrib)

        for key, value in attrib.items():
            item.params[key] = self._convert_value(value, ast.ExprList)

        item.children = self.convert(elem)

        if (item.signature == 'action/assignment' and
            len(item.params) == 2 and
            not item.children):
            variable = attrib.get('variable')
            if variable and self._is_valid_assignment_target(variable):
                value = attrib.get('value')
                if value and self._is_valid_expr(value, ast.Expr):
                    item = Assignment()
                    item.variable = variable
                    item.value = value

        return item

    def _convert_value(self, value, valid_expr_type):
        if self._is_valid_expr(value, valid_expr_type):
            return value
        return self._quote_value(value)

    def _quote_value(self, value):
        if "'" in value:
            if '"' not in value:
                return '"%s"' % value
            value = value.replace("'", "\\'")
        return "'%s'" % value

    def _cleanup_variable_params(self, item, attrib):
        for name in ('logging', 'scope'):
            value = attrib.get(name)
            if value:
                attrib[name] = value.lower()

        for name in ('exclude_from_data_file', 'persistant'):
            if attrib.get(name, '').lower() in ('no', 'false', '0'):
                del attrib[name]

        for name in ('description', 'editable', 'viewable'):
            attrib.pop(name, None)

        subtype = attrib.get('type', '').lower()
        default_value = attrib.get('default_value')

        if not default_value:
            return
        elif subtype == 'string':
            default_value = self._quote_value(default_value)
        elif subtype == 'list':
            if not self._is_valid_expr(default_value, ast.ExprList):
                return
            default_value = '[%s]' % default_value
        elif not self._is_valid_expr(default_value, ast.Expr):
            return
        elif (not subtype) or (subtype == 'any'):
            # No cast needed.  Use default value as is.
            pass
        elif subtype == 'integer':
            if (not self._is_valid_expr(default_value, ast.NumberLiteralExpr) or
                set(default_value) & {'.', 'e', 'E'}):
                default_value = '(int)(%s)' % default_value
        elif subtype in ('float', 'double'):
            if not (self._is_valid_expr(default_value, ast.NumberLiteralExpr) and
                    (set(default_value) & {'.', 'e', 'E'})):
                default_value = '(float)(%s)' % default_value
        elif subtype == 'boolean':
            if default_value.lower() not in ('true', 'false', 'yes', 'no'):
                default_value = '(bool)(%s)' % default_value
        else:
            # Unknown type
            return

        item.value = default_value
        attrib.pop('type', None)  # Might not be there
        del attrib['default_value']

    def _is_valid_expr(self, expr, valid_expr_type):
        self._expr_error_count = 0
        if valid_expr_type is ast.ExprList:
            result = self._expr_list_parser.parse(expr)
        else:
            result = self._expr_parser.parse(expr)
        if self._expr_error_count:
            return False
        return isinstance(result, valid_expr_type)

    def _is_valid_assignment_target(self, expr):
        self._expr_error_count = 0
        target = self._expr_parser.parse(expr)
        if self._expr_error_count:
            return False
        while isinstance(target, ast.SubscriptExpr):
            target = target.target
        return isinstance(target, ast.IdentifierExpr)
