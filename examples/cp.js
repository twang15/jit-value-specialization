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

