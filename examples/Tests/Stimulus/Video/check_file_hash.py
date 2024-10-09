import hashlib


def check_file_hash():
    sdu = getvar('#stimDisplayUpdate')
    assert len(sdu) == 1
    file_hash = sdu[0]['file_hash']

    with open(getvar('path'), 'rb') as fp:
        computed_hash = hashlib.file_digest(fp, hashlib.sha1).hexdigest()

    assert file_hash == computed_hash
