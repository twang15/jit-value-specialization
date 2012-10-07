function bce(array, limit) {
    var n = 0;

    for (var i = 0; i < limit; i++)
        for (var j = i; j < limit; j++)
            n = n + array[j];

    return n;
}

var array = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11];
bce(array, 12);

