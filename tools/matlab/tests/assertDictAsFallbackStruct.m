function assertDictAsFallbackStruct(expectedKeys, expectedValues, ...
                                    actual)
assertTrue(isstruct(actual));
assertTrue(isscalar(actual));
assertEqual(2, length(fieldnames(actual)));
assertTrue(isfield(actual, 'keys'));
assertTrue(isfield(actual, 'values'));
actualKeys = actual.keys;
actualValues = actual.values;

assertCellVector(expectedKeys);
assertCellVector(actualKeys);
assertCellVector(expectedValues);
assertCellVector(actualValues);

expectedSize = size(expectedKeys);
assertEqual(expectedSize, size(actualKeys));
assertEqual(expectedSize, size(expectedValues));
assertEqual(expectedSize, size(actualValues));

for i = 1:length(expectedKeys)
    index = findInCellVector(expectedKeys{i}, actualKeys);
    assertEqual(expectedValues{i}, actualValues{index});
end

function assertCellVector(value)
assertTrue(iscell(value));
assertTrue(isvector(value));

function index = findInCellVector(value, cv)
index = 0;
for i = 1:length(cv)
    if isequal(cv{i}, value)
        index = i;
        break;
    end
end
assertTrue(index ~= 0);
