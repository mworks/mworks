function assertFloat(expected, actual)
assertTrue(isa(actual, 'double'));
assertEqual(expected, actual);
