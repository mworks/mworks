from __future__ import division, print_function, unicode_literals


###############################################################################
#
# Base class
#
###############################################################################


class AST(object):

    _fields = ()

    @property
    def _name(self):
        return type(self).__name__

    def __init__(self, lineno=-1, colno=-1, **kwargs):
        self.lineno = lineno
        self.colno = colno
        for field in self._fields:
            setattr(self, field, kwargs.pop(field))
        if kwargs:
            raise TypeError('%s has no field %r' %
                            (self._name, kwargs.popitem()[0]))

    def __eq__(self, other):
        return ((type(other) is type(self)) and
                all(getattr(self, field) == getattr(other, field)
                    for field in self._fields))

    def __ne__(self, other):
        return not (self == other)

    def __repr__(self):
        args = ', '.join('%s=%r' % (f, getattr(self, f)) for f in self._fields)
        return '%s(%s)' % (self._name, args)


###############################################################################
#
# Statements
#
###############################################################################


class Module(AST):

    _fields = ('filename', 'statements')


class Stmt(AST):
    pass


class VarStmt(Stmt):

    _fields = ('name', 'value')


class AssignmentStmt(Stmt):

    _fields = ('varname', 'value')


class IndexAssignmentStmt(Stmt):

    _fields = ('varname', 'indices', 'value')


class DeclarationStmt(Stmt):

    _fields = ('type', 'tag', 'params', 'children')


class MacroStmt(Stmt):
    pass


class ExpressionMacroStmt(MacroStmt):

    _fields = ('name', 'parameters', 'value')


class StatementMacroStmt(MacroStmt):

    _fields = ('name', 'parameters', 'statements')


###############################################################################
#
# Expressions
#
###############################################################################


class ExprList(AST):

    _fields = ('items',)


class RangeExpr(AST):

    _fields = ('start', 'stop')


class Expr(AST):
    pass


class BinaryOpExpr(Expr):

    _fields = ('op', 'operands')


class CastExpr(Expr):

    _fields = ('totype', 'target')


class UnaryOpExpr(Expr):

    _fields = ('op', 'operand')


class SubscriptExpr(Expr):

    _fields = ('target', 'value')


class UnitsExpr(Expr):

    _fields = ('target', 'unit')


class AtomicExpr(Expr):
    pass


class ParentheticExpr(AtomicExpr):

    _fields = ('expr',)


class FunctionCallExpr(AtomicExpr):

    _fields = ('name', 'args')


class DictLiteralExpr(AtomicExpr):

    _fields = ('items',)


class ListLiteralExpr(AtomicExpr):

    _fields = ('items',)


class StringLiteralExpr(AtomicExpr):

    _fields = ('value',)


class NumberLiteralExpr(AtomicExpr):

    _fields = ('value',)


class IdentifierExpr(AtomicExpr):

    _fields = ('value',)
