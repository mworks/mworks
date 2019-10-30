function verifyFloat(testCase, actual, expected)
verifyTrue(testCase, isa(actual, 'double'));
verifyEqual(testCase, actual, expected);
