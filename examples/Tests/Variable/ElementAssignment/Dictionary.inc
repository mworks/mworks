%require scope


var x = 0 (scope = scope) {
    report ('x = $x')
}


protocol {
    // Initialize x
    x = {}
    x['foo'] = {}
    x['bar'] = {}
    x['foo'][1] = 'a'
    x['foo'][2] = 'b'
    x['foo'][3] = 'c'
    x['bar']['a'] = 1.5
    x['bar']['b'] = 2.5

    // Modify x
    x['foo'][1] = 'xyz'
    x['bar'] = 3.5

    // Attempt invalid assignments
    x['bar']['blah'] = 3
    x['bar']['blah'][0] = 4
}
