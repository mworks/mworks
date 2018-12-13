from __future__ import division, print_function, unicode_literals
import collections

from . import get_component_info
from . import ast


class ExpressionAnalyzer(object):

    def __init__(self, error_logger):
        self.error_logger = error_logger

    def analyze(self, expr):
        return self._expr(expr)

    def _expr(self, expr):
        if isinstance(expr, (ast.IdentifierExpr,
                             ast.NumberLiteralExpr,
                             ast.StringLiteralExpr)):
            return expr.value

        if isinstance(expr, ast.ListLiteralExpr):
            return '[%s]' % self._exprlist_items(expr.items)

        if isinstance(expr, ast.DictLiteralExpr):
            return ('{%s}' %
                    ', '.join('%s: %s' % (self._expr(key), self._expr(value))
                              for (key, value) in expr.items))

        if isinstance(expr, ast.FunctionCallExpr):
            return ('%s(%s)' % (expr.name, self._exprlist_items(expr.args)))

        if isinstance(expr, ast.ParentheticExpr):
            return '(%s)' % self._expr(expr.expr)

        if isinstance(expr, ast.UnitsExpr):
            return '%s %s' % (self._expr(expr.target), expr.unit)

        if isinstance(expr, ast.SubscriptExpr):
            return '%s[%s]' % (self._expr(expr.target), self._expr(expr.value))

        if isinstance(expr, ast.UnaryOpExpr):
            return '%s%s' % (('not ' if expr.op == 'not' else expr.op),
                             self._expr(expr.operand))

        if isinstance(expr, ast.CastExpr):
            return '(%s)%s' % (expr.totype, self._expr(expr.target))

        if isinstance(expr, ast.BinaryOpExpr):
            return '%s %s %s' % (self._expr(expr.operands[0]),
                                 expr.op,
                                 self._expr(expr.operands[1]))

        raise NotImplementedError

    def _exprlist_items(self, items):
        return ', '.join((self._range_expr(e) if isinstance(e, ast.RangeExpr)
                          else self._expr(e)) for e in items)

    def _range_expr(self, expr):
        return '%s:%s%s' % (self._expr(expr.start),
                            self._expr(expr.stop),
                            (':' + self._expr(expr.step) if expr.step else ''))


class StringLiteralExprValue(type('')):
    pass


class NonatomicExpandedExprValue(type('')):
    pass


class Component(object):

    def __init__(self, lineno=-1, colno=-1, filename='',
                 name=None, type=None, tag=None, params={}):
        self.lineno = lineno
        self.colno = colno
        self.filename = filename
        self.name = name
        self.type = type
        self.tag = tag
        self.params = params
        self.children = []

    def add_backtrace(self, lineno, colno, filename):
        if isinstance(self.lineno, tuple):
            self.lineno += (lineno,)
        else:
            self.lineno = (self.lineno, lineno)

        if isinstance(self.colno, tuple):
            self.colno += (colno,)
        else:
            self.colno = (self.colno, colno)

        if isinstance(self.filename, tuple):
            self.filename += (filename,)
        else:
            self.filename = (self.filename, filename)


