classdef GetEventsTest < mworkstests.GetEventsTestBase
    methods
        function varargout = getEvents(t, varargin)
            f = mworks.MWKFile(t.getFilename);
            [varargout{1:nargout}] = f.getEvents(varargin{:});
        end
    end
end
