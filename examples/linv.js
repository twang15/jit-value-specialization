/*
Loop inversion transforms a while loop into a do-while loop with an outter
conditional.
For example, the JavaScript fragment:
    while (i < 16) {
        i++;
    }
would be transformed, with loop inversion, into:
    if (i < 16) {
         do {
           i++;
         } while (i < 16);
    }
Unlike the while loop, a do-while loop will always be executed at least once,
which allows the compiler to do loop-invariant code motion.
In some cases, constant propagation with instruction folding and dead code
elimination lead to the removal of the the generated 'if' statement.
Compiling the 'linv' function with loop inversion is almost equivalent to
compiling 'linvb' without loop inversion.
*/

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

function linvb(n) {
    var i = 0;
    var s = 0;
    var c = 5;

    if (i < n) {
        do {
            s = i + (c * 7);
            i++;
        } while (i < n);
    }

    return s;
}

linv(100);
linvb(100);

