classdef GetEventsTest < mworkstests.TestBase
    methods
        function verifyEvents(t, evts, expected_codes)
            t.verifyEvent(evts);
            t.verifyEqual(length(evts), length(expected_codes));
            t.verifyEqual([evts.event_code], int32(expected_codes));
            t.verifyEqual([evts.time_us], int64(expected_codes));
        end

        function verifySeparateArrays(t, codes, times, values, evts)
            t.verifyEqual(codes, [evts.event_code]);
            t.verifyEqual(times, [evts.time_us]);
            t.verifyEqual(values, {evts.data});
        end
    end

    methods (Test)
        function testBadNumberOfInputs(t)
            function tooFewInputs
                e = getEvents;
            end

            function tooManyInputs
                e = getEvents(t.getFilename, 2, 3, 4, 5);
            end

            t.verifyError(@tooFewInputs, 'MATLAB:minrhs');
            t.verifyError(@tooManyInputs, 'MATLAB:TooManyInputs');
        end

        function testBadNumberOfOutputs(t)
            function zeroOutputs
                getEvents(t.getFilename);
            end

            function twoOutputs
                [a, b] = getEvents(t.getFilename);
            end

            function fourOutputs
                [a, b, c, d] = getEvents(t.getFilename);
            end

            t.verifyError(@zeroOutputs, 'MWorks:WrongNumberOfOutputs');
            t.verifyError(@twoOutputs, 'MWorks:WrongNumberOfOutputs');
            t.verifyError(@fourOutputs, 'MWorks:WrongNumberOfOutputs');
        end

        function testInvalidInputs(t)
            function codesNotNumeric
                e = getEvents(t.getFilename, {1});
            end

            function codesNotIntegral
                e = getEvents(t.getFilename, 1.2);
            end

            function codesNotNonNegative
                e = getEvents(t.getFilename, -1);
            end

            function minTimeNotNumeric
                e = getEvents(t.getFilename, 0, {1});
            end

            function minTimeNotIntegral
                e = getEvents(t.getFilename, 0, 1.2);
            end

            function minTimeNotScalar
                e = getEvents(t.getFilename, 0, [1,2]);
            end

            function maxTimeNotNumeric
                e = getEvents(t.getFilename, 0, 0, {1});
            end

            function maxTimeNotIntegral
                e = getEvents(t.getFilename, 0, 0, 1.2);
            end

            function maxTimeNotScalar
                e = getEvents(t.getFilename, 0, 0, [1,2]);
            end

            t.verifyError(@codesNotNumeric, ...
                          'MATLAB:validators:mustBeNumericOrLogical');
            t.verifyError(@codesNotIntegral, 'MATLAB:validators:mustBeInteger');
            t.verifyError(@codesNotNonNegative, ...
                          'MATLAB:validators:mustBeNonnegative');
            t.verifyError(@minTimeNotNumeric, ...
                          'MATLAB:validators:mustBeNumericOrLogical');
            t.verifyError(@minTimeNotIntegral, 'MATLAB:validators:mustBeInteger');
            t.verifyError(@minTimeNotScalar, 'MATLAB:validation:IncompatibleSize');
            t.verifyError(@maxTimeNotNumeric, ...
                          'MATLAB:validators:mustBeNumericOrLogical');
            t.verifyError(@maxTimeNotIntegral, 'MATLAB:validators:mustBeInteger');
            t.verifyError(@maxTimeNotScalar, 'MATLAB:validation:IncompatibleSize');
        end

        function testGetAllEvents(t)
            expected_codes = [0:length(fieldnames(t.getTagMap))+1];
            t.verifyEvents(getEvents(t.getFilename), expected_codes);
            t.verifyEvents(getEvents(t.getFilename, []), expected_codes);
        end

        function testGetEventsByCode(t)
            t.verifyEvents(getEvents(t.getFilename, 7), 7);
            t.verifyEvents(getEvents(t.getFilename, [15, 3, 9, 3]), [3, 9, 15]);
            t.verifyEvents(getEvents(t.getFilename, [7, 10000, 8]), [7,8]);
            t.verifyEmpty(getEvents(t.getFilename, 10000));
        end

        function testGetEventsByTime(t)
            function badTimeRange
                e = getEvents(t.getFilename, [], 10, 5);
            end

            max_time = length(fieldnames(t.getTagMap)) + 1;
            t.verifyEvents(getEvents(t.getFilename, [], 5), [5:max_time]);
            t.verifyEvents(getEvents(t.getFilename, [], -1, 10), [0:10]);
            t.verifyEvents(getEvents(t.getFilename, [], 7, 7), 7);
            t.verifyEmpty(getEvents(t.getFilename, [], 100, 200));
            t.verifyError(@badTimeRange, 'MWorks:MWKFileError');
        end

        function testGetEventsByCodeAndTime(t)
            t.verifyEvents(getEvents(t.getFilename, 5:8, 7), 7:8);
            t.verifyEvents(getEvents(t.getFilename, 5:8, 0, 6), 5:6);
            t.verifyEvents(getEvents(t.getFilename, 5:8, 6, 7), 6:7);
            t.verifyEvents(getEvents(t.getFilename, 5:8, 4, 9), 5:8);
        end

        function testSeparateOutputArrays(t)
            evts = getEvents(t.getFilename);
            [codes, times, values] = getEvents(t.getFilename);
            t.verifySeparateArrays(codes, times, values, evts);

            evts = getEvents(t.getFilename, [5:10], 4, 8);
            [codes, times, values] = getEvents(t.getFilename, [5:10], 4, 8);
            t.verifySeparateArrays(codes, times, values, evts);

            [codes, times, values] = getEvents(t.getFilename, 10000);
            t.verifyEqual(codes, int32([]));
            t.verifyEqual(times, int64([]));
            t.verifyEqual(values, {});
        end
    end
end
