function verifyList(testCase, actual, expected)
verifyTrue(testCase, iscell(actual));
verifyEqual(testCase, actual, expected);
