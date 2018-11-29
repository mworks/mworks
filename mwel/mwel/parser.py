from __future__ import division, print_function, unicode_literals
import collections
from contextlib import contextmanager
import os

from . import ast, readfile
from .lexer import Lexer


class InvalidSyntax(Exception):

    def __init__(self, msg=None, token=None, lineno=None, colno=None):
        super(InvalidSyntax, self).__init__()
        self.msg = msg
        self.token = token
        self.lineno = lineno or (token and token.lineno)
        self.colno = colno or (token and token.colno)


class TokenStream(object):

    def __init__(self, lexer, text):
        self._tokens = collections.deque()
        self._ignore_newlines = 0

        lexer.input(text)
        while True:
            t = lexer.token()
            if not t:
                break
            self._tokens.append(t)

    def _ignore(self, t):
        return self._ignore_newlines and t.type == 'NEWLINE'

    def peek(self, depth):
        count = 0
        for t in self._tokens:
            if not self._ignore(t):
                count += 1
                if count == depth:
                    return t

    def accept(self):
        while True:
            t = self._tokens.popleft()
            if not self._ignore(t):
                return t

    def ignore_newlines(self):
        @contextmanager
        def cm():
            self._ignore_newlines += 1
            yield
            self._ignore_newlines -= 1
        return cm()


