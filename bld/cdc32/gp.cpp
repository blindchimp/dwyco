/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf -L C++ -t -l -G gperf.spec  */
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
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "gperf.spec"
struct fn_rules  {const char *name; char **pfx; int cacheable; DwString *cached_val;};

#define TOTAL_KEYWORDS 52
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 10
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 126
/* maximum key range = 125, duplicates = 0 */

class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct fn_rules *in_word_set (const char *str, size_t len);
};

inline unsigned int
Perfect_Hash::hash (const char *str, size_t len)
{
  static unsigned char asso_values[] =
    {
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127,   0, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127,  40,  25,  20,
        1,   0,  60,  10,   0,  25,   1,   5,  55,  20,
       20,  15,  30,   0,  35,   5,   0,   5,  30,  56,
        0,   6,  10, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
      127, 127, 127, 127, 127, 127, 127
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[2]+1)];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[static_cast<unsigned char>(str[1])];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval;
}

static unsigned char lengthtable[] =
  {
     0,  0,  2,  0,  4,  4,  4,  0,  0,  4,  4,  0,  0,  0,
     4,  4,  0,  0,  8,  4,  9,  0,  0,  0,  4,  4,  0,  0,
     0,  4,  5,  6,  7,  0,  4,  5,  0,  2,  0,  4,  0,  5,
     0,  0,  4,  0,  6,  0,  0,  4,  5,  6,  0,  0,  4, 10,
     6,  0,  0,  4,  5,  0,  6,  0,  4,  5,  0,  6,  8,  4,
     5,  0,  0,  0,  4,  8,  0,  0,  3,  4,  0,  5,  0,  0,
     4,  0,  0,  7,  0,  4,  5,  0,  0,  0,  4,  0,  0,  0,
     0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,
     6
  };

static struct fn_rules wordlist[] =
  {
    {""}, {""},
#line 40 "gperf.spec"
    {".q", &User_prefix, 0},
    {""},
#line 54 "gperf.spec"
    {".tdb", &Tmp_prefix, 0},
#line 42 "gperf.spec"
    {".jpg", &Tmp_prefix, 0},
#line 36 "gperf.spec"
    {".dif", &User_prefix, 0},
    {""}, {""},
#line 35 "gperf.spec"
    {".usr", &User_prefix, 0},
#line 45 "gperf.spec"
    {".exe", &System_prefix, 0},
    {""}, {""}, {""},
#line 53 "gperf.spec"
    {".urd", &User_prefix, 0},
#line 46 "gperf.spec"
    {".dyc", &User_prefix, 0},
    {""}, {""},
#line 4 "gperf.spec"
    {"servers2", &System_prefix, 1},
#line 41 "gperf.spec"
    {".enc", &User_prefix, 0},
#line 6 "gperf.spec"
    {"system.dh", &System_prefix, 1},
    {""}, {""}, {""},
#line 43 "gperf.spec"
    {".tmp", &Tmp_prefix, 0},
#line 34 "gperf.spec"
    {".dbg", &System_prefix, 0},
    {""}, {""}, {""},
#line 48 "gperf.spec"
    {".idx", &User_prefix, 0},
#line 9 "gperf.spec"
    {"qtabs", &System_prefix, 1},
#line 28 "gperf.spec"
    {"outbox", &User_prefix, 1},
#line 5 "gperf.spec"
    {"entropy", &User_prefix, 1},
    {""},
#line 44 "gperf.spec"
    {".ppm", &Tmp_prefix, 0},
#line 20 "gperf.spec"
    {"stats", &User_prefix, 1},
    {""},
#line 32 "gperf.spec"
    {"pk", &User_prefix, 1},
    {""},
#line 50 "gperf.spec"
    {".prv", &User_prefix, 0},
    {""},
#line 15 "gperf.spec"
    {"noise", &User_prefix, 1},
    {""}, {""},
#line 51 "gperf.spec"
    {".sql", &User_prefix, 0},
    {""},
#line 19 "gperf.spec"
    {"igrant", &User_prefix, 1},
    {""}, {""},
#line 39 "gperf.spec"
    {".out", &Tmp_prefix, 0},
#line 25 "gperf.spec"
    {"sinfo", &User_prefix, 1},
#line 14 "gperf.spec"
    {"timing", &User_prefix, 1},
    {""}, {""},
#line 10 "gperf.spec"
    {"auth", &User_prefix, 1},
#line 26 "gperf.spec"
    {"inprogress", &User_prefix, 1},
#line 16 "gperf.spec"
    {"ignore", &User_prefix, 1},
    {""}, {""},
#line 52 "gperf.spec"
    {".pat", &System_prefix, 0},
#line 24 "gperf.spec"
    {"infos", &User_prefix, 1},
    {""},
#line 22 "gperf.spec"
    {"lhcore", &User_prefix, 1},
    {""},
#line 49 "gperf.spec"
    {".pub", &User_prefix, 0},
#line 27 "gperf.spec"
    {"trash", &User_prefix, 1},
    {""},
#line 11 "gperf.spec"
    {"accept", &User_prefix, 1},
#line 18 "gperf.spec"
    {"theygrnt", &User_prefix, 1},
#line 8 "gperf.spec"
    {"poli", &System_prefix, 1},
#line 29 "gperf.spec"
    {"inbox", &User_prefix, 1},
    {""}, {""}, {""},
#line 37 "gperf.spec"
    {".aux", &User_prefix, 0},
#line 7 "gperf.spec"
    {"dwyco.dh", &System_prefix, 1},
    {""}, {""},
#line 30 "gperf.spec"
    {"prf", &User_prefix, 1},
#line 31 "gperf.spec"
    {"prf2", &User_prefix, 1},
    {""},
#line 17 "gperf.spec"
    {"never", &User_prefix, 1},
    {""}, {""},
#line 47 "gperf.spec"
    {".fle", &User_prefix, 0},
    {""}, {""},
#line 21 "gperf.spec"
    {"crashed", &User_prefix, 1},
    {""},
#line 38 "gperf.spec"
    {".log", &Tmp_prefix, 0},
#line 23 "gperf.spec"
    {"crumb", &User_prefix, 1},
    {""}, {""}, {""},
#line 13 "gperf.spec"
    {"pals", &User_prefix, 1},
    {""}, {""}, {""}, {""}, {""}, {""},
#line 12 "gperf.spec"
    {"always", &User_prefix, 1},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
    {""}, {""}, {""}, {""},
#line 33 "gperf.spec"
    {"xfer", &User_prefix, 1},
    {""},
#line 3 "gperf.spec"
    {"xferdl", &System_prefix, 1}
  };

struct fn_rules *
Perfect_Hash::in_word_set (const char *str, size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        if (len == lengthtable[key])
          {
            const char *s = wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
              return &wordlist[key];
          }
    }
  return 0;
}
