classdef GetEventsTest < mworkstests.GetEventsTestBase
    methods
        function varargout = getEvents(t, varargin)
            [varargout{1:nargout}] = getEvents(t.getFilename, varargin{:});
        end
    end

    methods (Test)
        function testNotEnoughInputs(t)
            function notEnoughInputs
                e = getEvents;
            end

            t.verifyError(@notEnoughInputs, 'MATLAB:minrhs');
        end
    end
end
