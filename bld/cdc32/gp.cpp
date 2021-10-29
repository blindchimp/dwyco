
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/* C++ code produced by gperf version 3.0.4 */
/* Command-line: gperf -L C++ -t -l -G  */
/* Computed positions: -k'1-3' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

struct fn_rules  {
    const char *name;
    char **pfx;
    int cacheable;
    DwString *cached_val;
};

#define TOTAL_KEYWORDS 51
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 10
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 121
/* maximum key range = 118, duplicates = 0 */

class Perfect_Hash
{
private:
    static inline unsigned int hash (const char *str, unsigned int len);
public:
    static struct fn_rules *in_word_set (const char *str, unsigned int len);
};

inline unsigned int
Perfect_Hash::hash ( const char *str,  unsigned int len)
{
    static unsigned char asso_values[] =
    {
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122,   0, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122,  45,  30,  25,
        45,   0,  40,   5,   0,  20,  45,   0,  36,  45,
        5,   5,  25,   5,  35,   0,  10,   0,  35,   0,
        15,   1,  55, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
        122, 122, 122, 122, 122, 122, 122
    };
     int hval = len;

    switch (hval)
    {
    default:
        hval += asso_values[(unsigned char)str[2]+1];
    /*FALLTHROUGH*/
    case 2:
        hval += asso_values[(unsigned char)str[1]];
    /*FALLTHROUGH*/
    case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
    return hval;
}

static unsigned char lengthtable[] =
{
    0,  0,  0,  0,  4,  4,  0,  2,  8,  4,  5,  6,  7,  0,
    4,  0,  0,  0,  0,  4,  9,  0,  0,  0,  4,  0,  0,  2,
    0,  4,  5,  6,  0,  0,  4,  5,  6,  0,  0,  4, 10,  6,
    0,  0,  4,  5,  0,  0,  0,  4,  5,  0,  0,  0,  4,  5,
    0,  0,  8,  4,  5,  0,  0,  0,  4,  4,  0,  0,  3,  4,
    0,  0,  0,  0,  4,  0,  0,  0,  0,  4,  5,  0,  0,  0,
    4,  0,  0,  6,  0,  4,  0,  0,  0,  0,  4,  0,  0,  7,
    0,  4,  5,  6,  6,  0,  4,  0,  0,  0,  8,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  4,  0,  6
};

static struct fn_rules wordlist[] =
{
    {""}, {""}, {""}, {""},
    {".urd", &User_prefix, 0},
    {".exe", &System_prefix, 0},
    {""},
    {".q", &User_prefix, 0},
    {"servers2", &System_prefix, 1},
    {".enc", &User_prefix, 0},
    {"never", &User_prefix, 1},
    {"outbox", &User_prefix, 1},
    {"entropy", &User_prefix, 1},
    {""},
    {".usr", &User_prefix, 0},
    {""}, {""}, {""}, {""},
    {".tmp", &Tmp_prefix, 0},
    {"system.dh", &System_prefix, 1},
    {""}, {""}, {""},
    {".idx", &User_prefix, 0},
    {""}, {""},
    {"pk", &User_prefix, 1},
    {""},
    {".prv", &User_prefix, 0},
    {"sinfo", &User_prefix, 1},
    {"igrant", &User_prefix, 1},
    {""}, {""},
    {".ppm", &Tmp_prefix, 0},
    {"infos", &User_prefix, 1},
    {"ignore", &User_prefix, 1},
    {""}, {""},
    {".sql", &User_prefix, 0},
    {"inprogress", &User_prefix, 1},
    {"timing", &User_prefix, 1},
    {""}, {""},
    {".out", &Tmp_prefix, 0},
    {"stats", &User_prefix, 1},
    {""}, {""}, {""},
    {"auth", &User_prefix, 1},
    {"qtabs", &System_prefix, 1},
    {""}, {""}, {""},
    {".jpg", &Tmp_prefix, 0},
    {"inbox", &User_prefix, 1},
    {""}, {""},
    {"theygrnt", &User_prefix, 1},
    {".pat", &System_prefix, 0},
    {"noise", &User_prefix, 1},
    {""}, {""}, {""},
    {".pub", &User_prefix, 0},
    {".log", &Tmp_prefix, 0},
    {""}, {""},
    {"prf", &User_prefix, 1},
    {"prf2", &User_prefix, 1},
    {""}, {""}, {""}, {""},
    {".dbg", &System_prefix, 0},
    {""}, {""}, {""}, {""},
    {"poli", &System_prefix, 1},
    {"trash", &User_prefix, 1},
    {""}, {""}, {""},
    {".aux", &User_prefix, 0},
    {""}, {""},
    {"lhcore", &User_prefix, 1},
    {""},
    {".fle", &User_prefix, 0},
    {""}, {""}, {""}, {""},
    {".dif", &User_prefix, 0},
    {""}, {""},
    {"crashed", &User_prefix, 1},
    {""},
    {"xfer", &User_prefix, 1},
    {"crumb", &User_prefix, 1},
    {"xferdl", &System_prefix, 1},
    {"always", &User_prefix, 1},
    {""},
    {".dyc", &User_prefix, 0},
    {""}, {""}, {""},
    {"dwyco.dh", &System_prefix, 1},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""},
    {"pals", &User_prefix, 1},
    {""},
    {"accept", &User_prefix, 1}
};

struct fn_rules *
Perfect_Hash::in_word_set ( const char *str,  unsigned int len)
{
    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
         int key = hash (str, len);

        if (key <= MAX_HASH_VALUE && key >= 0)
            if (len == lengthtable[key])
            {
                 const char *s = wordlist[key].name;

                if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
                    return &wordlist[key];
            }
    }
    return 0;
}
