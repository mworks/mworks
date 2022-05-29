import collections
from contextlib import contextmanager
import unittest

from mwel.lexer import Lexer


class TestLexer(unittest.TestCase):

    def setUp(self):
        self.errors = collections.deque()
        def error_logger(*info):
            self.errors.append(info)
            
        self.lexer = Lexer(error_logger)
        self.ignore_newlines = True

        @contextmanager
        def input_wrapper(s):
            self.lexer.input(s)
            self.advance()

            yield

            extra_tokens = []
            while self.next_token is not None:
                extra_tokens.append(self.next_token)
                self.advance()
            
            if extra_tokens:
                self.fail('input contained unexpected tokens: ' +
                          ', '.join(repr(t) for t in extra_tokens))
                
            if self.errors:
                self.fail('input contained unexpected errors: ' +
                          ', '.join(repr(e) for e in self.errors))

        self.input = input_wrapper

    def advance(self):
        while True:
            self.next_token = self.lexer.token()
            if not (self.next_token and
                    self.ignore_newlines and
                    self.next_token.type == 'NEWLINE'):
                break

    def assertError(self, value=None, lineno=None, msg=None):
        self.assertTrue(self.errors, 'expected error was not detected')
        e = self.errors.popleft()
        
        self.assertEqual(value, e[1])
        if lineno:
            self.assertEqual(lineno, e[2])
        if msg:
            self.assertEqual(msg, e[0])

    def assertToken(self, type, value=None, lineno=None):
        t = self.next_token
        
        self.assertIsNotNone(t)
        self.assertEqual(type, t.type)
        self.assertEqual(value or type, t.value)
        if lineno:
            self.assertEqual(lineno, t.lineno)

        self.advance()

        return t

    def assertString(self, value):
        self.assertToken('STRING', value)

    def assertNumber(self, value, lineno=None):
        self.assertToken('NUMBER', value, lineno)

    def assertIdentifier(self, value, lineno=None):
        self.assertToken('IDENTIFIER', value, lineno)

    def test_whitespace(self):
        self.ignore_newlines = False
        # Linefeed ('\n') is a token.  Space, horizontal tab ('\t'), and
        # carriage return ('\r') are ignored.  All others
        # ('\f', '\v') are invalid.
        with self.input('  \n \n\t\t 1 2 \f 3 \r \v\r \n\n\n 4  \t'):
            self.assertToken('NEWLINE', '\n')
            self.assertToken('NEWLINE', '\n')
            self.assertToken('NUMBER', '1', lineno=3)
            self.assertToken('NUMBER', '2')
            self.assertError('\f')
            self.assertToken('NUMBER', '3')
            self.assertError('\v')
            self.assertToken('NEWLINE', '\n\n\n')
            self.assertToken('NUMBER', '4', lineno=6)

    def test_comments(self):
        with self.input('   // foo // 123 abc'):
            self.assertIsNone(self.next_token)

        with self.input('1 // foo 123 abc\n// blah blah\n2'):
            self.assertToken('NUMBER', '1')
            self.assertToken('NUMBER', '2', lineno=3)

    def test_multiline_comments(self):
        self.ignore_newlines = False

        # Basic
        with self.input('''
                        foo
                        /* bar blah
                        1 2 3 * /
                        + - */
                        / * 4
                        abc
                        /* def'''):
            self.assertToken('NEWLINE', '\n')
            self.assertIdentifier('foo')
            self.assertToken('NEWLINE', '\n')
            self.assertToken('NEWLINE', '\n')
            self.assertToken('/', lineno=6)
            self.assertToken('*')
            self.assertNumber('4')
            self.assertToken('NEWLINE', '\n')
            self.assertIdentifier('abc', lineno=7)
            self.assertToken('NEWLINE', '\n')
            self.assertError(lineno=8, msg='Unterminated multiline comment')

        # Nested
        with self.input('''
                        foo
                        /* bar /* blah /* 1 2 3
                        */
                        + */ - */
                        4
                        '''):
            self.assertToken('NEWLINE', '\n')
            self.assertIdentifier('foo')
            self.assertToken('NEWLINE', '\n')
            self.assertToken('NEWLINE', '\n')
            self.assertNumber('4', lineno=6)
            self.assertToken('NEWLINE', '\n')

    def test_operators(self):
        # Arithmetic
        with self.input('''
                        +
                        -
                        *
                        /
                        %
                        '''):
            self.assertToken('ADDITIVEOP', '+')
            self.assertToken('ADDITIVEOP', '-')
            self.assertToken('*')
            self.assertToken('/')
            self.assertToken('%')

        # Comparison
        with self.input('''
                        ==
                        !=
                        <=
                        =<
                        #LE
                        <
                        #LT
                        >=
                        =>
                        #GE
                        >
                        #GT

                        // Lowercase not accepted
                        #le #lt #ge #gt
                        '''):
            self.assertToken('COMPARISONOP', '==')
            self.assertToken('COMPARISONOP', '!=')
            self.assertToken('COMPARISONOP', '<=')
            self.assertToken('COMPARISONOP', '=<')
            self.assertToken('COMPARISONOP', '#LE')
            self.assertToken('COMPARISONOP', '<')
            self.assertToken('COMPARISONOP', '#LT')
            self.assertToken('COMPARISONOP', '>=')
            self.assertToken('COMPARISONOP', '=>')
            self.assertToken('COMPARISONOP', '#GE')
            self.assertToken('COMPARISONOP', '>')
            self.assertToken('COMPARISONOP', '#GT')
            for ident in ('le', 'lt', 'ge', 'gt'):
                self.assertError('#')
                self.assertIdentifier(ident)

        # Logical
        with self.input('''
                        &&
                        #AND
                        !
                        ||
                        #OR

                        // Lowercase not accepted
                        #and #or
                        '''):
            self.assertToken('AND', '&&')
            self.assertToken('AND', '#AND')
            self.assertToken('!')
            self.assertToken('OR', '||')
            self.assertToken('OR', '#OR')
            for ident in ('and', 'or'):
                self.assertError('#')
                self.assertIdentifier(ident)

        # Augmented assignment
        with self.input('''
                        +=
                        -=
                        *=
                        /=
                        %=

                        // Can't have spaces between characters
                        + =
                        '''):
            self.assertToken('AUGASSIGN', '+=')
            self.assertToken('AUGASSIGN', '-=')
            self.assertToken('AUGASSIGN', '*=')
            self.assertToken('AUGASSIGN', '/=')
            self.assertToken('AUGASSIGN', '%=')
            self.assertToken('ADDITIVEOP', '+')
            self.assertToken('=')

        # Other
        with self.input('''
                        =
                        :
                        ,
                        {
                        [
                        (
                        }
                        ]
                        )
                        ;
                        '''):
            self.assertToken('=')
            self.assertToken(':')
            self.assertToken(',')
            self.assertToken('{')
            self.assertToken('[')
            self.assertToken('(')
            self.assertToken('}')
            self.assertToken(']')
            self.assertToken(')')
            self.assertToken(';')

    def test_strings(self):
        # Single quotes
        with self.input(r"""
                        ''
                        'a bc def'
                        '"'
                        '123\\'
                        '4\'5\"6\7'
                        'abcd
                        'efgh"""):
            self.assertString("''")
            self.assertString("'a bc def'")
            self.assertString("'\"'")
            self.assertString(r"'123\\'")
            self.assertString(r"'4\'5\"6\7'")
            self.assertString("'abcd")
            self.assertError('\n', lineno=7, msg='Unterminated string literal')
            self.assertError(lineno=8, msg='Unterminated string literal')

        # Double quotes
        with self.input(r'''
                        ""
                        "a bc def"
                        "'"
                        "123\\"
                        "4\'5\"6\7"
                        "abcd
                        "efgh'''):
            self.assertString('""')
            self.assertString('"a bc def"')
            self.assertString('"\'"')
            self.assertString(r'"123\\"')
            self.assertString(r'"4\'5\"6\7"')
            self.assertString('"abcd')
            self.assertError('\n', lineno=7, msg='Unterminated string literal')
            self.assertError(lineno=8, msg='Unterminated string literal')

    def test_numbers(self):
        # Without decimal point
        with self.input('''
                        0
                        12
                        012
                        +123
                        -4567
                        + 1
                        - 2
                        '''):
            self.assertNumber('0')
            self.assertNumber('12')
            self.assertNumber('012')
            self.assertToken('ADDITIVEOP', '+')
            self.assertNumber('123')
            self.assertToken('ADDITIVEOP', '-')
            self.assertNumber('4567')
            self.assertToken('ADDITIVEOP', '+')
            self.assertNumber('1')
            self.assertToken('ADDITIVEOP', '-')
            self.assertNumber('2')

        # With decimal point
        with self.input('''
                        1.2
                        12.
                        .123
                        123.4567
                        +1.2
                        -2.3
                        + 1.0
                        - 2.0
                        .
                        '''):
            self.assertNumber('1.2')
            self.assertNumber('12.')
            self.assertNumber('.123')
            self.assertNumber('123.4567')
            self.assertToken('ADDITIVEOP', '+')
            self.assertNumber('1.2')
            self.assertToken('ADDITIVEOP', '-')
            self.assertNumber('2.3')
            self.assertToken('ADDITIVEOP', '+')
            self.assertNumber('1.0')
            self.assertToken('ADDITIVEOP', '-')
            self.assertNumber('2.0')
            self.assertError('.')

        # With exponent
        with self.input('''
                        1e0
                        .2E2
                        3.e3
                        -2.2E+12
                        +3.45e-123
                        1.23e
                        '''):
            self.assertNumber('1e0')
            self.assertNumber('.2E2')
            self.assertNumber('3.e3')
            self.assertToken('ADDITIVEOP', '-')
            self.assertNumber('2.2E+12')
            self.assertToken('ADDITIVEOP', '+')
            self.assertNumber('3.45e-123')
            self.assertNumber('1.23')
            self.assertIdentifier('e')

    def test_identifiers(self):
        with self.input('''
                        a
                        AbC
                        a1B2c3_4_D
                        ${a1B2c3_4_D}
                        ${ABC}_XYZ
                        a${b}1_2_3${c}d
                        $a1B2c3_4_D
                        $ABC_XYZ
                        a$b1_2_3$cd
                        ${ABC}123$XYZ456
                        123abc
                        _A
                        ${abc def}
                        ${_A}
                        ${xyz
                        $_A
                        '''):
            self.assertIdentifier('a')
            self.assertIdentifier('AbC')
            self.assertIdentifier('a1B2c3_4_D')
            self.assertIdentifier('${a1B2c3_4_D}')
            self.assertIdentifier('${ABC}_XYZ')
            self.assertIdentifier('a${b}1_2_3${c}d')
            self.assertIdentifier('$a1B2c3_4_D')
            self.assertIdentifier('$ABC_XYZ')
            self.assertIdentifier('a$b1_2_3$cd')
            self.assertIdentifier('${ABC}123$XYZ456')

            self.assertNumber('123')
            self.assertIdentifier('abc')

            self.assertError('_')
            self.assertIdentifier('A')

            self.assertError('$')
            self.assertToken('{')
            self.assertIdentifier('abc')
            self.assertIdentifier('def')
            self.assertToken('}')

            self.assertError('$')
            self.assertToken('{')
            self.assertError('_')
            self.assertIdentifier('A')
            self.assertToken('}')

            self.assertError('$')
            self.assertToken('{')
            self.assertIdentifier('xyz')

            self.assertError('$')
            self.assertError('_')
            self.assertIdentifier('A')

    def test_illegal_char_message(self):
        with self.input('a.b'):
            self.assertIdentifier('a')

            self.assertTrue(self.errors)
            e = self.errors.popleft()
            self.assertEqual("Illegal character: '.'", e[0])

            self.assertIdentifier('b')
