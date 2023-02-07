classdef MWKFile < handle
    properties
        Filename {mustBeText} = ''
    end

    properties (Transient, Access = protected)
        Handle uint64 = 0
    end

    properties (Transient, SetAccess = protected)
        Codec = []
        ReverseCodec = []
    end

    methods
        function obj = MWKFile(filename)
            if nargin > 0
                obj.Filename = filename;
                obj.open;
            end
        end

        function delete(obj)
            obj.close;
        end

        function open(obj)
            if obj.Handle ~= 0
                % File is already open
                return
            end

            obj.Handle = mworks.mwfeval('MWKFile/open', char(obj.Filename));

            codecCode = mworks.ReservedEventCode.RESERVED_CODEC_CODE;
            [~, ~, rawCodecs] = obj.getEvents(codecCode);
            if ~isempty(rawCodecs)
                rawCodec = rawCodecs{1};
                assert(all(cellfun(@(rc) isequal(rc, rawCodec), rawCodecs)), ...
                       'MWorks:MWKFileError', ...
                       'File contains conflicting codecs');
                codes = [];
                tagnames = {};
                for code = keys(rawCodec)
                    code = code{1};
                    codes = [codes, code];
                    tagnames = [tagnames, rawCodec(code).tagname];
                end
                obj.Codec = containers.Map(codes, tagnames);
                obj.ReverseCodec = containers.Map(tagnames, codes);
            end
        end

        function close(obj)
            if obj.Handle == 0
                % File is already closed
                return
            end

            mworks.mwfeval('MWKFile/close', obj.Handle);
            obj.Handle = 0;

            obj.Codec = [];
            obj.ReverseCodec = [];
        end

        function numEvents = getNumEvents(obj)
            numEvents = mworks.mwfeval('MWKFile/getNumEvents', obj.Handle);
        end

        function minTime = getMinTime(obj)
            minTime = mworks.mwfeval('MWKFile/getMinTime', obj.Handle);
        end

        function maxTime = getMaxTime(obj)
            maxTime = mworks.mwfeval('MWKFile/getMaxTime', obj.Handle);
        end

        function selectEvents(obj, codes, minTime, maxTime)
            arguments
                obj
                codes (1,:) {mustBeInteger, mustBeNonnegative} = int32([])
                minTime (1,1) {mustBeInteger} = intmin('int64')
                maxTime (1,1) {mustBeInteger} = intmax('int64')
            end

            codes = int32(codes);
            minTime = int64(minTime);
            maxTime = int64(maxTime);

            mworks.mwfeval('MWKFile/selectEvents', ...
                           obj.Handle, codes, minTime, maxTime);
        end

        function [evt, varargout] = nextEvent(obj)
            if (nargout ~= 1) && (nargout ~=3)
                error('MWorks:WrongNumberOfOutputs', ...
                      'nextEvent requires 1 or 3 output arguments');

            end

            f = @() mworks.mwfeval('MWKFile/nextEvent', obj.Handle);
            if nargout == 1
                evt = f();
            else
                [evt, varargout{1}, varargout{2}] = f();
            end
        end

        function [evts, varargout] = getEvents(obj, varargin)
            if (nargout ~= 1) && (nargout ~=3)
                error('MWorks:WrongNumberOfOutputs', ...
                      'getEvents requires 1 or 3 output arguments');

            end

            obj.selectEvents(varargin{:});

            f = @() mworks.mwfeval('MWKFile/getEvents', obj.Handle);
            if nargout == 1
                evts = f();
            else
                [evts, varargout{1}, varargout{2}] = f();
            end
        end
    end
end
