/*
Constant propagation replaces every use of a variable with its value
wherever the compiler can infer that value. For example, if we have the
following code:
    var n = 16;
    if (n < 0)
        return false;
    // More stuff down here...
At the comparison 'n < 0', the value of 'n' is known to be 16, so the compiler
can simplify the code to the following:
    if (16 < 0)
        return false;
    // More stuff down here...
Our constant propagation also does instruction folding, which basically
evaluates instructions on the MIR control flow graph and, if it evaluates to a
constant, replaces all uses of that instruction with the result of the
evaluation. For example, 'if (16 < 0)' is equivalent to the MIR fragment:
    0 constant 16
    1 constant 0
    2 compare(<) constant0 constant1
    3 test compare2
Instruction #2 evaluates to false, so, after instruction folding, that
MIR fragment would become:
    4 constant false
    3 test constant4
It's as if the original code were:
    if (false)
        return false;
    // More stuff down here...
*/

function cp(b, mask, max_mask) {
    var m = 1, c = 0;

    while (m <= max_mask) {
        if(b & m) c++;
        m <<= 1;
    }

    b &= mask;
    m = 1;
    while (m <= max_mask) {
        if(b & m) c++;
        m <<= 1;
    }

    return c;
}

cp(0x4124a14, 0xffff, 0x0fffffff);

