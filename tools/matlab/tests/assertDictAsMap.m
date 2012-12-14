function assertDictAsMap(expectedKeys, expectedValues, actual)
assertTrue(isa(actual, 'containers.Map'));
assertEqual(expectedKeys, keys(actual));
assertEqual(expectedValues, values(actual));
