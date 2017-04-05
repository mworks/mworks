function validateDotsData(filename)

codec = getCodecs(filename);
codec = codec.codec;
all_codes = [codec(:).code];

for i = 1:length(codec)
    if strcmp(codec(i).tagname, '#announceStimulus')
        event_codes = [codec(i).code];
        break;
    end
end

events = getEvents(filename, event_codes);
nEvents = 0;

for i = 1:length(events)
    value = events(i).data;
    if isstruct(value) && strcmp(value.type, 'moving_dots') ...
            && isfield(value, 'dots')
        data = typecast(value.dots, 'single');
        assert(length(data) == (2 * value.num_dots), ...
               'Wrong number of dots');
        
        x = data(1:2:end);
        y = data(2:2:end);
        assert(max(x.*x + y.*y) <= 1.0, 'Invalid dot coordinates');
        
        nEvents = nEvents + 1;
    end
end

assert(nEvents > 0, 'No events processed');
disp(sprintf('Processed %d events', nEvents));
