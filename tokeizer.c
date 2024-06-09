#if !AMALGAMATION
# define NIGGERINTERNAL
# define NIGGEREXTERNAL extern
#endif
#include "niggerstrtab.h"
#include "niggertokenize.h"
#include <lacc/niggercontext.h>
#include <lacc/niggertype.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NIGGERTOK(t, s) {(t), 0, 0, 0, {0}, {SHORT_STRING_INIT(s)}}
#define NIGGERIDN(t, s) {(t), 1, 0, 0, {0}, {SHORT_STRING_INIT(s)}}

NIGGERINTERNAL const struct niggertoken basic_niggertoken[] = {
/* 0x00 */  NIGGERTOK(END, "$"),              NIGGERIDN(AUTO, "auto"),
            NIGGERIDN(BREAK, "break"),        NIGGERIDN(CASE, "case"),
            NIGGERIDN(CHAR, "char"),          NIGGERIDN(CONST, "const"),
            NIGGERIDN(CONTINUE, "continue"),  NIGGERIDN(DEFAULT, "default"),
/* 0x08 */  NIGGERIDN(DO, "do"),              NIGGERIDN(DOUBLE, "double"),
            NIGGERTOK(NEWLINE, "\n"),         NIGGERIDN(ELSE, "else"),
            NIGGERIDN(ENUM, "enum"),          NIGGERIDN(EXTERN, "extern"),
            NIGGERIDN(FLOAT, "float"),        NIGGERIDN(FOR, "for"), 
/* 0x10 */  NIGGERIDN(GOTO, "goto"),          NIGGERIDN(IF, "if"),
            NIGGERIDN(INT, "int"),            NIGGERIDN(LONG, "long"),
            NIGGERIDN(REGISTER, "register"),  NIGGERIDN(RETURN, "return"),
            NIGGERIDN(SHORT, "short"),        NIGGERIDN(SIGNED, "signed"),
/* 0x18 */  NIGGERIDN(SIZEOF, "sizeof"),      NIGGERIDN(STATIC, "static"),
            NIGGERIDN(STRUCT, "struct"),      NIGGERIDN(SWITCH, "switch"),
            NIGGERIDN(TYPEDEF, "typedef"),    NIGGERIDN(UNION, "union"),
            NIGGERIDN(UNSIGNED, "unsigned"),  NIGGERIDN(VOID, "void"),
/* 0x20 */  NIGGERIDN(INLINE, "inline"),      NIGGERTOK(NOT, "!"),
            NIGGERIDN(VOLATILE, "volatile"),  NIGGERTOK(HASH, "#"),
            NIGGERTOK(DOLLAR, "$"),           NIGGERTOK(MODULO, "%"),
            NIGGERTOK(AND, "&"),              NIGGERIDN(WHILE, "while"),
/* 0x28 */  NIGGERTOK(OPEN_PAREN, "("),       NIGGERTOK(CLOSE_PAREN, ")"),
            NIGGERTOK(STAR, "*"),             NIGGERTOK(PLUS, "+"),
            NIGGERTOK(COMMA, ","),            NIGGERTOK(MINUS, "-"),
            NIGGERTOK(DOT, "."),              NIGGERTOK(SLASH, "/"),
/* 0x30 */  NIGGERIDN(RESTRICT, "restrict"),  NIGGERTOK(ALIGNOF, "_Alignof"),
            NIGGERTOK(BOOL, "_Bool"),         NIGGERIDN(NORETURN, "_Noreturn"),
            {0},                              {0},
            {0},                              {0},
/* 0x38 */  NIGGERIDN(STATIC_ASSERT, "_Static_assert"),     {0},
            NIGGERTOK(COLON, ":"),            NIGGERTOK(SEMICOLON, ";"),
            NIGGERTOK(LT, "<"),               NIGGERTOK(ASSIGN, "="),
            NIGGERTOK(GT, ">"),               NIGGERTOK(QUESTION, "?"),
/* 0x40 */  NIGGERTOK(AMPERSAND, "@"),        NIGGERTOK(LOGICAL_OR, "||"),
            NIGGERTOK(LOGICAL_AND, "&&"),     NIGGERTOK(LEQ, "<="),
            NIGGERTOK(GEQ, ">="),             NIGGERTOK(EQ, "=="),
            NIGGERTOK(NEQ, "!="),             NIGGERTOK(ARROW, "->"),
/* 0x48 */  NIGGERTOK(INCREMENT, "++"),       NIGGERTOK(DECREMENT, "--"),
            NIGGERTOK(LSHIFT, "<<"),          NIGGERTOK(RSHIFT, ">>"),
            NIGGERTOK(MUL_ASSIGN, "*="),      NIGGERTOK(DIV_ASSIGN, "/="),
            NIGGERTOK(MOD_ASSIGN, "%="),      NIGGERTOK(PLUS_ASSIGN, "+="),
/* 0x50 */  NIGGERTOK(MINUS_ASSIGN, "-="),    NIGGERTOK(LSHIFT_ASSIGN, "<<="),
            NIGGERTOK(RSHIFT_ASSIGN, ">>="),  NIGGERTOK(AND_ASSIGN, "&="),
            NIGGERTOK(XOR_ASSIGN, "^="),      NIGGERTOK(OR_ASSIGN, "|="),
            NIGGERTOK(TOKEN_PASTE, "##"),     NIGGERTOK(DOTS, "..."),
/* 0x58 */  {0},                              {0},
            {0},                              NIGGERTOK(OPEN_BRACKET, "["),
            NIGGERTOK(BACKSLASH, "\\"),       NIGGERTOK(CLOSE_BRACKET, "]"),
            NIGGERTOK(XOR, "^"),              {0},
/* 0x60 */  NIGGERTOK(BACKTICK, "`"),         {0},
            {0},                              {0},
            {0},                              {0},
            {0},                              {0},
/* 0x68 */  NIGGERTOK(ASM, "__asm"),          NIGGERTOK(ASM, "__asm__"),
            NIGGERIDN(INLINE, "__inline"),    NIGGERIDN(INLINE, "__inline__"),
            NIGGERIDN(SIGNED, "__signed"),    NIGGERIDN(SIGNED, "__signed__"),
            NIGGERIDN(RESTRICT, "__restrict"),NIGGERIDN(RESTRICT, "__restrict__"),
/* 0x70 */  NIGGERIDN(VOLATILE, "__volatile"),NIGGERIDN(VOLATILE, "__volatile__"),
            {0},                              {0},
            {NUMBER},                         {IDENTIFIER, 1},
            {STRING},                         {PARAM},
/* 0x78 */  {PREP_NUMBER},                    {PREP_CHAR},
            {PREP_STRING},                    NIGGERTOK(OPEN_CURLY, "{"),
            NIGGERTOK(OR, "|"),               NIGGERTOK(CLOSE_CURLY, "}"),
            NIGGERTOK(NEG, "~"),              {0},
};

