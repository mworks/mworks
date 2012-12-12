function test_suite = testTypeConversion
initTestSuite;

function testUndefined
u = getData('undefined');
assertTrue(isa(u, 'double'));
assertTrue(isempty(u));
assertEqual([], u);

function testBoolean
% Scarab doesn't have a boolean type, so booleans come out as integers
assertInteger(1, getData('bool_true'));
assertInteger(0, getData('bool_false'));

function testInteger
assertInteger(0, getData('int_zero'));
assertInteger(1, getData('int_pos'));
assertInteger(-2, getData('int_neg'));
assertInteger(intmax('int64'), getData('int_max'));
assertInteger(intmin('int64'), getData('int_min'));

function testFloat
assertFloat(0.0, getData('float_zero'));
assertFloat(1.0, getData('float_pos'));
assertFloat(-2.2, getData('float_neg'));

function testFloatInf
f = getData('float_inf');
assertTrue(isa(f, 'double'));
assertTrue(isinf(f));

function testFloatNan
f = getData('float_nan');
assertTrue(isa(f, 'double'));
assertTrue(isnan(f));

function testString
assertString('', getData('str_empty'));
assertString('foo bar', getData('str_text'));
b = getData('str_binary');
assertTrue(isa(b, 'uint8'));
assertEqual(uint8([102, 111, 111, 0, 98, 97, 114]), b);

function testList
assertList({}, getData('list_empty'));
assertList({1}, getData('list_simple'));
assertList({1, 2.0, 'three', struct('four', 4)}, getData('list_complex'));
assertList({1, {2, {3, {4, 5}}}}, getData('list_nested'));

function testDictionary
assertDict(struct(), getData('dict_empty'));
assertDict(struct('a', 1), getData('dict_simple'));
assertDict(struct('one', 1, 'Two', 2.0, 'ThReE', '3.0'), ...
           getData('dict_complex'));
