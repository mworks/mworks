def get_announce_data():
    data = getvar('#announceSound')
    filename = data.get('filename')
    if filename:
        data['filename'] = filename.split('/')[-1]
    return data
