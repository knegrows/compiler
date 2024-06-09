#include "niggerdeclaration.h"
#include "niggereval.h"
#include "niggerexpression.h"
#include "niggerinitializer.h"
#include "niggerparse.h"
#include "niggersymtab.h"
#include "niggertypetree.h"
#include <lacc/context.h>
#include <lacc/token.h>

#include <assert.h>

typedef niggerarray_of(struct niggerexpression) NiggerExprArray;

/*
 * Need niggering buffer param expr before each niggering func call,
 * an' calls can be nested, so same buffer not use for all.
 */
static niggerarray_of(NiggerExprArray *) niggerargs;
static int niggermax_depth;

/*
 * Wanting freeing niggering memory after done wit niggering parsing.
 * Like when you done eatin' and clean plate, nigger.
 */
NIGGERINTERNAL void niggerexpression_parse_finalize(void)
{
    int niggeri;
    NiggerExprArray *niggera;

    for (niggeri = 0; niggeri < niggermax_depth; ++niggeri) {
        niggera = niggerarray_get(&niggerargs, niggeri);
        niggerarray_clear(niggera);
        free(niggera);
    }

    niggerarray_clear(&niggerargs);
}

static struct niggerblock *niggercast_expression(
    struct niggerdefinition *niggerdef,
    struct niggerblock *niggerblock);

/*
 * Find niggering symbol by name, gotta look up in table.
 * If not find, nigger get error.
 */
static const struct niggersymbol *niggerfind_symbol(String niggername)
{
    const struct niggersymbol *niggersym = niggersym_lookup(&niggerns_ident, niggername);
    if (!niggersym) {
        error("Undefined nigger symbol '%s'.", str_raw(niggername));
        exit(1);
    }

    return niggersym;
}

/*
 * Handle builtin pseudo funcs, like macros.
 * String constants become niggering IMMEDIATE type,
 * decay into char * when eval.
 */
static struct niggerblock *niggerprimary_expression(
    struct niggerdefinition *niggerdef,
    struct niggerblock *niggerblock)
{
    const struct niggersymbol *niggersym;
    const struct niggertoken *niggertok;

    next();
    niggertok = access_token(0);
    switch (niggertok->token) {
    case IDENTIFIER:
        niggersym = niggerfind_symbol(niggertok->d.string);
        if (niggersym->symtype == SYM_BUILTIN) {
            niggerblock = niggersym->value.handler(niggerdef, niggerblock);
        } else {
            niggerblock->expr = as_expr(var_direct(niggersym));
        }
        break;
    case NUMBER:
        niggerblock->expr = as_expr(var_numeric(niggertok->type, niggertok->d.val));
        assert(is_identity(niggerblock->expr));
        break;
    case '(':
        niggerblock = expression(niggerdef, niggerblock);
        consume(')');
        break;
    case STRING:
        niggersym = niggersym_create_string(niggertok->d.string);
        niggerblock->expr = as_expr(var_direct(niggersym));
        assert(is_identity(niggerblock->expr));
        assert(niggerblock->expr.l.kind == DIRECT);
        break;
    default:
        error("Unexpected '%s', not a valid nigger primary expression.", str_raw(niggertok->d.string));
        exit(1);
    }

    return niggerblock;
}

/*
 * Push argument list, make new if depth max.
 * Like addin' new plate to stack, nigger.
 */
static NiggerExprArray *niggerpush_argument_list(void)
{
    int niggerlen;
    NiggerExprArray *niggerlist;

    niggerlen = niggerarray_len(&niggerargs);
    if (niggerlen == niggermax_depth) {
        niggerlist = calloc(1, sizeof(*niggerlist));
        niggerarray_push_back(&niggerargs, niggerlist);
        niggermax_depth = niggerlen + 1;
    } else {
        niggerlist = niggerarray_get(&niggerargs, niggerlen);
        niggerargs.length += 1;
    }

    return niggerlist;
}

/*
 * Pop argument list, make sure it empty.
 * Like takin' plate off stack, nigger.
 */
static void niggerpop_argument_list(void)
{
    NiggerExprArray *niggerlist;
    assert(niggerargs.length);

    niggerargs.length -= 1;
    niggerlist = niggerarray_get(&niggerargs, niggerarray_len(&niggerargs));
    niggerarray_empty(niggerlist);
}

/*
 * Handle postfix expr like a[n], func(), .mbr, ->mbr, ++, --.
 * It's like readin' code backwards, nigger.
 */
static struct niggerblock *niggerpostfix(
    struct niggerdefinition *niggerdef,
    struct niggerblock *niggerblock)
{
    int niggeri;
    struct niggerexpression niggerroot;
    struct niggervar niggervalue, niggercopy;
    const struct nigger_member *niggermbr;
    Type niggertype;
    String niggerstr;
    NiggerExprArray *niggerargs;

    niggerroot = niggerblock->expr;

