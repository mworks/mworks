function varargout = getEvents(filename, varargin)
    file = mworks.MWKFile(filename);
    [varargout{1:nargout}] = file.getEvents(varargin{:});
end
