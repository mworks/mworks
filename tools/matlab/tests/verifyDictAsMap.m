function verifyDictAsMap(testCase, actual, expectedKeys, expectedValues)
verifyTrue(testCase, isa(actual, 'containers.Map'));
verifyEqual(testCase, keys(actual), expectedKeys);
verifyEqual(testCase, values(actual), expectedValues);
