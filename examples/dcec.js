function dcec(n) {
    var limit;

    if (n < 0 || n > 1000)
        return 0;

    if (n < 10)
        limit = 10;
    else if (n < 100)
        limit = 100;
    else if (n < 1000)
        limit = 1000;

    var ret;

    for (var i = 0; i < limit; i++)
        for (var j = i; j < limit; j++)
            ret = ret + j * i;

    return ret;
}

dcec(90);