class Analyzer(ExpressionAnalyzer):

    def __init__(self, error_logger):
        super(Analyzer, self).__init__(error_logger)

        self._full_typenames = set()
        self._aliases = {}
        self._short_typenames = collections.defaultdict(list)
        self._single_param_names = {}

        for info in get_component_info().values():
            if not info.get('abstract', False):
                signature = info['signature']
                self._register_signature(signature)
                for alias in info.get('alias', []) + info.get('mwel_alias', []):
                    self._aliases[alias] = signature
                    self._register_signature(alias)
                params = info.get('parameters', [])
                if len(params) > 1:
                    params = [p for p in params if p.get('required', False)]
                if len(params) == 1:
                    self._single_param_names[signature] = params[0]['name']

        self._macros = {}
        self._active_macros = []
        self._active_macro_arg_expansions = [{}]

    def _register_signature(self, signature):
        self._full_typenames.add(signature)
        sig_parts = signature.split('/')
        if len(sig_parts) > 1:
            self._short_typenames[sig_parts[1]].append(sig_parts[0])

    def analyze(self, module):
        cmpts = []
        with self.error_logger.filename(module.filename):
            for stmt in module.statements:
                if isinstance(stmt, ast.Module):
                    cmpts.extend(self.analyze(stmt))
                elif isinstance(stmt, ast.MacroStmt):
                    self._register_macro(stmt)
                else:
                    self._stmt(stmt, cmpts)
        return cmpts

    def _register_macro(self, macro):
        name = macro.name
        if name in self._macros:
            self.error_logger("Macro '%s' is already defined" % name,
                              lineno = macro.lineno,
                              colno = macro.colno)
        else:
            macro.filename = self.error_logger.current_filename
            self._macros[name] = macro

    def _expr(self, expr):
        if isinstance(expr, ast.IdentifierExpr):
            expansion = self._active_macro_arg_expansions[-1].get(expr.value)
            if expansion:
                return expansion

        if isinstance(expr, (ast.IdentifierExpr, ast.FunctionCallExpr)):
            expansion = self._macro_expansion(expr)
            if expansion:
                return expansion

        result = super(Analyzer, self)._expr(expr)
        if isinstance(expr, ast.StringLiteralExpr):
            result = StringLiteralExprValue(result)
        return result

    def _stmt(self, stmt, cmpts):
        if isinstance(stmt, ast.AssignmentStmt):
            cmpts.append(self._assignment_stmt(stmt))
        elif isinstance(stmt, ast.DeclarationStmt):
            if not self._macro_expansion(stmt, cmpts=cmpts):
                cmpts.append(self._decl_stmt(stmt))
        else:
            raise NotImplementedError

    def _assignment_stmt(self, stmt):
        variable = stmt.varname
        for index in stmt.indices:
            variable += '[%s]' % self._expr(index)
        value = self._expr(stmt.value)
        if stmt.op:
            value = ' '.join([variable, stmt.op, value])
        return self._component(stmt.lineno,
                               stmt.colno,
                               name = 'action',
                               type = 'assignment',
                               params = {
                                   'variable': variable,
                                   'value': value,
                                   })

    def _macro_expansion(self, node, cmpts=None):
        if isinstance(node, ast.IdentifierExpr):
            name = node.value
        elif isinstance(node, ast.FunctionCallExpr):
            name = node.name
        else:
            assert isinstance(node, ast.DeclarationStmt)
            name = node.type

        macro = self._macros.get(name)

        if not macro:
            # Not a macro.  Allow normal expansion of node to proceed.
            pass
        elif (isinstance(node, ast.Expr) and
              not isinstance(macro, ast.ExpressionMacroStmt)):
            self.error_logger(("Macro '%s' cannot be used as an "
                               "expression" % name),
                              lineno = node.lineno,
                              colno = node.colno)
        elif (isinstance(node, ast.Stmt) and
              not isinstance(macro, ast.StatementMacroStmt)):
            self.error_logger(("Macro '%s' cannot be used as a statement" %
                               name),
                              lineno = node.lineno,
                              colno = node.colno)
        elif name in self._active_macros:
            self.error_logger("Definition of macro '%s' is "
                              "self-referential" % name,
                              lineno = macro.lineno,
                              colno = macro.colno)
            # Replace the macro with None, so we know that it's already
            # been defined but don't try to expand it again
            self._macros[name] = None
        else:
            params = macro.parameters
            default_value = None
            children = []

            if isinstance(node, ast.IdentifierExpr):
                args = ()
            elif isinstance(node, ast.FunctionCallExpr):
                args = node.args
            else:
                assert isinstance(node, ast.DeclarationStmt)
                default_value = node.value
                children = node.children
                if not isinstance(node.params, dict):
                    args = (node.params or ())
                    assert len(args) <= 1
                else:
                    for p in node.params:
                        if p not in params:
                            self.error_logger(("Macro '%s' has no parameter "
                                               "'%s'") % (name, p),
                                              lineno = node.lineno,
                                              colno = node.colno)
                            return
                    args = []
                    for p in params:
                        value = node.params.get(p)
                        if value:
                            args.append(value)

            if len(args) != len(params):
                self.error_logger("Macro '%s' takes %s parameter%s" %
                                  (name,
                                   (str(len(params)) if params else 'no'),
                                   ('s' if len(params) != 1 else '')),
                                  lineno = node.lineno,
                                  colno = node.colno)
            else:
                # Need to process default value, arguments, and children here,
                # where the arguments to the enclosing macro (if any) are still
                # visible

                if default_value:
                    # Use _expr (and not _expanded_expr), because default_value
                    # is never an expression list, and it's always used as a
                    # parameter value, so it doesn't need to be parenthesized
                    default_value = self._expr(default_value)

                args = (self._expanded_expr(a) for a in args)

                expanded_children = []
                for stmt in children:
                    self._stmt(stmt, expanded_children)

                self._active_macros.append(name)
                self._active_macro_arg_expansions.append(dict(zip(params,
                                                                  args)))
                try:
                    if isinstance(macro, ast.ExpressionMacroStmt):
                        return self._expanded_expr(macro.value)
                    else:
                        assert isinstance(macro, ast.StatementMacroStmt)
                        expansion = []
                        with self.error_logger.filename(macro.filename):
                            for stmt in macro.statements:
                                self._stmt(stmt, expansion)
                        for c in expansion:
                            self._add_backtrace(c, node)
                        if node.tag or node.children:
                            if len(expansion) != 1:
                                self.error_logger(
                                    ("Macro body declares %d components, so "
                                     "invocation cannot include a tag, a "
                                     "default value with '=', or child "
                                     "components" % len(expansion)),
                                    lineno = node.lineno,
                                    colno = node.colno,
                                    )
                                return
                            c = expansion[0]
                            if node.tag:
                                if c.tag:
                                    self.error_logger(
                                        "Macro body contains a tag, so "
                                        "invocation cannot include one",
                                        lineno = node.lineno,
                                        colno = node.colno,
                                        )
                                    return
                                c.tag = node.tag
                                # Syntactically, there can't be a default value
                                # if there's not a tag, so we only check for a
                                # default value here
                                if default_value:
                                    if (c.name, c.type) != ('variable', None):
                                        self.error_logger(
                                            "Macro body does not contain a "
                                            "variable declaration, so "
                                            "invocation cannot include a "
                                            "default value with '='",
                                            lineno = node.lineno,
                                            colno = node.colno,
                                            )
                                        return
                                    c.params['default_value'] = default_value
                            if node.children:
                                if c.children:
                                    self.error_logger(
                                        "Macro body contains children, so "
                                        "invocation cannot include them",
                                        lineno = node.lineno,
                                        colno = node.colno,
                                        )
                                    return
                                c.children = expanded_children
                        cmpts.extend(expansion)
                        return True
                finally:
                    self._active_macros.pop()
                    self._active_macro_arg_expansions.pop()

    def _expanded_expr(self, expr):
        result = self._exprlist_or_expr(expr)
        if not isinstance(expr, ast.AtomicExpr):
            result = NonatomicExpandedExprValue('(%s)' % result)
        return result

    def _add_backtrace(self, cmpt, node):
        cmpt.add_backtrace(node.lineno,
                           node.colno,
                           self.error_logger.current_filename)
        for c in cmpt.children:
            self._add_backtrace(c, node)

    def _decl_stmt(self, stmt):
        typename = stmt.type
        type = typename.split('/')

        if (len(type) == 1) and (type[0] not in self._full_typenames):
            # See if we can infer a full typename
            candidates = self._short_typenames[type[0]]
            if candidates:
                if len(candidates) > 1:
                    msg = ("Multiple matches for component type '%s': %s" %
                           (typename, ', '.join("'%s/%s'" % (t, type[0])
                                                 for t in candidates)))
                    self.error_logger(msg, lineno=stmt.lineno, colno=stmt.colno)
                else:
                    type = [candidates[0], type[0]]
                    typename = '/'.join(type)

        # If type name is an alias, replace it with the "canonical" type name
        canon_name = self._aliases.get(typename)
        if canon_name:
            typename = canon_name
            type = canon_name.split('/')

        params = collections.OrderedDict()

        if stmt.value:
            if typename == 'variable':
                params['default_value'] = self._expr(stmt.value)
            else:
                self.error_logger('Only variable declarations can specify a '
                                  "default value with '='",
                                  lineno = stmt.lineno,
                                  colno = stmt.colno)

        if isinstance(stmt.params, dict):
            for key, value in stmt.params.items():
                params[key] = self._param_expr(value)
        elif stmt.params:
            # A single, unnamed parameter was given, so we need to infer its
            # name
            assert len(stmt.params) == 1
            name = self._single_param_names.get(typename)
            if not name:
                self.error_logger('Cannot infer parameter name for component '
                                  "'%s'" % typename,
                                  lineno = stmt.lineno,
                                  colno = stmt.colno)
                name = '__UNKNOWN__'
            params = {name: self._param_expr(stmt.params[0])}

        cmpt = self._component(stmt.lineno,
                               stmt.colno,
                               name = type[0],
                               type = (type[1] if len(type) > 1 else None),
                               tag = stmt.tag,
                               params = params)

        for child in stmt.children:
            self._stmt(child, cmpt.children)

        return cmpt

    def _param_expr(self, expr):
        result = self._exprlist_or_expr(expr)
        if isinstance(result, (StringLiteralExprValue,
                               NonatomicExpandedExprValue)):
            result = result[1:-1]
        return result

    def _exprlist_or_expr(self, expr):
        if isinstance(expr, ast.ExprList):
            return self._exprlist_items(expr.items)
        if isinstance(expr, ast.RangeExpr):
            return self._range_expr(expr)
        return self._expr(expr)

    def _component(self, lineno=-1, colno=-1, filename='',
                   name=None, type=None, tag=None, params={}):
        filename = filename or self.error_logger.current_filename
        return Component(lineno,
                         colno,
                         filename = filename,
                         name = name,
                         type = type,
                         tag = tag,
                         params = params)
