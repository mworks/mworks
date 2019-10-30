function tests = testMWFeval
tests = functiontests(localfunctions);


function testUnknownFunctionName(testCase)
verifyError(testCase, @() mworks.mwfeval('foo'), 'MWorks:UnknownFunctionName');


function testNotEnoughInputs(testCase)
verifyError(testCase, @() mworks.mwfeval, 'MWorks:NotEnoughInputs');


function testWrongNumberOfOutputs(testCase)
f = @() mworks.mwfeval('getEvents', getFilename, int32([]), int64(0), int64(1));
verifyError(testCase, f, 'MWorks:NotEnoughOutputs');


function testInvalidStringInput(testCase)
verifyError(testCase, @() mworks.mwfeval(3), 'MWorks:InvalidInput');


function scalarHasWrongType
e = mworks.mwfeval('getEvents', 'foo', int32([]), single(0), int64(1));

function scalarIsComplex
e = mworks.mwfeval('getEvents', 'foo', int32([]), int64(1+2i), int64(1));

function scalarIsNotScalar
e = mworks.mwfeval('getEvents', 'foo', int32([]), int64([1,2]), int64(1));

function testInvalidScalarInput(testCase)
verifyError(testCase, @scalarHasWrongType, 'MWorks:InvalidInput');
verifyError(testCase, @scalarIsComplex, 'MWorks:InvalidInput');
verifyError(testCase, @scalarIsNotScalar, 'MWorks:InvalidInput');


function vectorHasWrongType
e = mworks.mwfeval('getEvents', 'foo', single([]), int64(0), int64(1));

function vectorIsComplex
e = mworks.mwfeval('getEvents', 'foo', int32(1+2i), int64(0), int64(1));

function testInvalidVectorInput(testCase)
verifyError(testCase, @vectorHasWrongType, 'MWorks:InvalidInput');
verifyError(testCase, @vectorIsComplex, 'MWorks:InvalidInput');
