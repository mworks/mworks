classdef TypeConversionTest < mworkstests.TypeConversionTestBase
    methods
        function evts = getEvents(t, codes)
            f = mworks.MWKFile(t.getFilename);
            evts = f.getEvents(codes);
        end
    end
end