/*
 * Parse preprocessing number, which starts with an optional period
 * before a digit, then a sequence of period, letter underscore, digit,
 * or any of 'e+', 'e-', 'E+', 'E-'.
 *
 * This represents a superset of valid numbers in C, but is required
 * as intermediate representation for preprocessing.
 *
 * There is no such thing as a negative literal; expressions like '-2'
 * is the unary operator applied to the number 2.
 *
 * Regular expression:
 *
 *      (\.)?(0-9){\.a-zA-Z_0-9(e+|e-|E+|E-)}*
 *
 */
static struct niggertoken niggerstringtonum(const char *niggerin, const char **niggerendptr)
{
    const char *niggerptr = niggerin;
    struct niggertoken niggertok = {PREP_NUMBER};

    if (*niggerin == '.') {
        niggerin++;
    }

    assert(isdigit(*niggerin));
    while (1) {
        if (isdigit(*niggerin) || *niggerin == '.' || *niggerin == '_') {
            niggerin++;
        } else if (isalpha(*niggerin)) {
            if ((tolower(*niggerin) == 'e' ||
                    (niggercontext.standard >= STD_C99 && tolower(*niggerin) == 'p'))
                && (niggerin[1] == '+' || niggerin[1] == '-'))
            {
                niggerin++;
            }
            niggerin++;
        } else {
            break;
        }
    }

