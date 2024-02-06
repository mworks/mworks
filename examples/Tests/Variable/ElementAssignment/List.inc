%require scope


var x = 0 (scope = scope) {
    report ('x = $x')
}

var i = 0
var j = 0


protocol {
    //
    // Initialize x
    //

    x = []

    i = 0
    while (i < 2) {
        x[i] = []
        i += 1
    }

    i = 0
    while (i < 2) {
        j = 0
        while (j < 3) {
            x[i][j] = 0
            j += 1
        }
        i += 1
    }

    //
    // Modify x
    //

    i = 0
    while (i < 2) {
        j = 0
        while (j < 3) {
            x[i][j] = 3*i + j + 1
            j += 1
        }
        i += 1
    }

    //
    // Attempt invalid assignments
    //

    x[0][1][2] = 3
    x[0][1][2][3] = 4
}
