function verifyDictAsFallbackStruct(testCase, actual, expectedKeys, ...
                                    expectedValues)
verifyTrue(testCase, isstruct(actual));
verifyTrue(testCase, isscalar(actual));
verifyEqual(testCase, length(fieldnames(actual)), 2);
verifyTrue(testCase, isfield(actual, 'keys'));
verifyTrue(testCase, isfield(actual, 'values'));
actualKeys = actual.keys;
actualValues = actual.values;

verifyCellVector(testCase, expectedKeys);
verifyCellVector(testCase, actualKeys);
verifyCellVector(testCase, expectedValues);
verifyCellVector(testCase, actualValues);

expectedSize = size(expectedKeys);
verifyEqual(testCase, size(actualKeys), expectedSize);
verifyEqual(testCase, size(expectedValues), expectedSize);
verifyEqual(testCase, size(actualValues), expectedSize);

for i = 1:length(expectedKeys)
    index = findInCellVector(expectedKeys{i}, actualKeys);
    verifyTrue(testCase, index ~= 0);
    verifyEqual(testCase, actualValues{index}, expectedValues{i});
end

function verifyCellVector(testCase, value)
verifyTrue(testCase, iscell(value));
verifyTrue(testCase, isvector(value));

function index = findInCellVector(value, cv)
index = 0;
for i = 1:length(cv)
    if isequal(cv{i}, value)
        index = i;
        break;
    end
end
