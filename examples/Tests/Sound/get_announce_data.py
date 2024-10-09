import hashlib


def get_announce_data():
    data = getvar('#announceSound')
    filename = data.get('filename')
    if filename:
        data['filename'] = filename.split('/')[-1]
    return data


def compute_file_hash(filename):
    with open(filename, 'rb') as fp:
        return hashlib.file_digest(fp, hashlib.sha1).hexdigest()
