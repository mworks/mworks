function [events, varargout] = getEvents(filename, codes, mintime, maxtime)

if (nargin < 1) || (nargin > 4)
    error('MWorks:WrongNumberOfInputs', ...
          '%s requires 1-4 input arguments', mfilename);
    
end

if (nargout ~= 1) && (nargout ~=3)
    error('MWorks:WrongNumberOfOutputs', ...
          '%s requires 1 or 3 output arguments', mfilename);
    
end

validateattributes(filename, {'char'}, {}, 1)

if nargin > 1
    validateattributes(codes, {'numeric'}, {'integer', 'nonnegative'}, 2)
    codes = uint32(codes);
else
    codes = uint32([]);
end

if nargin > 2
    validateattributes(mintime, {'numeric'}, {'integer', 'scalar'}, 3)
    mintime = int64(mintime);
else
    mintime = intmin('int64');
end

if nargin > 3
    validateattributes(maxtime, {'numeric'}, {'integer', 'scalar'}, 4)
    maxtime = int64(maxtime);
else
    maxtime = intmax('int64');
end

f = @() mworks.mwfeval(mfilename, filename, codes, mintime, maxtime);

if nargout == 1
    events = f();
else
    varargout = cell(1, 2);
    [events, varargout{1}, varargout{2}] = f();
end
