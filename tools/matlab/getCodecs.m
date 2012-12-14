function codecs = getCodecs(filename)

all_times = {};
all_codecs = {};

for evt = getEvents(filename, 0)
    all_times = [all_times, evt.time_us];
    
    raw_codec = evt.data;
    codec = struct([]);
    
    for code = keys(raw_codec)
        code = code{1};
        info = setfield(raw_codec(code), 'code', code);
        if isempty(codec)
            codec = orderfields(info);
        else
            codec = [codec, info];
        end
    end
    
    all_codecs = [all_codecs, codec];
end

codecs = struct('time_us', all_times, 'codec', all_codecs);