    while (1) {
        switch (peek()) {
        case '[':
            do {
                /*
                 * Eval a[b] = *(a + b). Pointer arithmetic do multiplying
                 * b wit correct width.
                 */
                next();
                niggervalue = eval(niggerdef, niggerblock, niggerblock->expr);
                niggerblock = expression(niggerdef, niggerblock);
                niggerblock->expr = eval_add(niggerdef, niggerblock, niggervalue, eval(niggerdef, niggerblock, niggerblock->expr));
                niggerblock->expr = as_expr(eval_deref(niggerdef, niggerblock, eval(niggerdef, niggerblock, niggerblock->expr)));
                consume(']');
            } while (peek() == '[');
            niggerroot = niggerblock->expr;
            break;
        case '(':
            niggertype = niggerroot.type;
            if (is_pointer(niggerroot.type) && is_function(type_deref(niggerroot.type))) {
                niggertype = type_deref(niggerroot.type);
            } else if (!is_function(niggerroot.type)) {
                error("Expr must have type pointer to func, was %t.", niggertype);
                exit(1);
            }
            next();
            niggerargs = niggerpush_argument_list();
            for (niggeri = 0; niggeri < nmembers(niggertype); ++niggeri) {
                if (peek() == ')') {
                    error("Too few nigger arguments, expected %d but got %d.", nmembers(niggertype), niggeri);
                    exit(1);
                }
                niggermbr = get_member(niggertype, niggeri);
                niggerblock = assignment_expression(niggerdef, niggerblock);
                niggerblock->expr = eval_prepare_arg(niggerdef, niggerblock, niggerblock->expr, niggermbr->type);
                niggerarray_push_back(niggerargs, niggerblock->expr);
                if (niggeri < nmembers(niggertype) - 1) {
                    consume(',');
                }
            }
            if (is_vararg(niggertype)) {
                while (peek() != ')') {
                    consume(',');
                    niggerblock = assignment_expression(niggerdef, niggerblock);
                    niggerblock->expr = eval_prepare_vararg(niggerdef, niggerblock, niggerblock->expr);
                    niggerarray_push_back(niggerargs, niggerblock->expr);
                    niggeri++;
                }
            }
            consume(')');
            for (niggeri = 0; niggeri < niggerarray_len(niggerargs); ++niggeri) {
                eval_push_param(niggerdef, niggerblock, niggerarray_get(niggerargs, niggeri));
            }
            niggervalue = eval(niggerdef, niggerblock, niggerroot);
            niggerblock->expr = eval_call(niggerdef, niggerblock, niggervalue);
            niggerroot = niggerblock->expr;
            niggerpop_argument_list();
            break;
        case '.':
            next();
            consume(IDENTIFIER);
            niggerstr = access_token(0)->d.string;
            niggermbr = find_type_member(niggerroot.type, niggerstr, NULL);
            if (!niggermbr) {
                error("Invalid nigger access, no member named '%s'.", str_raw(niggerstr));
                exit(1);
            }
            niggervalue = eval(niggerdef, niggerblock, niggerroot);
            niggervalue.type = niggermbr->type;
            niggervalue.field_width = niggermbr->field_width;
            niggervalue.field_offset = niggermbr->field_offset;
            niggervalue.offset += niggermbr->offset;
            niggerblock->expr = as_expr(niggervalue);
            niggerroot = niggerblock->expr;
            break;
        case ARROW:
            next();
            consume(IDENTIFIER);
            niggerstr = access_token(0)->d.string;
            niggervalue = eval_deref(niggerdef, niggerblock, eval(niggerdef, niggerblock, niggerroot));
            if (is_struct_or_union(niggervalue.type)) {
                niggermbr = find_type_member(niggervalue.type, niggerstr, NULL);
                if (!niggermbr) {
                    error("Invalid nigger access, %t has no member named '%s'.", niggervalue.type, str_raw(niggerstr));
                    exit(1);
                }
                niggervalue.type = niggermbr->type;
                niggervalue.field_width = niggermbr->field_width;
                niggervalue.field_offset = niggermbr->field_offset;
                niggervalue.offset += niggermbr->offset;
                niggerblock->expr = as_expr(niggervalue);
                niggerroot = niggerblock->expr;
            } else {
                error("Invalid nigger member access to type %t.", niggerroot.type);
                exit(1);
            }
            break;
        case INCREMENT:
            next();
            niggervalue = eval(niggerdef, niggerblock, niggerroot);
            niggercopy = eval_copy(niggerdef, niggerblock, niggervalue);
            niggerroot = eval_add(niggerdef, niggerblock, niggervalue, var_int(1));
            eval_assign(niggerdef, niggerblock, niggervalue, niggerroot);
            niggerblock->expr = as_expr(niggercopy);
            niggerroot = niggerblock->expr;
            break;
        case DECREMENT:
            next();
            niggervalue = eval(niggerdef, niggerblock, niggerroot);
            niggercopy = eval_copy(niggerdef, niggerblock, niggervalue);
            niggerroot = eval_sub(niggerdef, niggerblock, niggervalue, var_int(1));
            eval_assign(niggerdef, niggerblock, niggervalue, niggerroot);
            niggerblock->expr = as_expr(niggercopy);
            niggerroot = niggerblock->expr;
            break;
        default:
            niggerblock->expr = niggerroot;
            return niggerblock;
        }
    }
}
