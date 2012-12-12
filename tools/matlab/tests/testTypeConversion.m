function test_suite = testTypeConversion
initTestSuite;

function assertInteger(expected, actual)
assertTrue(isa(actual, 'int64'));
assertEqual(int64(expected), actual);

function data = getData(code)
if ischar(code)
    tagmap = getData(0);
    data = getData(double(getfield(tagmap, code)));
else
    e = getEvents(getenv('MW_MATLAB_TEST_FILENAME'), [code]);
    assertTrue(isstruct(e));
    assertTrue(isvector(e));
    assertEqual(1, length(e));
    assertEqual(3, length(fieldnames(e)));
    assertInteger(code, e.event_code);
    assertInteger(code, e.time_us);
    data = e.data;
end

function testUndefined
assertTrue(isempty(getData('undefined')));

function testInteger
assertInteger(0, getData('int_zero'));
assertInteger(1, getData('int_pos'));
assertInteger(-2, getData('int_neg'));