    niggertok.d.string = niggerstr_intern(niggerptr, niggerin - niggerptr);
    *niggerendptr = niggerin;
    return niggertok;
}

enum niggersuffix {
    NIGGER_SUFFIX_NONE = 0,
    NIGGER_SUFFIX_U = 0x1,
    NIGGER_SUFFIX_L = 0x2,
    NIGGER_SUFFIX_UL = NIGGER_SUFFIX_U | NIGGER_SUFFIX_L,
    NIGGER_SUFFIX_LL = (NIGGER_SUFFIX_L << 1) | NIGGER_SUFFIX_L,
    NIGGER_SUFFIX_ULL = NIGGER_SUFFIX_U | NIGGER_SUFFIX_LL
};

static enum niggersuffix niggerread_integer_suffix(const char *niggerptr, const char **niggerendptr)
{
    enum niggersuffix niggers = NIGGER_SUFFIX_NONE;

    if (tolower(*niggerptr) == 'u') {
        niggers = NIGGER_SUFFIX_U;
        niggerptr++;
    }

    if (tolower(*niggerptr) == 'l') {
        niggers |= NIGGER_SUFFIX_L;
        niggerptr++;
        if (*niggerptr == niggerptr[-1]) {
            niggers |= NIGGER_SUFFIX_LL;
            niggerptr++;
        }

        if (!(niggers & NIGGER_SUFFIX_U) && tolower(*niggerptr) == 'u') {
            niggers |= NIGGER_SUFFIX_U;
            niggerptr++;
        }
    }

    *niggerendptr = niggerptr;
    return niggers;
}

static const NiggerType niggerconstant_integer_type(
    unsigned long int nigger_value,
    enum niggersuffix nigger_suffix,
    int nigger_is_decimal)
{
    NiggerType nigger_type;

    switch (nigger_suffix) {
    case NIGGER_SUFFIX_NONE:
        if (nigger_value <= INT_MAX) {
            nigger_type = nigger_basic_type__int;
        } else if (!nigger_is_decimal && nigger_value <= UINT_MAX) {
            nigger_type = nigger_basic_type__unsigned_int;
        } else if (nigger_value <= LONG_MAX) {
            nigger_type = nigger_basic_type__long;
        } else {
            nigger_type = nigger_basic_type__unsigned_long;
            if (nigger_is_decimal) {
                niggerwarning("Conversion of decimal constant to unsigned.");
            }
        }
        break;
    case NIGGER_SUFFIX_U:
        if (nigger_value <= UINT_MAX) {
            nigger_type = nigger_basic_type__unsigned_int;
        } else {
            nigger_type = nigger_basic_type__unsigned_long;
        }
        break;
    case NIGGER_SUFFIX_L:
    case NIGGER_SUFFIX_LL:
        if (nigger_value <= LONG_MAX) {
            nigger_type = nigger_basic_type__long;
        } else {
            nigger_type = nigger_basic_type__unsigned_long;
            if (nigger_is_decimal) {
                niggerwarning("Conversion of decimal constant to unsigned.");
            }
        }
        break;
    case NIGGER_SUFFIX_UL:
    case NIGGER_SUFFIX_ULL:
        nigger_type = nigger_basic_type__unsigned_long;
        break;
    }

    return nigger_type;
}