class ExpressionParser(object):

    def __init__(self, error_logger):
        self.error_logger = error_logger
        self.lexer = Lexer(error_logger)
        self._token_streams = []

    @property
    def _tokens(self):
        return self._token_streams[-1]

    def peek(self, ttype=None, depth=1):
        t = self._tokens.peek(depth)
        if t and (ttype is None or t.type == ttype):
            return t

    def accept(self, ttype=None):
        if self.peek(ttype):
            self.curr = self._tokens.accept()
            return self.curr

    def accept_keyword(self, *keywords):
        t = self.peek('IDENTIFIER')
        if t and (t.value.lower() in keywords):
            return self.accept()

    def expect(self, ttype):
        return self.accept(ttype) or self.unexpected_token(ttype)

    def error(self, msg=None, token=None):
        token = token or self.peek()
        if token:
            if token.type == 'NEWLINE':
                msg = 'Line ended unexpectedly'
            raise InvalidSyntax(msg, token)
        raise InvalidSyntax(msg or 'Input ended unexpectedly')

    def unexpected_token(self, expected, token=None):
        if len(expected) == 1:
            expected = repr(str(expected))
        else:
            expected = expected.lower()
        msg = 'Expected ' + expected
        token = token or self.peek()
        if token and token.type != 'NEWLINE':
            msg += ', found %r' % str(token.value)
        raise InvalidSyntax(msg, token)

    def ignore_newlines(self):
        return self._tokens.ignore_newlines()

    def parse(self, text):
        self._token_streams.append(TokenStream(self.lexer, text))
        try:
            result = self.start()
            if self.peek():
                self.error()
            return result
        except InvalidSyntax as e:
            token = e.token
            if token is None:
                self.error_logger(e.msg, lineno=e.lineno, colno=e.colno)
            else:
                msg = e.msg or ('Invalid syntax at %r' % str(token.value))
                self.error_logger(msg, token.value, token.lineno, token.colno)
        finally:
            self._token_streams.pop()

    def start(self):
        return self.expr()

    def expr_list(self):
        items = []
        while True:
            expr = self.expr()
            if self.accept(':'):
                lineno = self.curr.lineno
                colno = self.curr.colno
                start = expr
                stop = self.expr()
                if self.accept(':'):
                    step = self.expr()
                else:
                    step = None
                expr = ast.RangeExpr(lineno,
                                     colno,
                                     start = start,
                                     stop = stop,
                                     step = step)
            items.append(expr)
            if not self.accept(','):
                break
        return ast.ExprList(items=tuple(items))

    def expr(self):
        return self.or_expr()

    def or_expr(self):
        expr = self.and_expr()
        while self.accept('OR') or self.accept_keyword('or'):
            expr = self._binary_op(expr, self.curr, self.and_expr())
        return expr

    def and_expr(self):
        expr = self.comparison_expr()
        while self.accept('AND') or self.accept_keyword('and'):
            expr = self._binary_op(expr, self.curr, self.comparison_expr())
        return expr

    def comparison_expr(self):
        expr = self.additive_expr()
        while self.accept('COMPARISONOP'):
            expr = self._binary_op(expr, self.curr, self.additive_expr())
        return expr

    def additive_expr(self):
        expr = self.multiplicative_expr()
        while self.accept('ADDITIVEOP'):
            expr = self._binary_op(expr, self.curr, self.multiplicative_expr())
        return expr

    def multiplicative_expr(self):
        expr = self.cast_expr()
        while self.accept('*') or self.accept('/') or self.accept('%'):
            expr = self._binary_op(expr, self.curr, self.cast_expr())
        return expr

    def _binary_op(self, lhs, op, rhs):
        return ast.BinaryOpExpr(op.lineno,
                                op.colno,
                                op = op.value,
                                operands = (lhs, rhs))

    def cast_expr(self):
        t1 = self.peek('(')
        t2 = self.peek('IDENTIFIER', depth=2)
        if (t1 and t2 and t2.value in ('bool',
                                       'char',
                                       'short',
                                       'int',
                                       'integer',
                                       'long',
                                       'byte',
                                       'word',
                                       'dword',
                                       'qword',
                                       'float',
                                       'double',
                                       'string')):

            self.accept()
            lineno = self.curr.lineno
            colno = self.curr.colno
            self.accept()
            totype = self.curr.value
            self.expect(')')
            target = self.unary_expr()
            return ast.CastExpr(lineno,
                                colno,
                                totype = totype,
                                target = target)

        return self.unary_expr()

    def unary_expr(self):
        if (self.accept('ADDITIVEOP') or
            self.accept('!') or
            self.accept_keyword('not')):

            op = self.curr
            operand = self.subscript_expr()
            return ast.UnaryOpExpr(op.lineno,
                                   op.colno,
                                   op = op.value,
                                   operand = operand)

        return self.subscript_expr()

    def subscript_expr(self):
        expr = self.units_expr()

        while self.accept('['):
            lineno = self.curr.lineno
            colno = self.curr.colno
            with self.ignore_newlines():
                value = self.expr()
                self.expect(']')
            expr = ast.SubscriptExpr(lineno,
                                     colno,
                                     target = expr,
                                     value = value)


        return expr

    def units_expr(self):
        expr = self.atomic_expr()

        if self.accept_keyword('us', 'ms', 's'):
            expr = ast.UnitsExpr(self.curr.lineno,
                                 self.curr.colno,
                                 target = expr,
                                 unit = self.curr.value)

        return expr

    def atomic_expr(self):
        if self.accept('('):
            return self._parenthetic_expr()

        if self.accept('{'):
            return self._dict_literal_expr()

        if self.accept('['):
            return self._list_literal_expr()

        if self.accept('STRING'):
            return self._string_literal_expr()

        if self.accept('NUMBER'):
            return self._number_literal_expr()

        if self.accept('IDENTIFIER'):
            expr = self._identifier_expr()
            if self.accept('('):
                return self._function_call_expr(expr.value)
            return expr

        self.error()

    def _parenthetic_expr(self):
        lineno = self.curr.lineno
        colno = self.curr.colno
        with self.ignore_newlines():
            expr = self.expr()
            self.expect(')')
        return ast.ParentheticExpr(lineno, colno, expr=expr)

    def _dict_literal_expr(self):
        lineno = self.curr.lineno
        colno = self.curr.colno
        items = []

        with self.ignore_newlines():
            if not self.accept('}'):
                while True:
                    key = self.expr()
                    self.expect(':')
                    value = self.expr()
                    items.append((key, value))
                    if not self.accept(','):
                        self.expect('}')
                        break

        return ast.DictLiteralExpr(lineno, colno, items=tuple(items))

    def _list_literal_expr(self):
        lineno = self.curr.lineno
        colno = self.curr.colno

        with self.ignore_newlines():
            if self.accept(']'):
                items = ()
            else:
                items = self.expr_list().items
                self.expect(']')

        return ast.ListLiteralExpr(lineno, colno, items=items)

    def _string_literal_expr(self):
        return ast.StringLiteralExpr(self.curr.lineno,
                                     self.curr.colno,
                                     value = self.curr.value)

    def _number_literal_expr(self):
        return ast.NumberLiteralExpr(self.curr.lineno,
                                     self.curr.colno,
                                     value = self.curr.value)

    def _function_call_expr(self, name):
        lineno = self.curr.lineno
        colno = self.curr.colno

        with self.ignore_newlines():
            if self.accept(')'):
                args = ()
            else:
                args = self.expr_list().items
                self.expect(')')

        return ast.FunctionCallExpr(lineno, colno, name=name, args=args)

    def _identifier_expr(self):
        return ast.IdentifierExpr(self.curr.lineno,
                                  self.curr.colno,
                                  value = self.curr.value)


