function tests = testGetReverseCodec
tests = functiontests(localfunctions);

function testAll(testCase)
reverse_codec = getReverseCodec(getFilename());

verifyTrue(testCase, isa(reverse_codec, 'containers.Map'));
verifyEqual(testCase, double(reverse_codec.Count), 20);

verifyEqual(testCase, reverse_codec.KeyType, 'char');
for k = keys(reverse_codec)
    verifyTrue(testCase, isa(k{1}, 'char'));
    verifyFalse(testCase, isempty(k{1}));
end

verifyEqual(testCase, reverse_codec.ValueType, 'int64');
for v = values(reverse_codec)
    verifyTrue(testCase, isa(v{1}, 'int64'));
    verifyTrue(testCase, isscalar(v{1}));
end
