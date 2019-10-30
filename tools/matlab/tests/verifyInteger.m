function verifyInteger(testCase, actual, expected)
verifyTrue(testCase, isa(actual, 'int64'));
verifyEqual(testCase, actual, int64(expected));
