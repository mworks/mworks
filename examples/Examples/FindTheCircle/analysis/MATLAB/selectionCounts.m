function selectionCounts(filename)

addpath('/Library/Application Support/MWorks/Scripting/MATLAB');

r_codec = getReverseCodec(filename);
red_code = r_codec('red_selected');
green_code = r_codec('green_selected');
blue_code = r_codec('blue_selected');

red_count = 0;
green_count = 0;
blue_count = 0;

events = getEvents(filename, [red_code, green_code, blue_code]);

for i = 1:length(events)
    evt = events(i);
    if evt.data
        switch evt.event_code
        case red_code
            red_count = red_count + 1;
        case green_code
            green_count = green_count + 1;
        case blue_code
            blue_count = blue_count + 1;
        end
    end
end

b = bar([red_count, green_count, blue_count]);
b.FaceColor = 'flat';
b.CData = [1, 0, 0; 0, 1, 0; 0, 0, 1];
xticklabels({'Red', 'Green', 'Blue'});
title('Selection Counts');
