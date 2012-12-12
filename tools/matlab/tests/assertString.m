function assertString(expected, actual)
assertTrue(isa(actual, 'char'));
assertEqual(expected, actual);
