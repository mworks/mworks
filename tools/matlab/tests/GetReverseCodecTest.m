classdef GetReverseCodecTest < TestBase
    methods (Test)
        function testAll(t)
            reverseCodec = getReverseCodec(t.getFilename());

            t.verifyClass(reverseCodec, 'containers.Map');
            t.verifyEqual(double(reverseCodec.Count), 20);

            t.verifyEqual(reverseCodec.KeyType, 'char');
            for k = keys(reverseCodec)
                t.verifyClass(k{1}, 'char');
                t.verifyNotEmpty(k{1});
            end

            t.verifyEqual(reverseCodec.ValueType, 'int64');
            for v = values(reverseCodec)
                t.verifyClass(v{1}, 'int64');
                t.verifySize(v{1}, [1 1]);
            end
        end
    end
end
