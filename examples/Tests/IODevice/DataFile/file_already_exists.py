def get_filename():
    return get_base_name() + 'already_exists'


def create_file():
    with open(get_filename() + '.mwk2', 'w') as fp:
        fp.write('Blah, blah, blah\n')


create_file()
