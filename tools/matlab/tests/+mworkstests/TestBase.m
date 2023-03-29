classdef TestBase < matlab.unittest.TestCase
    methods
        function filename = getFilename(t)
            filename = getenv('MW_MATLAB_TEST_FILENAME');
        end

        function tagmap = getTagMap(t)
            e = getEvents(t.getFilename, 1);
            tagmap = e.data;
        end

        function verifyInteger(t, actual, expected)
            t.verifyClass(actual, 'int64');
            t.verifyEqual(actual, int64(expected));
        end

        function verifyFloat(t, actual, expected)
            t.verifyClass(actual, 'double');
            t.verifyEqual(actual, expected);
        end

        function verifyString(t, actual, expected)
            t.verifyClass(actual, 'char');
            t.verifyEqual(actual, expected);
        end

        function verifyList(t, actual, expected)
            t.verifyClass(actual, 'cell');
            t.verifyEqual(actual, expected);
        end

        function verifyDict(t, actual, expected)
            t.verifyClass(actual, 'struct');
            t.verifyEqual(actual, expected);
        end

        function verifyDictAsMap(t, actual, expectedKeys, expectedValues)
            t.verifyClass(actual, 'containers.Map');
            t.verifyEqual(keys(actual), expectedKeys);
            t.verifyEqual(values(actual), expectedValues);
        end

        function verifyDictAsFallbackStruct(t, actual, expectedKeys, ...
                                            expectedValues)
            t.verifyClass(actual, 'struct');
            t.verifySize(actual, [1 1]);
            t.verifyLength(fieldnames(actual), 2);
            t.verifyTrue(isfield(actual, 'keys'));
            t.verifyTrue(isfield(actual, 'values'));
            actualKeys = actual.keys;
            actualValues = actual.values;

            t.verifyCellVector(expectedKeys);
            t.verifyCellVector(actualKeys);
            t.verifyCellVector(expectedValues);
            t.verifyCellVector(actualValues);

            expectedSize = size(expectedKeys);
            t.verifySize(actualKeys, expectedSize);
            t.verifySize(expectedValues, expectedSize);
            t.verifySize(actualValues, expectedSize);

           for i = 1:length(expectedKeys)
               index = t.findInCellVector(expectedKeys{i}, actualKeys);
               t.verifyTrue(index ~= 0);
               t.verifyEqual(actualValues{index}, expectedValues{i});
           end
        end

        function verifyCellVector(t, value)
            t.verifyClass(value, 'cell');
            t.verifyTrue(isvector(value));
        end

        function verifyEvent(t, evt)
            t.verifyClass(evt, 'struct');
            t.verifyTrue(isvector(evt));
            t.verifyLength(fieldnames(evt), 3);
            t.verifyTrue(all(arrayfun(@(e) isa(e.event_code, 'int32'), evt)));
            t.verifyTrue(all(arrayfun(@(e) isa(e.time_us, 'int64'), evt)));
            t.verifyTrue(isfield(evt, 'data'));
        end
    end

    methods (Static)
        function index = findInCellVector(value, cv)
            index = 0;
            for i = 1:length(cv)
                if isequal(cv{i}, value)
                    index = i;
                    break;
                end
            end
        end
    end
end
