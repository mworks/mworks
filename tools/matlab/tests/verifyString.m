function verifyString(testCase, actual, expected)
verifyTrue(testCase, isa(actual, 'char'));
verifyEqual(testCase, actual, expected);
