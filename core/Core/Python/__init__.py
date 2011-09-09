import hashlib


def file_hash(filename):
    with open(filename, 'rb') as fp:
        return hashlib.sha1(fp.read()).hexdigest()
