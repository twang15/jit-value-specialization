function linv(n) {
    var i = 0;
    var s = 0;
    var c = 5;

    while (i < n) {
        s = i + (c * 7); 
        i++;
    };

    return s;
}

linv(100);

