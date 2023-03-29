classdef GetReverseCodecTest < mworkstests.ReverseCodecTestBase
    methods
        function reverseCodec = getReverseCodec(t)
            reverseCodec = getReverseCodec(t.getFilename());
        end
    end
end
