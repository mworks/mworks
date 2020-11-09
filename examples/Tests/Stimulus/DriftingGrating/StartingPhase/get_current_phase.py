def get_current_phase(tag):
    sdu = getvar('#stimDisplayUpdate')
    for item in sdu:
        if item['name'] == tag:
            return '%.2f' % -item['current_phase']
    return ''
