function verifyDict(testCase, actual, expected)
verifyTrue(testCase, isstruct(actual));
verifyEqual(testCase, actual, expected);
