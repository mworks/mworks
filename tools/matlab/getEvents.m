function [events, varargout] = getEvents(filename, codes, mintime, maxtime)
    arguments
        filename {mustBeText}
        codes (1,:) {mustBeInteger, mustBeNonnegative} = int32([])
        mintime (1,1) {mustBeInteger} = intmin('int64')
        maxtime (1,1) {mustBeInteger} = intmax('int64')
    end

    if (nargout ~= 1) && (nargout ~=3)
        error('MWorks:WrongNumberOfOutputs', ...
              '%s requires 1 or 3 output arguments', mfilename);

    end

    filename = char(filename);
    codes = int32(codes);
    mintime = int64(mintime);
    maxtime = int64(maxtime);

    f = @() mworks.mwfeval(mfilename, filename, codes, mintime, maxtime);

    if nargout == 1
        events = f();
    else
        [events, varargout{1}, varargout{2}] = f();
    end
end
