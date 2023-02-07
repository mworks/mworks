function reverseCodec = getReverseCodec(filename)
    file = mworks.MWKFile(filename);
    reverseCodec = file.ReverseCodec;
end
