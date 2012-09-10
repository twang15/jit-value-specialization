function split_line(vx, vy, n, a, b)                                                                                                                   
{
    //There are no points in the vector
    if (n == 0)
        return undefined;

    else {
        var above = {};
        var below = {};
        var numAbove = 0;
        var numBelow = 0;

        for (var i = 0; i < n; i ++) {
            var ly = a * vx[i] + b;

            //The point is above
            if (ly < vy[i]) {
                above[numAbove] = [vx[i], vy[i]];
                numAbove++;
            
            //The point is below
            } else if (ly > vy[i]) {
                below[numBelow] = [vx[i], vy[i]];
                numBelow++;
            }
        }
        
        return new Array(above, below);
    }
}

var vx = {};
var vy = {};
var n = 100000;

for (var i = 0; i < n; i++) {
    vx[i] = i % 100;
    vy[i] = i % 1000;
}

var points = split_line(vx, vy, n, 5, 3);
