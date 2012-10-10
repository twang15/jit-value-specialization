/*
Array accesses are bounds checked in JavaScript. An out-of-bounds array access
returns the 'undefined' constant.
Bounds check elimination starts by finding an induction variable in a loop - a
variable that gets incremented or decremented during every iteration of that
loop. It then searchs for array accesses where the index is the induction
variable.
If the bounds of the induction variable are within the bounds of the array, then
the bounds check for that particular access can be removed. For example,
consider the following JavaScript fragment:
    for (var i = 0; i < 16; i++)
        n = array[i];
The lower bound of the induction variable - 'i' - is 0 and the upper bound is 15.
The lower bound of the array is always 0, so the lower bound on 'i' is greater
than or equal to the lower bound of the array - that one is okay.
If the upper bound of the array is greated than or equal to 15, then the upper bound
is also okay, and the array access need not be bounds checked.
*/

function bce(array, limit) {
    var n = 0;

    for (var i = 0; i < limit; i++)
        for (var j = i; j < limit; j++)
            n = n + array[j];

    return n;
}

var array = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11];
bce(array, 12);

