classdef CodecTestBase < mworkstests.TestBase
    methods (Abstract)
        codec = getCodec(t)
    end

    methods (Test)
        function testAll(t)
            codec = t.getCodec;

            t.verifyClass(codec, 'containers.Map');
            t.verifyEqual(double(codec.Count), 20);

            t.verifyEqual(codec.KeyType, 'int64');
            for k = keys(codec)
                t.verifyClass(k{1}, 'int64');
                t.verifySize(k{1}, [1 1]);
            end

            t.verifyEqual(codec.ValueType, 'char');
            for v = values(codec)
                t.verifyClass(v{1}, 'char');
                t.verifyNotEmpty(v{1});
            end
        end
    end
end
