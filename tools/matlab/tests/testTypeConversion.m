function tests = testTypeConversion
tests = functiontests(localfunctions);

function testUndefined(testCase)
u = getData(testCase, 'undefined');
verifyTrue(testCase, isa(u, 'double'));
verifyTrue(testCase, isempty(u));
verifyEqual(testCase, u, []);

function testBoolean(testCase)
% Neither Scarab nor SQLite has a boolean type, so booleans come out as integers
verifyInteger(testCase, getData(testCase, 'bool_true'), 1);
verifyInteger(testCase, getData(testCase, 'bool_false'), 0);

function testInteger(testCase)
verifyInteger(testCase, getData(testCase, 'int_zero'), 0);
verifyInteger(testCase, getData(testCase, 'int_pos'), 1);
verifyInteger(testCase, getData(testCase, 'int_neg'), -2);
verifyInteger(testCase, getData(testCase, 'int_max'), intmax('int64'));
verifyInteger(testCase, getData(testCase, 'int_min'), intmin('int64'));

function testFloat(testCase)
verifyFloat(testCase, getData(testCase, 'float_zero'), 0.0);
verifyFloat(testCase, getData(testCase, 'float_pos'), 1.0);
verifyFloat(testCase, getData(testCase, 'float_neg'), -2.5);

function testFloatInf(testCase)
f = getData(testCase, 'float_inf');
verifyTrue(testCase, isa(f, 'double'));
verifyTrue(testCase, isinf(f));

function testFloatNan(testCase)
f = getData(testCase, 'float_nan');
verifyTrue(testCase, isa(f, 'double'));
if endsWith(getFilename(), '.mwk2')
    % SQLite stores NaN as NULL
    verifyTrue(testCase, isempty(f));
    verifyEqual(testCase, f, []);
else
    verifyTrue(testCase, isnan(f));
end

function testString(testCase)
verifyString(testCase, getData(testCase, 'str_empty'), '');
verifyString(testCase, getData(testCase, 'str_text'), 'foo bar');
b = getData(testCase, 'str_binary');
verifyTrue(testCase, isa(b, 'uint8'));
verifyEqual(testCase, b, uint8([102, 111, 111, 0, 98, 97, 114]));

function testList(testCase)
verifyList(testCase, getData(testCase, 'list_empty'), {});
verifyList(testCase, getData(testCase, 'list_simple'), {int64(1)});
verifyList(testCase, getData(testCase, 'list_complex'), ...
           {int64(1), 2.0, 'three', struct('four', int64(4))});
verifyList(testCase, getData(testCase, 'list_nested'), ...
           {int64(1), {int64(2), {int64(3), {int64(4), int64(5)}}}});

function testDictionary(testCase)
verifyDict(testCase, getData(testCase, 'dict_empty'), struct());
verifyDict(testCase, getData(testCase, 'dict_simple'), struct('a', int64(1)));
verifyDict(testCase, getData(testCase, 'dict_complex'), ...
           struct('one', int64(1), 'Two', 2.0, 'ThReE_3', '3.0'));

function testDictionaryAsMap(testCase)
verifyDictAsMap(testCase, getData(testCase, 'dict_int_key'), {int64(1)}, {'a'});
verifyDictAsMap(testCase, getData(testCase, 'dict_empty_str_key'), ...
                containers.Map({''}, {int64(1)}).keys(), {int64(1)})
verifyDictAsMap(testCase, ...
                getData(testCase, 'dict_str_key_starts_with_non_alpha'), ...
                {'1a'}, {int64(1)});
verifyDictAsMap(testCase, ...
                getData(testCase, 'dict_str_key_contains_non_alphanum'), ...
                {'foo.1'}, {int64(1)});

function testDictionaryAsFallbackStruct(testCase)
verifyDictAsFallbackStruct(testCase, getData(testCase, 'dict_mixed_keys'), ...
                           {int64(1), 'two'}, {'a', int64(2)});
verifyDictAsFallbackStruct(testCase, ...
                           getData(testCase, 'dict_binary_str_key'), ...
                           {uint8([97, 0, 98])}, {int64(1)});
