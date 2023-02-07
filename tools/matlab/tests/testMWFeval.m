function tests = testMWFeval
    tests = functiontests(localfunctions);
end


function setupOnce(testCase)
    testCase.TestData.handle = mworks.mwfeval('MWKFile/open', getFilename());
end


function teardownOnce(testCase)
    mworks.mwfeval('MWKFile/close', testCase.TestData.handle);
end


function testUnknownFunctionName(testCase)
    verifyError(testCase, @() mworks.mwfeval('foo'), ...
                'MWorks:UnknownFunctionName');
end


function testNotEnoughInputs(testCase)
    verifyError(testCase, @() mworks.mwfeval, 'MWorks:NotEnoughInputs');
end


function testWrongNumberOfOutputs(testCase)
    f = @() mworks.mwfeval('MWKFile/getEvents', testCase.TestData.handle);
    verifyError(testCase, f, 'MWorks:NotEnoughOutputs');
end


function testInvalidStringInput(testCase)
    verifyError(testCase, @() mworks.mwfeval(3), 'MWorks:InvalidInput');
end


function testInvalidScalarInput(testCase)
    function scalarHasWrongType
        mworks.mwfeval('MWKFile/selectEvents', testCase.TestData.handle, ...
                       int32([]), single(0), int64(1));
    end

    function scalarIsComplex
        mworks.mwfeval('MWKFile/selectEvents', testCase.TestData.handle, ...
                       int32([]), int64(1+2i), int64(1));
    end

    function scalarIsNotScalar
        mworks.mwfeval('MWKFile/selectEvents', testCase.TestData.handle, ...
                       int32([]), int64([1,2]), int64(1));
    end

    verifyError(testCase, @scalarHasWrongType, 'MWorks:InvalidInput');
    verifyError(testCase, @scalarIsComplex, 'MWorks:InvalidInput');
    verifyError(testCase, @scalarIsNotScalar, 'MWorks:InvalidInput');
end


function testInvalidVectorInput(testCase)
    function vectorHasWrongType
        mworks.mwfeval('MWKFile/selectEvents', testCase.TestData.handle, ...
                       single([]), int64(0), int64(1));
    end

    function vectorIsComplex
        mworks.mwfeval('MWKFile/selectEvents', testCase.TestData.handle, ...
                       int32(1+2i), int64(0), int64(1));
    end

    verifyError(testCase, @vectorHasWrongType, 'MWorks:InvalidInput');
    verifyError(testCase, @vectorIsComplex, 'MWorks:InvalidInput');
end
