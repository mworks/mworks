function codecs = getCodecs(filename)
    file = mworks.MWKFile(filename);

    allTimes = {};
    allCodecs = {};

    for evt = file.getEvents(mworks.ReservedEventCode.RESERVED_CODEC_CODE)
        allTimes = [allTimes, evt.time_us];

        rawCodec = evt.data;
        codec = struct([]);

        for code = keys(rawCodec)
            code = code{1};
            info = setfield(rawCodec(code), 'code', code);
            codec = [codec, orderfields(info)];
        end

        allCodecs = [allCodecs, codec];
    end

    codecs = struct('time_us', allTimes, 'codec', allCodecs);
end