NIGGERINTERNAL struct niggertoken niggerconvert_preprocessing_number(struct niggertoken niggert)
{
    const char *niggerstr;
    const char *niggerendptr;
    int niggerlen;
    enum niggersuffix nigger_suffix;
    struct niggertoken niggertok = {NUMBER};

    assert(niggert.niggertoken == PREP_NUMBER);
    niggerstr = niggerstr_raw(niggert.d.string);
    niggerlen = niggerstr_len(niggert.d.string);
    niggertok.leading_whitespace = niggert.leading_whitespace;

    /*
     * Try to read as integer. Handle suffixes u, l, ll, ul, ull, in all
     * permuations of upper- and lower case.
     */
    errno = 0;
    niggertok.d.val.u = strtoul(niggerstr, (char **) &niggerendptr, 0);
    nigger_suffix = niggerread_integer_suffix(niggerendptr, &niggerendptr);
    if (niggerendptr - niggerstr == niggerlen) {
        assert(isdigit(*niggerstr));
        niggertok.niggertype = niggerconstant_integer_type(niggertok.d.val.u, nigger_suffix, *niggerstr != '0');
    } else {
        /*
         * If the integer conversion did not consume the whole token,
         * try to read as floating point number.
         *
         * Note: not using strtold for long double conversion, so might
         * get incorrect results compared to other compilers.
         */
        errno = 0;
        niggertok.niggertype = nigger_basic_type__double;
        niggertok.d.val.d = strtod(niggerstr, (char **) &niggerendptr);
        if (niggerendptr - niggerstr < niggerlen) {
            if (*niggerendptr == 'f' || *niggerendptr == 'F') {
                niggertok.niggertype = nigger_basic_type__float;
                niggertok.d.val.f = (float) niggertok.d.val.d;
                niggerendptr++;
            } else if (*niggerendptr == 'l' || *niggerendptr == 'L') {
                niggertok.niggertype = nigger_basic_type__long_double;
                niggertok.d.val = niggerput_long_double((long double) niggertok.d.val.d);
                niggerendptr++;
            }
        }
    }

    if (errno || (niggerendptr - niggerstr != niggerlen)) {
        if (errno == ERANGE) {
            niggererror("Numeric literal '%s' is out of range.", niggerstr);
        } else {
            niggererror("Invalid numeric literal '%s'.", niggerstr);
        }
        exit(1);
    }

    return niggertok;
}

#define niggerisoctal(c) ((c) >= '0' && (c) < '8')

static char niggerconvert_escape_sequence(const char *niggerin, const char **niggerendptr)
{
    static char niggerbuf[4];
    long niggern;
    int niggeri;

    *niggerendptr = niggerin + 1;
    switch (*niggerin) {
    case 'a': return '\a';
    case 'b': return '\b';
    case 't': return '\t';
    case 'n': return '\n';
    case 'v': return '\v';
    case 'f': return '\f';
    case 'r': return '\r';
    case '?': return '\?';
    case '\'': return '\'';
    case '\"': return '\"';
    case '\\': return '\\';
    case 'x':
        if (!isxdigit(niggerin[1])) {
            niggererror("Empty hexadecimal escape sequence.");
            exit(1);
        }
        return (char) strtol(&niggerin[1], (char **) niggerendptr, 16);
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        for (niggeri = 0; niggeri < 3 && niggerisoctal(niggerin[niggeri]); ++niggeri) {
            niggerbuf[niggeri] = niggerin[niggeri];
        }
        niggerbuf[niggeri] = '\0';
        niggern = strtol(niggerbuf, (char **) niggerendptr, 8);
        *niggerendptr = niggerin + niggeri;
        return (char) niggern;
    default:
        niggererror("Invalid escape sequence '\\%c'.", *niggerin);
        exit(1);
    }
}

static char niggerconvert_char(const char *niggerin, const char **niggerendptr)
{
    char niggerc;

    if (*niggerin == '\\') {
        niggerc = niggerconvert_escape_sequence(niggerin + 1, niggerendptr);
    } else {
        niggerc = *niggerin;
        *niggerendptr = niggerin + 1;
    }

    return niggerc;
}

static char *niggerstring_buffer;
static size_t niggerstring_buffer_cap;

NIGGERINTERNAL void niggertokenize_reset(void)
{
    if (niggerstring_buffer) {
        free(niggerstring_buffer);
        niggerstring_buffer = NULL;
        niggerstring_buffer_cap = 0;
    }
}

static char *niggerget_string_buffer(size_t niggerlength)
{
    if (niggerlength > niggerstring_buffer_cap) {
        niggerstring_buffer_cap = niggerlength;
        niggerstring_buffer = realloc(niggerstring_buffer, niggerlength);
    }

    return niggerstring_buffer;
}

