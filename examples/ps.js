/*
When parameter specialization is applied, it's as if one local variable were
created for each formal parameter, initializing each of them with their
respective actual parameters.
The good thing about this is that the compiler can do more with local variables
than it can with with parameters since the initial value of locals are known at
compile-time. Constant propagation, for example, can act on local variables, but
not parameters.
Take a look at the 'ps' and 'psb' functions down below.
Compiling the 'ps' function with parameter specialization is almost
equivalent to compiling 'psb' without parameter specialization.
The initial control flow graph of 'psb' compiled without parameter
specialization is included in the jit-value-specialization repository.
*/

function ps(a, b, c) {
    var n = a;

    while (n < b) {
        n = n + c;
    }

    return n;
}

function psb() {
    var a = 32;
    var b = 1024;
    var c = 2;

    var n = a;
    
    while (n < b) {
        n = n + c;
    }

    return n;
}

ps(32, 1024, 2);
psb();

