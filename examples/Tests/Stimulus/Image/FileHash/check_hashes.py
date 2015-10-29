import hashlib


stims = getvar('#stimDisplayUpdate')
assert isinstance(stims, list)
assert len(stims) == 5

all_hashes = set()

for info in stims:
    assert isinstance(info, dict)
    filehash = info['file_hash']
    assert isinstance(filehash, str)
    assert len(filehash) == 40

    with open(info['filename'], 'rb') as fp:
        computed_hash = hashlib.sha1(fp.read()).hexdigest()
    assert filehash == computed_hash

    all_hashes.add(filehash)

# Confirm that all hashes are unique
assert len(all_hashes) == 5
