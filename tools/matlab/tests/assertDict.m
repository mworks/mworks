function assertDict(expected, actual)
assertTrue(isstruct(actual));
assertEqual(expected, actual);
