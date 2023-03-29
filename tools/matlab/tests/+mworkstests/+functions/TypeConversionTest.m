classdef TypeConversionTest < mworkstests.TypeConversionTestBase
    methods
        function evts = getEvents(t, codes)
            evts = getEvents(t.getFilename, codes);
        end
    end
end
