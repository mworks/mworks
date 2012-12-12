function assertInteger(expected, actual)
assertTrue(isa(actual, 'int64'));
assertEqual(int64(expected), actual);
