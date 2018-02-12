function testGetCodec
codec = getCodec(getFilename());

assertTrue(isa(codec, 'containers.Map'));
assertEqual(20, double(codec.Count));

assertEqual('int64', codec.KeyType);
for k = keys(codec)
    assertTrue(isa(k{1}, 'int64'));
    assertTrue(isscalar(k{1}));
end

assertEqual('char', codec.ValueType);
for v = values(codec)
    assertTrue(isa(v{1}, 'char'));
    assertFalse(isempty(v{1}));
end
