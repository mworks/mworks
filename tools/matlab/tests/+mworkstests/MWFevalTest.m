classdef MWFevalTest < mworkstests.TestBase
    properties
        Handle
    end

    methods (TestClassSetup)
        function createHandle(t)
            t.Handle = mworks.mwfeval('MWKFile/open', t.getFilename());
            t.addTeardown(@mworks.mwfeval, 'MWKFile/close', t.Handle);
        end
    end

    methods (Test)
        function testUnknownFunctionName(t)
            t.verifyError(@() mworks.mwfeval('foo'), ...
                          'MWorks:UnknownFunctionName');
        end

        function testNotEnoughInputs(t)
            t.verifyError(@() mworks.mwfeval, 'MWorks:NotEnoughInputs');
        end

        function testWrongNumberOfOutputs(t)
            f = @() mworks.mwfeval('MWKFile/getEvents', t.Handle);
            t.verifyError(f, 'MWorks:NotEnoughOutputs');
        end

        function testInvalidStringInput(t)
            t.verifyError(@() mworks.mwfeval(3), 'MWorks:InvalidInput');
        end

        function testInvalidScalarInput(t)
            function scalarHasWrongType
                mworks.mwfeval('MWKFile/selectEvents', t.Handle, ...
                               int32([]), single(0), int64(1));
            end

            function scalarIsComplex
                mworks.mwfeval('MWKFile/selectEvents', t.Handle, ...
                               int32([]), int64(1+2i), int64(1));
            end

            function scalarIsNotScalar
                mworks.mwfeval('MWKFile/selectEvents', t.Handle, ...
                               int32([]), int64([1,2]), int64(1));
            end

            t.verifyError(@scalarHasWrongType, 'MWorks:InvalidInput');
            t.verifyError(@scalarIsComplex, 'MWorks:InvalidInput');
            t.verifyError(@scalarIsNotScalar, 'MWorks:InvalidInput');
        end

        function testInvalidVectorInput(t)
            function vectorHasWrongType
                mworks.mwfeval('MWKFile/selectEvents', t.Handle, ...
                               single([]), int64(0), int64(1));
            end

            function vectorIsComplex
                mworks.mwfeval('MWKFile/selectEvents', t.Handle, ...
                               int32(1+2i), int64(0), int64(1));
            end

            t.verifyError(@vectorHasWrongType, 'MWorks:InvalidInput');
            t.verifyError(@vectorIsComplex, 'MWorks:InvalidInput');
        end
    end
end
