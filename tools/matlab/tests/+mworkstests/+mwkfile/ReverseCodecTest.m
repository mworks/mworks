classdef ReverseCodecTest < mworkstests.ReverseCodecTestBase
    methods
        function reverseCodec = getReverseCodec(t)
            f = mworks.MWKFile(t.getFilename);
            reverseCodec = f.ReverseCodec;
        end
    end
end
