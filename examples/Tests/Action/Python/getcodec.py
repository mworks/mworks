codec = getcodec()
assert isinstance(codec, dict)
assert len(codec) > 0

codes = codec.keys()
assert all(isinstance(c, int) and (c >= 0) for c in codes)

names = codec.values()
assert all(isinstance(n, str) and (len(n) > 0) for n in names)

# System variables
assert '#state_system_mode' in names
assert '#stimDisplayUpdate' in names

# Experiment variables
assert 'foo' in names
assert 'bar' in names

# Reverse codec
reverse_codec = get_reverse_codec()
assert isinstance(reverse_codec, dict)
assert reverse_codec == dict((n, c) for c, n in codec.items())
