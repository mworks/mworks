function tests = testGetCodec
tests = functiontests(localfunctions);

function testAll(testCase)
codec = getCodec(getFilename());

verifyTrue(testCase, isa(codec, 'containers.Map'));
verifyEqual(testCase, double(codec.Count), 20);

verifyEqual(testCase, codec.KeyType, 'int64');
for k = keys(codec)
    verifyTrue(testCase, isa(k{1}, 'int64'));
    verifyTrue(testCase, isscalar(k{1}));
end

verifyEqual(testCase, codec.ValueType, 'char');
for v = values(codec)
    verifyTrue(testCase, isa(v{1}, 'char'));
    verifyFalse(testCase, isempty(v{1}));
end
