// Finds the k nearest points to a given line.

function k_nearest_neighbors(x, y, vx, vy, n, k)
{
    //There are fewer points than neighbors
    if (n < k)
        return undefined;

    else {
        var result = {};        

        for (var i = 0; i < k; i++) {
            //The initial minimum is the first point
            var minx = vx[0];
            var miny = vy[0];
            var currDis = Math.sqrt((minx - x) * (minx - x) + (miny - y) * (miny - y));
            var currIdx = 0;

            for (var j = 1; j < (n - i); j++) {
                var tmpDis = Math.sqrt((vx[j] - x) * (vx[j] - x) + (vy[j] - y) * (vy[j] - y));
                
                if (tmpDis < currDis) {
                    currDis = tmpDis;
                    currIdx = j;
                    minx = vx[j];
                    miny = vy[j];
                }
            }

            //Removes the nearest point
            vx.splice(currIdx, 1);
            vy.splice(currIdx, 1);

            result[i] = [minx, miny];
        }

        return result;
    }
}

// Line coordinates.
var x = 3;
var y = 400;

var k = 2000; // Number of neighbors.

var n = 3100; // Number of points.

// Points coordinates.
var vx = new Array();
var vy = new Array();

for (var i = 0; i < n; i++) {
    vx[i] = i % 100;
    vy[i] = i % 1000;
}

var cPoints = k_nearest_neighbors(x, y, vx, vy, n, k);
