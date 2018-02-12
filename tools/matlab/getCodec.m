function codec = getCodec(filename)

full_codecs = getCodecs(filename);
full_codec = full_codecs(1).codec;

codec = containers.Map([full_codec(:).code], {full_codec(:).tagname});
