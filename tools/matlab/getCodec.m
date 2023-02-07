function codec = getCodec(filename)
    file = mworks.MWKFile(filename);
    codec = file.Codec;
end
