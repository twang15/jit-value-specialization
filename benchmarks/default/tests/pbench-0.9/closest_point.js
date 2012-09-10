// Finds in a set of cartesian points the one which is the closest to a
// given line.

function closest_point(x, y, vx, vy, n)
{
    // There are no points.
    if (n == 0)
        return undefined;

    else {
        // The initial minimum is the first point.
        var minx = vx[0];
        var miny = vy[0];
        var currDis = Math.sqrt((minx - x) * (minx - x) + (miny - y) * (miny - y));

        for (var i = 1; i < n; i++) {
            var tmpDis = Math.sqrt((vx[i] - x) * (vx[i] - x) + (vy[i] - y) * (vy[i] - y));
        
            if (tmpDis < currDis) {
                currDis = tmpDis;
                minx = vx[i];
                miny = vy[i];
            }
        }

        return [minx, miny];
    }
}

// Line coordinates.
var x = 3;
var y = 400;

var n = 100000; // Number of points.

// Points coordinates.
var vx = {};
var vy = {};

for (var i = 0; i < n; i++) {
    vx[i] = i % 100;
    vy[i] = i % 1000;
}

var cPoint = closest_point(x, y, vx, vy, n);
