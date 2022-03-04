function printErrorStack(errorStruct)
% Print Error Stack: adapted from Malcom Wood's PLE script on Matlab Central. A typical use-case for this script is
% to call it in a catch statement after an error has occurred to get a full, hyperlinked stack trace in the console. 
%
% USAGE:
%   printErrorStack
%   printErrorStack(errorStruct)
%
% NOTE: If an error structure is supplied, it is used instead of lasterror
%
% Copyright 2007 The MathWorks, Inc.
%
%
% MH 100204 - remove hyperlinks in output for display in matlab
% plugin

    if(nargin < 1)
        errorStruct = lasterror;
    end

    if(isempty(errorStruct.message))
        fprintf(1,'No error message stored\n');
        return;
    end

    fprintf(1,'Error: %s (%s)\n', errorStruct.message, errorStruct.identifier);    
    for i=1:numel(errorStruct.stack)        
        printErrorStackFrame(errorStruct.stack(i));
    end    
    fprintf(1,'\n');
    
end


% Prints a single frame of the error stack
function printErrorStackFrame(stackFrame)

    functionName = stackFrame.name;
    filePath = which(stackFrame.file);
    errorHyperLink = sprintf('matlab:opentoline(''%s'',%d)', filePath, stackFrame.line);
    
    [ignore_dir, fileName, ext] = fileparts(filePath);
    fileNameWithExt = strcat(fileName, ext);

    if(strcmp(fileName, functionName))
        % Error is in main function of this file 
        fprintf(1,'    In %s at %d\n', fileNameWithExt, stackFrame.line);
    else
        % Error is in a subfunction of this file
        fprintf(1,'    In %s within %s() at %d\n', fileNameWithExt, functionName, stackFrame.line);          end        
end
