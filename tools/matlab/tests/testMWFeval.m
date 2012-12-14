function test_suite = testMWFeval
initTestSuite;


function testUnknownFunctionName
assertExceptionThrown(@() mwfeval('foo'), 'MWorks:UnknownFunctionName');


function testNotEnoughInputs
assertExceptionThrown(@() mwfeval, 'MWorks:NotEnoughInputs');


function testWrongNumberOfOutputs
f = @() mwfeval('getEvents', getFilename, uint32([]), int64(0), int64(1));
assertExceptionThrown(f, 'MWorks:NotEnoughOutputs');


function testInvalidStringInput
assertExceptionThrown(@() mwfeval(3), 'MWorks:InvalidInput');


function scalarHasWrongType
e = mwfeval('getEvents', 'foo', uint32([]), single(0), int64(1));

function scalarIsComplex
e = mwfeval('getEvents', 'foo', uint32([]), int64(1+2i), int64(1));

function scalarIsNotScalar
e = mwfeval('getEvents', 'foo', uint32([]), int64([1,2]), int64(1));

function testInvalidScalarInput
assertExceptionThrown(@scalarHasWrongType, 'MWorks:InvalidInput');
assertExceptionThrown(@scalarIsComplex, 'MWorks:InvalidInput');
assertExceptionThrown(@scalarIsNotScalar, 'MWorks:InvalidInput');


function vectorHasWrongType
e = mwfeval('getEvents', 'foo', single([]), int64(0), int64(1));

function vectorIsComplex
e = mwfeval('getEvents', 'foo', uint32(1+2i), int64(0), int64(1));

function testInvalidVectorInput
assertExceptionThrown(@vectorHasWrongType, 'MWorks:InvalidInput');
assertExceptionThrown(@vectorIsComplex, 'MWorks:InvalidInput');
