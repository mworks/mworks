classdef GetEventsTestBase < mworkstests.TestBase
    methods (Abstract)
        varargout = getEvents(t, varargin)
    end

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
        function testTooManyInputs(t)
            function tooManyInputs
                e = t.getEvents(1, 2, 3, 4);
            end

            t.verifyError(@tooManyInputs, 'MATLAB:TooManyInputs');
        end

        function testBadNumberOfOutputs(t)
            function zeroOutputs
                t.getEvents;
            end

            function twoOutputs
                [a, b] = t.getEvents;
            end

            function fourOutputs
                [a, b, c, d] = t.getEvents;
            end

            t.verifyError(@zeroOutputs, 'MWorks:WrongNumberOfOutputs');
            t.verifyError(@twoOutputs, 'MWorks:WrongNumberOfOutputs');
            t.verifyError(@fourOutputs, 'MWorks:WrongNumberOfOutputs');
        end

        function testInvalidInputs(t)
            function codesNotNumeric
                e = t.getEvents({1});
            end

            function codesNotIntegral
                e = t.getEvents(1.2);
            end

            function codesNotNonNegative
                e = t.getEvents(-1);
            end

            function minTimeNotNumeric
                e = t.getEvents(0, {1});
            end

            function minTimeNotIntegral
                e = t.getEvents(0, 1.2);
            end

            function minTimeNotScalar
                e = t.getEvents(0, [1,2]);
            end

            function maxTimeNotNumeric
                e = t.getEvents(0, 0, {1});
            end

            function maxTimeNotIntegral
                e = t.getEvents(0, 0, 1.2);
            end

            function maxTimeNotScalar
                e = t.getEvents(0, 0, [1,2]);
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
            t.verifyEvents(t.getEvents, expected_codes);
            t.verifyEvents(t.getEvents([]), expected_codes);
        end

        function testGetEventsByCode(t)
            t.verifyEvents(t.getEvents(7), 7);
            t.verifyEvents(t.getEvents([15, 3, 9, 3]), [3, 9, 15]);
            t.verifyEvents(t.getEvents([7, 10000, 8]), [7,8]);
            t.verifyEmpty(t.getEvents(10000));
        end

        function testGetEventsByTime(t)
            function badTimeRange
                e = t.getEvents([], 10, 5);
            end

            max_time = length(fieldnames(t.getTagMap)) + 1;
            t.verifyEvents(t.getEvents([], 5), [5:max_time]);
            t.verifyEvents(t.getEvents([], -1, 10), [0:10]);
            t.verifyEvents(t.getEvents([], 7, 7), 7);
            t.verifyEmpty(t.getEvents([], 100, 200));
            t.verifyError(@badTimeRange, 'MWorks:MWKFileError');
        end

        function testGetEventsByCodeAndTime(t)
            t.verifyEvents(t.getEvents(5:8, 7), 7:8);
            t.verifyEvents(t.getEvents(5:8, 0, 6), 5:6);
            t.verifyEvents(t.getEvents(5:8, 6, 7), 6:7);
            t.verifyEvents(t.getEvents(5:8, 4, 9), 5:8);
        end

        function testSeparateOutputArrays(t)
            evts = t.getEvents;
            [codes, times, values] = t.getEvents;
            t.verifySeparateArrays(codes, times, values, evts);

            evts = t.getEvents([5:10], 4, 8);
            [codes, times, values] = t.getEvents([5:10], 4, 8);
            t.verifySeparateArrays(codes, times, values, evts);

            [codes, times, values] = t.getEvents(10000);
            t.verifyEqual(codes, int32([]));
            t.verifyEqual(times, int64([]));
            t.verifyEqual(values, {});
        end
    end
end
