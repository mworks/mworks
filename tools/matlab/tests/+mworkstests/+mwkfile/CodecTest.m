classdef CodecTest < mworkstests.CodecTestBase
    methods
        function codec = getCodec(t)
            f = mworks.MWKFile(t.getFilename);
            codec = f.Codec;
        end
    end
end
