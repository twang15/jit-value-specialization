function matrix_mul(m1, m2, k, l, m) {
    var resultant = {};

    for(var i1 = 0; i1 < k; i1++) { // m1Row
        resultant[i1] = {};
        
        for(var i2 = 0; i2 < m; i2++) { // m2Column
            resultant[i1][i2] = 0;
        }
    }

    for(var i1 = 0; i1 < k; i1++) { // m1Row
        for(var i2 = 0; i2 < m; i2++) { // m2Column
            for(var i3 = 0; i3 < l; i3++) { // m1Column
                resultant[i1][i2] += m1[i1][i3] * m2[i3][i2];
            }
        }
    }

    return resultant;
}

var k = 100;
var l = 100;
var m = 100;

var m1 = {};
var m2 = {};

for (var i = 0; i < k; i++)
{
    m1[i] = {};

    for (var j = 0; j < l; j++)
        m1[i][j] = i + j;
}

for (var i = 0; i < l; i++)
{
    m2[i] = {};

    for (var j = 0; j < m; j++)
        m2[i][j] = i + j;
}

matrix_mul(m1, m2, k, l, m);
