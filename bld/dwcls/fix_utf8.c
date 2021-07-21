// this returns the new length of a string, assuming the string
// is utf-8, such that the new length trims off the last malformed
// utf-8 character. found on github, author unknown.
//
//
/* UTF-8 character might be encoded in up to 4 bytes according to RFC
 * 3629 so to truncate it correctly care needs to be taken for such
 * characters. ASCII character are in single byte using 7 bits (max is
 * 127) so any multi-byte character has first byte higher then
 * 127. Because first byte in multi-byte sequence is encoding how many
 * bytes this sequence has we can easily check for it looking for the
 * first byte in sequence and see how many bytes more to truncate.
 * Everything is much cleare if you see how the bits are encoded in
 * the sequences, see the wikipedia for it.
 * http://en.wikipedia.org/wiki/UTF-8
 */
int
dwyco_fix_utf8(char *buf, size_t len)
{
    char *b = 0;

    /* Might truncate up to 3 bytes*/
    b = buf + len - 3;

    /* Is the last byte part of multi-byte sequence? */
    if (b[2] & 0x80)
    {
        /*Is the last byte in buffer the first byte in a new
         * multi-byte sequence? */
        if (b[2] & 0x40) return len - 1;
        /* Is it a 3 byte sequence? */
        else if ((b[1] & 0xe0) == 0xe0) return len - 2;
        /* Is it a 4 byte sequence? */
        else if ((b[0] & 0xf0) == 0xf0) return len - 3;
    }
}

#ifdef TEST
#include <stdio.h>
#include <string.h>
int
main(int argc, char *argv[])
{
    /*∮ E⋅da = Q,  n → ∞, 𐍈∑ f(i) = ∏ g(i)*/
    char utf8[] = {'\xe2','\x88','\xae','\x20','\x45','\xe2','\x8b',
                   '\x85','\x64','\x61','\x20','\x3d','\x20','\x51',
                   '\x2c','\x20','\x20','\x6e','\x20','\xe2','\x86',
                   '\x92','\x20','\xe2','\x88','\x9e','\x2c','\x20',
                   '\xf0','\x90','\x8d','\x88','\xe2','\x88','\x91',
                   '\x20','\x66','\x28','\x69','\x29','\x20','\x3d',
                   '\x20','\xe2','\x88','\x8f','\x20','\x67','\x28',
                   '\x69','\x29', '\0'};

    char trunc_utf8[31];

    printf("%s \n", utf8);
    strncpy(trunc_utf8, utf8, 31);
    printf("%s \n", trunc_utf8);
    trunc_utf8[fix_utf8(trunc_utf8, strlen(trunc_utf8))] = '\0';
    printf("%s \n", trunc_utf8);

    return 0;
}
#endif
