function reverse_codec = getReverseCodec(filename)

full_codecs = getCodecs(filename);
full_codec = full_codecs(1).codec;

reverse_codec = containers.Map({full_codec(:).tagname}, [full_codec(:).code]);
