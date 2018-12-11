from __future__ import division, print_function, unicode_literals

from . import lex
from .lex import TOKEN


class Lexer(object):

    def __init__(self, error_logger):
        self.error_logger = error_logger
        self.tokens += tuple(t for t in
                             (t.split('_')[-1] for t in dir(self)
                              if t.startswith('t_'))
                             if t.isupper())
        self._lexer = lex.lex(module=self)
        self._lineno_to_lexpos = {1: -1}

    def input(self, text):
        self._lexer.lineno = 1  # Reset lineno
        self._lexer.input(text)

    def token(self):
        t = self._lexer.token()
        if t:
            t.colno = self.lexpos_to_colno(t.lineno, t.lexpos)
        elif self._lexer.current_state() in ('sstring', 'dstring'):
            self.log_error('Unterminated string literal',
                           lineno = self.string_lineno,
                           lexpos = self.string_lexpos)
            self._lexer.pop_state()
        elif self._lexer.current_state() == 'comment':
            self.log_error('Unterminated multiline comment',
                           lineno = self.comment_lineno,
                           lexpos = self.comment_lexpos)
            self._lexer.pop_state()
        return t

    def update_lineno(self, t):
        index = 0
        while True:
            index = t.value.find('\n', index)
            if index == -1:
                break
            t.lexer.lineno += 1
            self._lineno_to_lexpos[t.lexer.lineno] = t.lexpos + index
            index += 1

    def lexpos_to_colno(self, lineno, lexpos):
        if (lineno is None) or (lexpos is None):
            return None
        return lexpos - self._lineno_to_lexpos[lineno]

    def log_error(self, msg, token=None, lineno=None, lexpos=None):
        colno = self.lexpos_to_colno(lineno, lexpos)
        self.error_logger(msg, token, lineno, colno)

    states = (
        ('sstring', 'exclusive'),
        ('dstring', 'exclusive'),
        ('comment', 'exclusive'),
        )

    tokens = ('STRING',)

    t_ignore = ' \t\r'
    t_sstring_dstring_comment_ignore = ''

    t_ignore_comment = r'//[^\n]*'

    t_ADDITIVEOP = r'\+|-'
    t_AND = r'&&|\#AND'
    t_AUGASSIGN = r'\+=|-=|\*=|/=|%='
    t_COMPARISONOP = r'==|!=|<=|=<|\#LE|<|\#LT|>=|=>|\#GE|>|\#GT'
    t_OR = r'\|\||\#OR'

    literals = '=:,/{[(%*!}]);'

    def begin_string(self, t, state):
        t.lexer.push_state(state)
        self.string_lineno = t.lineno
        self.string_lexpos = t.lexpos
        self.string_value = t.value

    def end_string(self, t):
        t.lexer.pop_state()
        t.type = 'STRING'
        t.lineno = self.string_lineno
        t.lexpos = self.string_lexpos
        t.value = self.string_value + t.value
        return t

    def t_begin_sstring(self, t):
        r"'"
        self.begin_string(t, 'sstring')

    def t_begin_dstring(self, t):
        r'"'
        self.begin_string(t, 'dstring')

    def t_sstring_body(self, t):
        r"[^'\\\n]+"
        self.string_value += t.value

    def t_dstring_body(self, t):
        r'[^"\\\n]+'
        self.string_value += t.value

    def t_sstring_dstring_escape_sequence(self, t):
        r'''\\[\\'"]'''
        self.string_value += t.value

    def t_sstring_dstring_backslash(self, t):
        r'\\'
        self.string_value += t.value

    def t_sstring_dstring_newline(self, t):
        r'\n'
        self.log_error('Unterminated string literal',
                       t.value[0],
                       t.lexer.lineno,
                       t.lexer.lexpos)
        self.update_lineno(t)
        t.value = ''
        return self.end_string(t)

    def t_sstring_end(self, t):
        r"'"
        return self.end_string(t)

    def t_dstring_end(self, t):
        r'"'
        return self.end_string(t)

    def t_INITIAL_comment_begin_comment(self, t):
        r'/\*'
        t.lexer.push_state('comment')
        self.comment_lineno = t.lineno
        self.comment_lexpos = t.lexpos

    def t_comment_body(self, t):
        r'([^\n/*] | (/(?!\*)) | (\*(?!/)))+'
        pass

    def t_comment_newline(self, t):
        r'\n+'
        self.update_lineno(t)

    def t_comment_end(self, t):
        r'\*/'
        t.lexer.pop_state()

    @TOKEN(
        r'( ([0-9]+(\.[0-9]*)?) |'
        r'  (\.[0-9]+) )'
        r'([eE][+-]?[0-9]+)?'
        )
    def t_NUMBER(self, t):
        return t

    _identifier_re = r'[a-zA-Z][a-zA-Z0-9_]*'
    @TOKEN(r'((\$\{%s\}[0-9_]*)|(%s))+' % (_identifier_re, _identifier_re))
    def t_IDENTIFIER(self, t):
        return t

    def t_NEWLINE(self, t):
        r'\n+'
        self.update_lineno(t)
        return t

    def t_ANY_error(self, t):
        bad_char = t.value[0]
        try:
            bad_char = str(bad_char)
        except UnicodeEncodeError:
            pass
        self.log_error('Illegal character: %r' % bad_char,
                       bad_char,
                       t.lexer.lineno,
                       t.lexer.lexpos)
        t.lexer.skip(1)