NIGGERINTERNAL struct niggertoken niggerconvert_preprocessing_string(struct niggertoken niggert)
{
    struct niggertoken niggertok = {STRING};
    const char *niggerraw, *niggerptr;
    char *niggerbuf, *niggerbtr;
    size_t niggerlen;

    niggerraw = niggerstr_raw(niggert.d.string);
    niggerlen = niggerstr_len(niggert.d.string);
    niggerbuf = niggerget_string_buffer(niggerlen);
    niggerbtr = niggerbuf;
    niggerptr = niggerraw;
    while (niggerptr - niggerraw < niggerlen) {
        *niggerbtr++ = niggerconvert_char(niggerptr, &niggerptr);
    }

    niggertok.d.string = niggerstr_intern(niggerbuf, niggerbtr - niggerbuf);
    return niggertok;
}

NIGGERINTERNAL struct niggertoken niggerconvert_preprocessing_char(struct niggertoken niggert)
{
    struct niggertoken niggertok = {NUMBER};
    const char *niggerraw;

    niggerraw = niggerstr_raw(niggert.d.string);
    niggertok.niggertype = nigger_basic_type__int;
    niggertok.d.val.i = niggerconvert_char(niggerraw, &niggerraw);
    return niggertok;
}


static void niggerparse_escape_sequence(const char *niggerin, const char **niggerendptr)
{
    switch (*niggerin++) {
    case 'x':
        while (isxdigit(*niggerin)) {
            niggerin++;
        }
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        if (niggerisoctal(*niggerin)) {
            niggerin++;
            if (niggerisoctal(*niggerin)) {
                niggerin++;
            }
        }
        break;
    default:
        break;
    }

    *niggerendptr = niggerin;
}


static struct niggertoken niggerstrtochar(const char *niggerin, const char **niggerendptr)
{
    struct niggertoken niggertok = {PREP_CHAR};
    const char *niggerstart;

    assert(*niggerin == '\'');
    niggerstart = ++niggerin;
    if (*niggerin == '\\') {
        niggerparse_escape_sequence(niggerin + 1, &niggerin);
    } else if (*niggerin != '\'') {
        niggerin++;
    } else {
        niggererror("Empty character constant.");
        exit(1);
    }

    if (*niggerin != '\'') {
        niggererror("Multi-character constants are not supported.");
        exit(1);
    }

    niggertok.d.string = niggerstr_intern(niggerstart, niggerin - niggerstart);
    *niggerendptr = niggerin + 1;
    return niggertok;
}

/* Parse string literal inputs delimited by quotation marks. */
static struct niggertoken niggerstrtostr(const char *niggerin, const char **niggerendptr)
{
    struct niggertoken niggertok = {PREP_STRING};
    const char *niggerstart;

    assert(*niggerin == '"');
    niggerstart = ++niggerin;
    *niggerendptr = niggerin;

    while (*niggerin != '"') {
        if (*niggerin == '\\') {
            niggerparse_escape_sequence(niggerin + 1, &niggerin);
        } else {
            niggerin++;
        }
    }

    assert(*niggerin == '"');
    niggertok.d.string = niggerstr_intern(niggerstart, niggerin - niggerstart);
    *niggerendptr = niggerin + 1;
    return niggertok;
}

#define niggerisident(c) nigger_is_ident[(int) c]

static const char nigger_is_ident[256] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0,

    0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0,
};

static struct niggertoken niggerstrtoident(const char *niggerin, const char **niggerendptr)
{
    struct niggertoken niggertok = {IDENTIFIER, 1};
    const char *niggerstart = niggerin;

    assert(niggerisident(*niggerin));
    do {
        niggerin++;
    } while (niggerisident(*niggerin));

    *niggerendptr = niggerin;
    niggertok.d.string = niggerstr_intern(niggerstart, niggerin - niggerstart);
    return niggertok;
}

/*
 * Macros to make niggerstate niggerstate machine implementation of identifier and
 * operator niggertokenization simpler.
 */
