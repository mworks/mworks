function testGetReverseCodec
reverse_codec = getReverseCodec(getFilename());

assertTrue(isa(reverse_codec, 'containers.Map'));
assertEqual(20, double(reverse_codec.Count));

assertEqual('char', reverse_codec.KeyType);
for k = keys(reverse_codec)
    assertTrue(isa(k{1}, 'char'));
    assertFalse(isempty(k{1}));
end

assertEqual('int64', reverse_codec.ValueType);
for v = values(reverse_codec)
    assertTrue(isa(v{1}, 'int64'));
    assertTrue(isscalar(v{1}));
end
