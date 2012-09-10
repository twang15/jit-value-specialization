function strim(v, l, u, n) {
    var elements = {};
    var eCount = 0;

    for (var i = 0; i < n; i++) {
        if ((l <= v[i]) && (v[i] <= u)) {
            elements[eCount] = v[i];
            eCount++;
        }
    }

    return elements;
}

var n = 100000;
var v = {};

for (var i = 0; i < n; i++) {
    v[i] = i % 100;
}

strim(v, 25, 70, n);
