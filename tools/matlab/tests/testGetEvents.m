function test_suite = testGetEvents
initTestSuite;


function tooFewInputs
e = getEvents();

function tooManyInputs
e = getEvents(1, 2, 3, 4, 5);

function testBadNumberOfInputs
assertExceptionThrown(@tooFewInputs, 'MWorks:WrongNumberOfInputs');
assertExceptionThrown(@tooManyInputs, 'MWorks:WrongNumberOfInputs');


function zeroOutputs
getEvents('foo');

function twoOutputs
[a, b] = getEvents('foo');

function fourOutputs
[a, b, c, d] = getEvents('foo');

function testBadNumberOfOutputs
assertExceptionThrown(@zeroOutputs, 'MWorks:WrongNumberOfOutputs');
assertExceptionThrown(@twoOutputs, 'MWorks:WrongNumberOfOutputs');
assertExceptionThrown(@fourOutputs, 'MWorks:WrongNumberOfOutputs');


function filenameNotAString
e = getEvents(3);

function codesNotNumeric
e = getEvents('foo', true);

function minTimeNotNumeric
e = getEvents('foo', 0, true);

function maxTimeNotNumeric
e = getEvents('foo', 0, 0, true);

function testInvalidInputs
assertExceptionThrown(@filenameNotAString, 'MWorks:InvalidInput');
assertExceptionThrown(@codesNotNumeric, 'MWorks:InvalidInput');
assertExceptionThrown(@minTimeNotNumeric, 'MWorks:InvalidInput');
assertExceptionThrown(@maxTimeNotNumeric, 'MWorks:InvalidInput');


function notAFile
e = getEvents('not_a_file.mwk');

function testNonexistentFile
assertExceptionThrown(@notAFile, 'MWorks:DataFileIndexerError');


function assertEvents(expected_codes, evts)
assertEvent(evts);
assertEqual(length(expected_codes), length(evts));
assertEqual(int64(expected_codes), [evts.event_code]);
assertEqual(int64(expected_codes), [evts.time_us]);


function testGetAllEvents
expected_codes = [1:length(fieldnames(getTagMap())), 0];
assertEvents(expected_codes, getEvents(getFilename()));
assertEvents(expected_codes, getEvents(getFilename(), []));


function testGetEventsByCode
assertEvents(7, getEvents(getFilename(), 7));
assertEvents([3, 9, 15], getEvents(getFilename(), [15, 3, 9, 3]));
assertEvents([7,8], getEvents(getFilename(), [7, 10000, 8]));
assertTrue(isempty(getEvents(getFilename(), 10000)));


function badTimeRange
e = getEvents(getFilename(), [], 10, 5);

function testGetEventsByTime
max_time = length(fieldnames(getTagMap()));
assertEvents([5:max_time], getEvents(getFilename(), [], 5));
assertEvents([1:10, 0], getEvents(getFilename(), [], -1, 10));
% Non-integer time values are truncated
assertEvents([4:12], getEvents(getFilename(), [], 4.6, 12.9));
assertEvents(7, getEvents(getFilename(), [], 7, 7));
assertTrue(isempty(getEvents(getFilename(), [], 100, 200)));
assertExceptionThrown(@badTimeRange, 'MWorks:DataFileIndexerError');


function testGetEventsByCodeAndTime
assertEvents(7:8, getEvents(getFilename(), 5:8, 7));
assertEvents(5:6, getEvents(getFilename(), 5:8, 0, 6));
assertEvents(6:7, getEvents(getFilename(), 5:8, 6, 7));
assertEvents(5:8, getEvents(getFilename(), 5:8, 4, 9));


function assertSeparateArrays(evts, codes, times, values)
assertEqual([evts.event_code], int64(codes));
assertEqual([evts.time_us], times);
assertEqual({evts.data}, values);

function testSeparateOutputArrays
evts = getEvents(getFilename());
[codes, times, values] = getEvents(getFilename());
assertSeparateArrays(evts, codes, times, values);

evts = getEvents(getFilename(), [5:10], 4, 8);
[codes, times, values] = getEvents(getFilename(), [5:10], 4, 8);
assertSeparateArrays(evts, codes, times, values);

[codes, times, values] = getEvents(getFilename(), 10000);
assertEqual(int32([]), codes);
assertEqual(int64([]), times);
assertEqual({}, values);
