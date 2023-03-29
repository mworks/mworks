classdef GetCodecTest < mworkstests.CodecTestBase
    methods
        function codec = getCodec(t)
            codec = getCodec(t.getFilename);
        end
    end
end
