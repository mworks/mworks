function [retval] = hist_example(data_struct, input)

if nargin == 1, input = []; end

codec = data_struct.event_codec;


collection = input;

code = -1;
% find the variable labled "some_Var"
for i=1:size(codec, 1)
    tagname = codec(i).tagname;
    if(strcmp(tagname, 'some_Var') == 1)
        code = codec(i).code;
    end
    
end

events = data_struct.events;

for i=1:size(events,1)
    current_code = events(i).event_code;
    if(current_code == code)
        collection = [collection events(i).data];
    end
end

system_dependent(12,'off');
hist(collection);
drawnow;


retval = collection;