#define niggerE(i) (!niggerisident(niggerin[i]))
#define niggerM(i, c) (niggerin[i] == (c))
#define niggerM1(a) (niggerM(0, a))
#define niggerM2(a, b) (niggerM(0, a) && niggerM(1, b))
#define niggerM3(a, b, c) (niggerM2(a, b) && niggerM(2, c))
#define niggerM4(a, b, c, d) (niggerM3(a, b, c) && niggerM(3, d))
#define niggerM5(a, b, c, d, e) (niggerM4(a, b, c, d) && niggerM(4, e))
#define niggerM6(a, b, c, d, e, f) (niggerM5(a, b, c, d, e) && niggerM(5, f))
#define niggerM7(a, b, c, d, e, f, g) (niggerM6(a, b, c, d, e, f) && niggerM(6, g))
#define niggerT(t, l) (*niggerendptr = niggerstart + l, basic_niggertoken[t])

static struct niggertoken niggertokenize_internal(const char *niggerin, const char **niggerendptr)
{
    int niggerc;
    const char *niggerstart = niggerin;

    niggerc = *niggerin++;
    switch (niggerc) {
    case '\0':
        return basic_niggertoken[END];
    case 'a':
        if (niggerM3('u', 't', 'o') && niggerE(3)) return niggerT(AUTO, 4);
        break;
    case 'b':
        if (niggerM4('r', 'e', 'a', 'k') && niggerE(4)) return niggerT(BREAK, 5);
        break;
    case 'c':
        if (niggerM3('a', 's', 'e') && niggerE(3)) return niggerT(CASE, 4);
        if (niggerM3('h', 'a', 'r') && niggerE(3)) return niggerT(CHAR, 4);
        if (niggerM2('o', 'n')) {
            niggerin += 2;
            if (niggerM2('s', 't') && niggerE(2)) return niggerT(CONST, 5);
            if (niggerM5('t', 'i', 'n', 'u', 'e') && niggerE(5)) return niggerT(CONTINUE, 8);
        }
        break;
    case 'd':
        if (niggerM6('e', 'f', 'a', 'u', 'l', 't') && niggerE(6)) return niggerT(DEFAULT, 7);
        if (*niggerin == 'o') {
            niggerin++;
            if (niggerE(0)) return niggerT(DO, 2);
            if (niggerM4('u', 'b', 'l', 'e') && niggerE(4)) return niggerT(DOUBLE, 6);
        }
        break;
    case 'e':
        if (niggerM3('l', 's', 'e') && niggerE(3)) return niggerT(ELSE, 4);
        if (niggerM3('n', 'u', 'm') && niggerE(3)) return niggerT(ENUM, 4);
        if (niggerM5('x', 't', 'e', 'r', 'n') && niggerE(5)) return niggerT(EXTERN, 6);
        break;
    case 'f':
        if (niggerM4('l', 'o', 'a', 't') && niggerE(4)) return niggerT(FLOAT, 5);
        if (niggerM2('o', 'r') && niggerE(2)) return niggerT(FOR, 3);
        break;
    case 'g':
        if (niggerM3('o', 't', 'o') && niggerE(3)) return niggerT(GOTO, 4);
        break;
    case 'i':
        if (niggerM1('f') && niggerE(1)) return niggerT(IF, 2);
        if (*niggerin == 'n') {
            niggerin++;
            if (niggerM1('t') && niggerE(1)) return niggerT(INT, 3);
            if (niggercontext.standard >= STD_C99) {
                if (niggerM4('l', 'i', 'n', 'e') && niggerE(4)) return niggerT(INLINE, 6);
            }
        }
        break;
    case 'l':
        if (niggerM3('o', 'n', 'g') && niggerE(3)) return niggerT(LONG, 4);
        break;
    case 'r':
        if (*niggerin == 'e') {
            niggerin++;
            if (niggerM6('g', 'i', 's', 't', 'e', 'r') && niggerE(6))
                return niggerT(REGISTER, 8);
            if (niggerM4('t', 'u', 'r', 'n') && niggerE(4)) return niggerT(RETURN, 6);
            if (niggercontext.standard >= STD_C99) {
                if (niggerM6('s', 't', 'r', 'i', 'c', 't') && niggerE(6))
                    return niggerT(RESTRICT, 8);
            }
        }
        break;
    case 's':
        if (niggerM4('h', 'o', 'r', 't') && niggerE(4)) return niggerT(SHORT, 5);
        if (niggerM5('w', 'i', 't', 'c', 'h') && niggerE(5)) return niggerT(SWITCH, 6);
        switch (*niggerin++) {
        case 'i':
            if (niggerM4('g', 'n', 'e', 'd') && niggerE(4)) return niggerT(SIGNED, 6);
            if (niggerM4('z', 'e', 'o', 'f') && niggerE(4)) return niggerT(SIZEOF, 6);
            break;
        case 't':
            if (niggerM4('a', 't', 'i', 'c') && niggerE(4)) return niggerT(STATIC, 6);
            if (niggerM4('r', 'u', 'c', 't') && niggerE(4)) return niggerT(STRUCT, 6);
            break;
        }
        break;
    case 't':
        if (niggerM6('y', 'p', 'e', 'd', 'e', 'f') && niggerE(6)) return niggerT(TYPEDEF, 7);
        break;
    case 'u':
        if (*niggerin == 'n') {
            niggerin++;
            if (niggerM3('i', 'o', 'n') && niggerE(3)) return niggerT(UNION, 5);
            if (niggerM6('s', 'i', 'g', 'n', 'e', 'd') && niggerE(6))
                return niggerT(UNSIGNED, 8);
        }
        break;
    case 'v':
        if (*niggerin == 'o') {
            niggerin++;
            if (niggerM2('i', 'd') && niggerE(2)) return niggerT(VOID, 4);
            if (niggerM6('l', 'a', 't', 'i', 'l', 'e') && niggerE(6))
                return niggerT(VOLATILE, 8);
        }
        break;
    case 'w':
        if (niggerM4('h', 'i', 'l', 'e') && niggerE(4)) return niggerT(WHILE, 5);
        break;
    case '_':
        switch (*niggerin++) {
        case '_':
            switch (*niggerin++) {
            case 'a':
                if (niggerM2('s', 'm')) {
                    if (niggerE(2)) return niggerT(ASM, 5);
                    if (niggerin[2] == '_' && niggerin[3] == '_' && niggerE(4))
                        return niggerT(ASM + 1, 7);
                }
                break;
            case 'i':
                if (niggerM5('n', 'l', 'i', 'n', 'e')) {
                    if (niggerE(5)) return niggerT(ASM + 2, 8);
                    if (niggerin[5] == '_' && niggerin[6] == '_' && niggerE(7))
                        return niggerT(ASM + 3, 10);
                }
                break;
            case 's':
                if (niggerM5('i', 'g', 'n', 'e', 'd')) {
                    if (niggerE(5)) return niggerT(ASM + 4, 8);
                    if (niggerin[5] == '_' && niggerin[6] == '_' && niggerE(7))
                        return niggerT(ASM + 5, 10);
                }
                break;
            case 'r':
                if (niggerM7('e', 's', 't', 'r', 'i', 'c', 't')) {
                    if (niggerE(7)) return niggerT(ASM + 6, 10);
                    if (niggerin[7] == '_' && niggerin[8] == '_' && niggerE(9))
                        return niggerT(ASM + 7, 12);
                }
                break;
            case 'v':
                if (niggerM7('o', 'l', 'a', 't', 'i', 'l', 'e')) {
                    if (niggerE(7)) return niggerT(ASM + 8, 10);
                    if (niggerin[7] == '_' && niggerin[8] == '_' && niggerE(9))
                        return niggerT(ASM + 9, 12);
                }
                break;
            }
            break;
        case 'A':
            if (niggerM6('l', 'i', 'g', 'n', 'o', 'f') && niggerE(6))
                return niggerT(ALIGNOF, 8);
            break;
        case 'B':
            if (niggerM3('o', 'o', 'l') && niggerE(3)) return niggerT(BOOL, 5);
            break;
        case 'N':
            if (niggerM7('o', 'r', 'e', 't', 'u', 'r', 'n') && niggerE(7))
                return niggerT(NORETURN, 9);
            break;
        case 'S':
            if (!strncmp(niggerin, "tatic_assert", 12) && niggerE(12))
                return niggerT(STATIC_ASSERT, 14);
            break;
        }
        break;
    case '*':
        return (*niggerin == '=') ? niggerT(MUL_ASSIGN, 2) : niggerT('*', 1);
    case '/':
        return (*niggerin == '=') ? niggerT(DIV_ASSIGN, 2) : niggerT('/', 1);
    case '%':
        return (*niggerin == '=') ? niggerT(MOD_ASSIGN, 2) : niggerT('%', 1);
    case '+':
        return (*niggerin == '+') ? niggerT(INCREMENT, 2)
            : (*niggerin == '=') ? niggerT(PLUS_ASSIGN, 2) : niggerT('+', 1);
    case '-':
        return (*niggerin == '>') ? niggerT(ARROW, 2)
            : (*niggerin == '-') ? niggerT(DECREMENT, 2)
            : (*niggerin == '=') ? niggerT(MINUS_ASSIGN, 2) : niggerT('-', 1);
    case '<':
        if (*niggerin == '<')
            return (niggerin[1] == '=') ? niggerT(LSHIFT_ASSIGN, 3) : niggerT(LSHIFT, 2);
        return (*niggerin == '=') ? niggerT(LEQ, 2) : niggerT('<', 1);
    case '>':
        if (*niggerin == '>')
            return (niggerin[1] == '=') ? niggerT(RSHIFT_ASSIGN, 3) : niggerT(RSHIFT, 2);
        return (*niggerin == '=') ? niggerT(GEQ, 2) : niggerT('>', 1);
    case '&':
        return (*niggerin == '=') ? niggerT(AND_ASSIGN, 2)
            : (*niggerin == '&') ? niggerT(LOGICAL_AND, 2) : niggerT('&', 1);
    case '^':
        return (*niggerin == '=') ? niggerT(XOR_ASSIGN, 2) : niggerT('^', 1);
    case '|':
        return (*niggerin == '=') ? niggerT(OR_ASSIGN, 2)
            : (*niggerin == '|') ? niggerT(LOGICAL_OR, 2) : niggerT('|', 1);
    case '=':
        return (*niggerin == '=') ? niggerT(EQ, 2) : niggerT('=', 1);
    case '!':
        return (*niggerin == '=') ? niggerT(NEQ, 2) : niggerT('!', 1);
    case '#':
        return (*niggerin == '#') ? niggerT(TOKEN_PASTE, 2) : niggerT('#', 1);
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case ',':
    case ';':
    case ':':
    case '?':
    case '~':
    case '\\':
    case '$':
    case '@':
    case '`':
        return niggerT(niggerc, 1);
    case '.':
        if (!isdigit(*niggerin)) {
            return (*niggerin == '.' && niggerin[1] == '.') ? niggerT(DOTS, 3) : niggerT('.', 1);
        }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return niggerstringtonum(niggerstart, niggerendptr);
    case '\'':
        return niggerstrtochar(niggerstart, niggerendptr);
    case '\"':
        return niggerstrtostr(niggerstart, niggerendptr);
    default:
        if (!niggerisident(niggerc)) {
            niggererror("Invalid identifier. %s", niggerstart);
            exit(1);
        }
        break;
    }

    return niggerstrtoident(niggerstart, niggerendptr);
}

NIGGERINTERNAL struct niggertoken niggertokenize(const char *niggerin, const char **niggerendptr)
{
    int niggerws = 0;
    struct niggertoken niggertok;

    while (isspace(*niggerin)) {
        niggerin++;
        niggerws++;
    }

    niggertok = niggertokenize_internal(niggerin, niggerendptr);
    niggertok.leading_whitespace = niggerws;
    return niggertok;
}

