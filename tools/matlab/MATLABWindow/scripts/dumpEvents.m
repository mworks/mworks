function [retval] = dumpEvents(data_struct, input)
%DUMPEVENTS (MW): log the MW events getting passed to Matlab
%
%   [retval] = dumpEvents(data_struct, input)
%   Dump MWorks events passed to Matlab to a file
%   Currently the file is ~/Desktop/MatlabOutput.txt, which is appended every
%   time a new set of events is passed to Matlab.  The time of each event is
%   given in milliseconds.  If an event with the tag 'trialStart' 
%   is received, event times are referenced to that event
%
%   This can be used as the script run by the MW MatlabWindow
%
% maunsell 100101: first ver
% histed 100115: cleanups and doc
%
%$Id$

if nargin == 1
	input.count = 0;
end

% dump the events passed to Matlab by in one call from MWorks

codes = [data_struct.event_codec(:).code];
trialStartTimeMS = -1;

fid = fopen('~/Desktop/MatlabOutput.txt', 'a');
fprintf(fid, sprintf('\nIteration %d, dumping %d MWorks events:\n', input.count, length(data_struct.events)));

for e = 1:length(data_struct.events)
	event = data_struct.events(e);
	index = find(codes == event.event_code);
	if trialStartTimeMS >= 0
		eventTimeString = sprintf('+%d ms',	round(event.time_us / 1000 - trialStartTimeMS));
	else
		eventTimeString = sprintf(' %d ms', round(event.time_us / 1000));
        end

        d = event.data;
        dStr = num2str(d);
        %fprintf(fid, '%s', evalc('disp(event)'));
	fprintf(fid, sprintf('  %-20s at %8s with data %s', ...
                             data_struct.event_codec(index).tagname, ...
                             eventTimeString, ...
                             dStr));
	if strcmp('trialStart', data_struct.event_codec(index).tagname) == 1
		trialStartTimeMS = round(event.time_us / 1000);
	end
	if trialStartTimeMS < 0 & e > 1
		fprintf(fid, sprintf('\t(delta %d ms)\n', round((event.time_us - data_struct.events(e - 1).time_us) / 1000)));
	else
		fprintf(fid, '\n');
	end
end

fclose(fid);

retval.count = input.count + 1;
return;