class Parser(ExpressionParser):

    def __init__(self, error_logger):
        super(Parser, self).__init__(error_logger)
        self._basepaths = []
        self.included_files = collections.OrderedDict()

    def accept_newline(self):
        t = self.accept('NEWLINE')
        if t:
            while self.accept('NEWLINE'):
                pass
        return t

    def accept_directive(self, name):
        t1 = self.peek('%')
        t2 = self.peek('IDENTIFIER', depth=2)
        if t1 and t2 and (t2.value == name):
            self.accept()
            return self.accept()

    def parse(self, text, basepath=''):
        self._basepaths.append(basepath)
        try:
            return super(Parser, self).parse(text)
        finally:
            self._basepaths.pop()

    def parse_file(self, filepath, _lineno=None, _colno=None):
        src = readfile(filepath, self.error_logger, _lineno, _colno)
        if src is not None:
            self.included_files[filepath] = src
            with self.error_logger.filename(filepath):
                return self.parse(src, os.path.dirname(filepath))

    def start(self):
        return self.module()

    def module(self):
        self.accept_newline()
        stmts = []
        while self.peek():
            stmts.append(self.stmt(True))
        return ast.Module(0, 0, filename='', statements=tuple(stmts))

    def stmt(self, _toplevel=False):
        if self.accept('IDENTIFIER'):
            if self.peek('=') or self.peek('AUGASSIGN') or self.peek('['):
                stmt = self._assignment_stmt()
            else:
                stmt = self._decl_stmt()

        elif self.accept_directive('include'):
            if not _toplevel:
                self.error('Include statements are permitted at the top level '
                           'only',
                           token = self.curr)
            stmt = self._include_stmt()

        elif self.accept_directive('define'):
            if not _toplevel:
                self.error('Macro definitions are permitted at the top level '
                           'only',
                           token = self.curr)
            stmt = self._define_stmt()

        else:
            self.error()

        if self.peek() and not self.accept_newline():
            self.error('Missing newline at end of statement')

        return stmt

    def _assignment_stmt(self):
        varname = self.curr.value

        indices = []
        while self.accept('['):
            indices.append(self.expr())
            self.expect(']')

        if self.accept('AUGASSIGN'):
            op = self.curr.value[0]
        else:
            op = None
            if not self.accept('='):
                self.unexpected_token('assignment operator')
        lineno = self.curr.lineno
        colno = self.curr.colno

        value = self.expr()

        return ast.AssignmentStmt(lineno,
                                  colno,
                                  varname = varname,
                                  indices = tuple(indices),
                                  op = op,
                                  value = value)

    def _decl_stmt(self):
        lineno = self.curr.lineno
        colno = self.curr.colno

        typename = self.curr.value
        if self.accept('/'):
            self.expect('IDENTIFIER')
            typename += '/' + self.curr.value

        tagname = None
        value = None
        if self.peek('IDENTIFIER') or self.peek('STRING'):
            tagname = self._identifier_or_string()
            if self.accept('='):
                value = self.expr()

        if not self.accept('('):
            if not (value or self.peek('{')):
                self.unexpected_token("'(' or '{'")
            params = ()
        else:
            self.accept_newline()
            if self.accept(')'):
                params = ()
            elif self.peek('IDENTIFIER') and self.peek('=', depth=2):
                params = collections.OrderedDict()
                while True:
                    self.expect('IDENTIFIER')
                    pname = self.curr.value
                    self.expect('=')
                    pvalue = self._exprlist_or_expr()
                    params[pname] = pvalue
                    if self.accept(')'):
                        break
                    if self.accept_newline():
                        if self.accept(')'):
                            break
                    elif not self.accept(';'):
                        self.unexpected_token("line ending or ';'")
            else:
                with self.ignore_newlines():
                    params = (self._exprlist_or_expr(),)
                    self.expect(')')

        children = []
        if self.accept('{'):
            self.accept_newline()
            while not self.accept('}'):
                children.append(self.stmt())

        return ast.DeclarationStmt(lineno,
                                   colno,
                                   type = typename,
                                   tag = tagname,
                                   value = value,
                                   params = params,
                                   children = tuple(children))

    def _exprlist_or_expr(self):
        expr = self.expr_list()
        if len(expr.items) == 1:
            expr = expr.items[0]
        return expr

    def _include_stmt(self):
        lineno = self.curr.lineno
        colno = self.curr.colno
        target = self._identifier_or_string()

        filepath = os.path.normpath(os.path.join(self._basepaths[-1], target))
        root, ext = os.path.splitext(filepath)
        filepath = root + (ext or '.mwel')

        statements = ()
        if filepath not in self.included_files:
            include_module = self.parse_file(filepath, lineno, colno)
            if include_module:
                statements = include_module.statements

        return ast.Module(lineno,
                          colno,
                          filename = filepath,
                          statements = statements)

    def _identifier_or_string(self):
        if self.accept('IDENTIFIER'):
            return self.curr.value
        if self.accept('STRING'):
            return self.curr.value[1:-1]
        self.unexpected_token('identifier or string')

    def _define_stmt(self):
        lineno = self.curr.lineno
        colno = self.curr.colno

        self.expect('IDENTIFIER')
        name = self.curr.value

        if self.accept('='):
            return self._expression_macro_stmt(lineno, colno, name)

        if not self.accept('('):
            self.unexpected_token("'=' or '('")

        parameters = []
        with self.ignore_newlines():
            if not self.accept(')'):
                while True:
                    self.expect('IDENTIFIER')
                    param = self.curr.value
                    if param in parameters:
                        self.error("Macro definition has multiple parameters "
                                   "named '%s'" % param,
                                   token = self.curr)
                    parameters.append(param)
                    if not self.accept(','):
                        break
                self.expect(')')

        if not self.accept_newline():
            return self._expression_macro_stmt(lineno, colno, name, parameters)

        statements = []
        while not self.accept_directive('end'):
            statements.append(self.stmt())

        return ast.StatementMacroStmt(lineno,
                                      colno,
                                      name = name,
                                      parameters = tuple(parameters),
                                      statements = tuple(statements))

    def _expression_macro_stmt(self, lineno, colno, name, parameters=()):
        value = self.expr()
        return ast.ExpressionMacroStmt(lineno,
                                       colno,
                                       name = name,
                                       parameters = tuple(parameters),
                                       value = value)
