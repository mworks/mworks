function tests = testGetEvents
tests = functiontests(localfunctions);


function tooFewInputs
e = getEvents();

function tooManyInputs
e = getEvents(getFilename(), 2, 3, 4, 5);

function testBadNumberOfInputs(testCase)
verifyError(testCase, @tooFewInputs, 'MATLAB:minrhs');
verifyError(testCase, @tooManyInputs, 'MATLAB:TooManyInputs');


function zeroOutputs
getEvents(getFilename());

function twoOutputs
[a, b] = getEvents(getFilename());

function fourOutputs
[a, b, c, d] = getEvents(getFilename());

function testBadNumberOfOutputs(testCase)
verifyError(testCase, @zeroOutputs, 'MWorks:WrongNumberOfOutputs');
verifyError(testCase, @twoOutputs, 'MWorks:WrongNumberOfOutputs');
verifyError(testCase, @fourOutputs, 'MWorks:WrongNumberOfOutputs');


function codesNotNumeric
e = getEvents(getFilename(), {1});

function codesNotIntegral
e = getEvents(getFilename(), 1.2);

function codesNotNonNegative
e = getEvents(getFilename(), -1);

function minTimeNotNumeric
e = getEvents(getFilename(), 0, {1});

function minTimeNotIntegral
e = getEvents(getFilename(), 0, 1.2);

function minTimeNotScalar
e = getEvents(getFilename(), 0, [1,2]);

function maxTimeNotNumeric
e = getEvents(getFilename(), 0, 0, {1});

function maxTimeNotIntegral
e = getEvents(getFilename(), 0, 0, 1.2);

function maxTimeNotScalar
e = getEvents(getFilename(), 0, 0, [1,2]);

function testInvalidInputs(testCase)
verifyError(testCase, @codesNotNumeric, ...
            'MATLAB:validators:mustBeNumericOrLogical');
verifyError(testCase, @codesNotIntegral, 'MATLAB:validators:mustBeInteger');
verifyError(testCase, @codesNotNonNegative, ...
            'MATLAB:validators:mustBeNonnegative');
verifyError(testCase, @minTimeNotNumeric, ...
            'MATLAB:validators:mustBeNumericOrLogical');
verifyError(testCase, @minTimeNotIntegral, 'MATLAB:validators:mustBeInteger');
verifyError(testCase, @minTimeNotScalar, 'MATLAB:validation:IncompatibleSize');
verifyError(testCase, @maxTimeNotNumeric, ...
            'MATLAB:validators:mustBeNumericOrLogical');
verifyError(testCase, @maxTimeNotIntegral, 'MATLAB:validators:mustBeInteger');
verifyError(testCase, @maxTimeNotScalar, 'MATLAB:validation:IncompatibleSize');


function verifyEvents(testCase, evts, expected_codes)
verifyEvent(testCase, evts);
verifyEqual(testCase, length(evts), length(expected_codes));
verifyEqual(testCase, [evts.event_code], int32(expected_codes));
verifyEqual(testCase, [evts.time_us], int64(expected_codes));


function testGetAllEvents(testCase)
expected_codes = [0:length(fieldnames(getTagMap(testCase)))+1];
verifyEvents(testCase, getEvents(getFilename()), expected_codes);
verifyEvents(testCase, getEvents(getFilename(), []), expected_codes);


function testGetEventsByCode(testCase)
verifyEvents(testCase, getEvents(getFilename(), 7), 7);
verifyEvents(testCase, getEvents(getFilename(), [15, 3, 9, 3]), [3, 9, 15]);
verifyEvents(testCase, getEvents(getFilename(), [7, 10000, 8]), [7,8]);
verifyTrue(testCase, isempty(getEvents(getFilename(), 10000)));


function badTimeRange
e = getEvents(getFilename(), [], 10, 5);

function testGetEventsByTime(testCase)
max_time = length(fieldnames(getTagMap(testCase))) + 1;
verifyEvents(testCase, getEvents(getFilename(), [], 5), [5:max_time]);
verifyEvents(testCase, getEvents(getFilename(), [], -1, 10), [0:10]);
verifyEvents(testCase, getEvents(getFilename(), [], 7, 7), 7);
verifyTrue(testCase, isempty(getEvents(getFilename(), [], 100, 200)));
verifyError(testCase, @badTimeRange, 'MWorks:MWKFileError');


function testGetEventsByCodeAndTime(testCase)
verifyEvents(testCase, getEvents(getFilename(), 5:8, 7), 7:8);
verifyEvents(testCase, getEvents(getFilename(), 5:8, 0, 6), 5:6);
verifyEvents(testCase, getEvents(getFilename(), 5:8, 6, 7), 6:7);
verifyEvents(testCase, getEvents(getFilename(), 5:8, 4, 9), 5:8);


function verifySeparateArrays(testCase, codes, times, values, evts)
verifyEqual(testCase, codes, [evts.event_code]);
verifyEqual(testCase, times, [evts.time_us]);
verifyEqual(testCase, values, {evts.data});

function testSeparateOutputArrays(testCase)
evts = getEvents(getFilename());
[codes, times, values] = getEvents(getFilename());
verifySeparateArrays(testCase, codes, times, values, evts);

evts = getEvents(getFilename(), [5:10], 4, 8);
[codes, times, values] = getEvents(getFilename(), [5:10], 4, 8);
verifySeparateArrays(testCase, codes, times, values, evts);

[codes, times, values] = getEvents(getFilename(), 10000);
verifyEqual(testCase, codes, int32([]));
verifyEqual(testCase, times, int64([]));
verifyEqual(testCase, values, {});
