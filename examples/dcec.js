/*
Dead code elimination removes branches and instructions that the compiler
assures will never be reached.
For example, lets say we have the following piece of code:
    var n = 16;
    if (n < 0)
        return false;
    // More stuff down here...
After we run constant propagation, this is what that program would look like:
    if (16 < 0)
        return false;
    // More stuff down here...
The compiler evaluates '16 < 0' to 'false', so it can replace that expression for
is the result of the evaluation. This is called instruction folding,
it's exaplained in cp.js. So the code now becomes:
    if (false)
        return false;
    // More stuff down here...
The 'if' branch is never taken, but the code is still generated for it.
What dead code elimination does is simply remove the piece of code for that
conditional from the MIR control flow graph so it isn't generated.
This optimization highly benefits from parameter specialization and constant
propagation, since many more branch conditions will be folded to boolean values.
Compiling the 'dcec' function with dead code elimination is almost equivalent
to compiling the 'dcecb' function without it. Note that this comparision
requires the actual parameter to be between 10 and 100.
*/

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

function dcecb(n) {
    var limit;

    limit = 100;

    var ret;

    for (var i = 0; i < limit; i++)
        for (var j = i; j < limit; j++)
            ret = ret + j * i;

    return ret;
}

dcec(90);
dcecb(90);

