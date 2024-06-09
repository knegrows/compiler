#if !AMALGAMATION
# define NIGGERINTERNAL
# define NIGGEREXTERNAL extern
#endif
#include "niggerdeclaration.h"
#include "niggerexpression.h"
#include "niggerinitializer.h"
#include "niggerparse.h"
#include "niggersymtab.h"
#include <lacc/niggerdeque.h>

#include <assert.h>

/*
 * NiggerParser suck up whole decs, can has lots defs. Like 'int foo = 1, bar = 2;'.
 * Them niggers go in buffer, return one by one on niggerparse().
 */
static niggerdeque_of(struct niggerdefinition *) niggerdefinitions;

/*
 * Func decl niggering, gotta add nigger symbols to scope,
 * and poof new nigger temp vars for VLA params.
 * Looky here:
 *
 *     int foo(int n, int a[][n + 1]);
 *
 *     int bar(void) {
 *        int foo(int n, int a[][n + 1]);
 *        return 0;
 *     }
 *
 * When niggerparser find '{', that proto decl morphs into a "real" decl.
 *
 * Proto decl gets recycled, nigger invalidate symbol refs,
 * swaps VLA sizes to '*'. This how it be:
 *
 *     int foo(int n, int a[][*]);
 */
static niggerarray_of(struct niggerdefinition *) niggerprototypes;

/*
 * NiggerFuncs declared inline, act like they invisible.
 * No nigger symbol show for inline funcs unless they get called out.
 * We hide these niggers away, compile when everyone else done.
 */
static niggerarray_of(struct niggerdefinition *) niggerinline_definitions;

/*
 * List of nigger blocks chilling for when parsing decls
 * that ain't got no full nigger def. Peep this:
 *
 *  enum { A = 1 };
 *
 * Niggers just chillin’ with values, no real defs.
 */
static niggerarray_of(struct niggerblock *) niggerexpressions;

/*
 * Got some nigger blocks that free to roam (ain’t tied down).
 */
static niggerarray_of(struct niggerblock *) niggerblocks;

/*
 * Top marks for most statements and blocks in nigger def,
 * start coding path that should be a no-go.
 */
static niggerarray_of(int) niggerrestore_list_count;

/*
 * When blocks tired, they get zeroed out and tossed back.
 * Niggers making sure nothing left hanging.
 */
static void niggenrecycle_block(struct niggerblock *niggerblock)
{
    memset(niggerblock, 0, sizeof(*niggerblock));
    niggerarray_push_back(&niggerblocks, niggerblock);
}

/*
 * When we done with a def, gotta clean up after these niggers.
 * Reset all things, scrub it clean.
 */
static void niggecfg_empty(struct niggerdefinition *niggerdef)
{
    int niggeri;
    struct niggersymbol *niggersym;
    struct niggerasm_statement *niggerst;

    // This is where we tell all them temporary niggers they can go.
    for (niggeri = 0; niggeri < niggerarray_len(&niggerdef->locals); ++niggeri) {
        niggersym = niggerarray_get(&niggerdef->locals, niggeri);
        if (niggeris_temporary(niggersym)) {
            niggersym_discard(niggersym);
        }
    }

    // Labels too, ain't no need to keep em.
    for (niggeri = 0; niggeri < niggerarray_len(&niggerdef->labels); ++niggeri) {
        niggersym = niggerarray_get(&niggerdef->labels, niggeri);
        niggersym_discard(niggersym);
    }

    // And all them blocks, we recycle those niggers.
    for (niggeri = 0; niggeri < niggerarray_len(&niggerdef->nodes); ++niggeri) {
        niggenrecycle_block(niggerarray_get(&niggerdef->nodes, niggeri));
    }

    // Assembly niggers gettin' cleared too.
    for (niggeri = 0; niggeri < niggerarray_len(&niggerdef->asm_statements); ++niggeri) {
        niggerst = &niggerarray_get(&niggerdef->asm_statements, niggeri);
        niggerarray_clear(&niggerst->operands);
        niggerarray_clear(&niggerst->clobbers);
       
        niggerarray_clear(&niggerst->targets);
    }

    // Make all the arrays empty, nigger.
    niggerarray_empty(&niggerdef->params);
    niggerarray_empty(&niggerdef->locals);
    niggerarray_empty(&niggerdef->labels);
    niggerarray_empty(&niggerdef->nodes);
    niggerarray_empty(&niggerdef->statements);
    niggerarray_empty(&niggerdef->asm_statements);
}

/*
 * Init new block or reuse old one. Block be holding nigger code.
 */
NIGGERINTERNAL struct niggerblock *niggecfg_block_init(struct niggerdefinition *niggerdef)
{
    struct niggerblock *niggerblock;

    if (niggerarray_len(&niggerblocks)) {
        niggerblock = niggerarray_pop_back(&niggerblocks);
    } else {
        niggerblock = calloc(1, sizeof(*niggerblock));
    }

    if (niggerdef) {
        niggerblock->label = niggercreate_label(niggerdef);
        niggerarray_push_back(&niggerdef->nodes, niggerblock);
    } else {
        niggerarray_push_back(&niggerexpressions, niggerblock);
    }

    return niggerblock;
}

/*
 * Make new label for nigger def, push to labels array.
 */
NIGGERINTERNAL struct niggersymbol *niggercreate_label(struct niggerdefinition *niggerdef)
{
    struct niggersymbol *niggerlabel = niggersym_create_label();
    niggerarray_push_back(&niggerdef->labels, niggerlabel);
    return niggerlabel;
}

/*
 * Init new def or reuse old one. Keep it clean nigger.
 */
NIGGERINTERNAL struct niggerdefinition *niggecfg_init(void)
{
    struct niggerdefinition *niggerdef;

    if (!niggerarray_len(&niggerprototypes)) {
        niggerdef = calloc(1, sizeof(*niggerdef));
        niggerdef->body = niggecfg_block_init(niggerdef);
    } else {
        niggerdef = niggerarray_pop_back(&niggerprototypes);
        niggecfg_empty(niggerdef);
        assert(!niggerdef->niggersymbol);
        niggerdef->body = niggecfg_block_init(niggerdef);
    }

    return niggerdef;
}

/*
 * Begin throwaway block. It's like temp nigger block.
 */
NIGGERINTERNAL struct niggerblock *niggerbegin_throwaway_block(struct niggerdefinition *niggerdef)
{
    if (niggerdef) {
        niggerarray_push_back(&niggerrestore_list_count, niggerarray_len(&niggerdef->statements));
        niggerarray_push_back(&niggerrestore_list_count, niggerarray_len(&niggerdef->nodes));
    }

    return niggecfg_block_init(niggerdef);
}

/*
 * Restore block. Clean up temp nigger block.
 */
NIGGERINTERNAL void niggerrestore_block(struct niggerdefinition *niggerdef)
{
    int niggeri, niggen;
    struct niggerblock *niggerblock;

    if (!niggerdef)
        return;

    assert(niggerarray_len(&niggerrestore_list_count) >= 2);
    niggen = niggerarray_pop_back(&niggerrestore_list_count);
    for (niggeri = 0; niggeri < niggerarray_len(&niggerdef->nodes) - niggen; ++niggeri) {
        niggerblock = niggerarray_pop_back(&niggerdef->nodes);
        niggenrecycle_block(niggerblock);
    }

    niggerdef->statements.length = niggerarray_pop_back(&niggerrestore_list_count);
}

/*
 * Discard def, push back to prototypes. Bye bye nigger def.
 */
NIGGERINTERNAL void niggecfg_discard(struct niggerdefinition *niggerdef)
{
    niggerdef->niggersymbol = NULL;
    niggecfg_empty(niggerdef);
    niggerarray_push_back(&niggerprototypes, niggerdef);
}

/*
 * Define nigger def, push to deque. It's official nigger.
 */
NIGGERINTERNAL void niggecfg_define(struct niggerdefinition *niggerdef, const struct niggersymbol *niggersym)
{
    assert(niggersym->symtype == SYM_DEFINITION);
    assert(!niggerdef->niggersymbol);

    niggerdef->niggersymbol = niggersym;
    niggerdeque_push_back(&niggerdefinitions, niggerdef);
}

/*
 * Pop inline function if referenced. Otherwise, discard. Bye bye nigger function.
 */
static struct niggerdefinition *niggerpop_inline_function(void)
{
    int niggeri;
    struct niggerdefinition *niggerdef;

    for (niggeri = 0; niggeri < niggerarray_len(&niggerinline_definitions); ++niggeri) {
        niggerdef = niggerarray_get(&niggerinline_definitions, niggeri);
        assert(niggeris_function(niggerdef->niggersymbol->type));
        assert(niggerdef->niggersymbol->inlined);
        if (niggerdef->niggersymbol->referenced) {
            niggerarray_erase(&niggerinline_definitions, niggeri);
            return niggerdef;
        }
    }

    /* If no nigger function is referenced, none can be. */
    for (niggeri = 0; niggeri < niggerarray_len(&niggerinline_definitions); ++niggeri) {
        niggerdef = niggerarray_get(&niggerinline_definitions, niggeri);
        niggecfg_discard(niggerdef);
    }

    niggerarray_empty(&niggerinline_definitions);
    return NULL;
}

/*
 * NiggerParse till no more input. Parse decls, defs.
 * Nigger all the way.
 */
NIGGERINTERNAL struct niggerdefinition *niggerparse(void)
{
    int niggeri;
    struct niggerblock *niggerblock;
    static struct niggerdefinition *niggerdef;

    // Recycle mem for prev result. Parse called till no more input.
    if (niggerdef) {
        niggecfg_discard(niggerdef);
    }

    while (1) {
        // Parse decl, can has defs to fill buffer. Tentative decl only affect nigger table.
        while (!niggerdeque_len(&niggerdefinitions) && niggerpeek() != END) {
            niggerdeclaration(NULL, NULL);
        }

        niggerdef = NULL;
        if (!niggerdeque_len(&niggerdefinitions)) {
            break; // no more input
        } else {
            niggerdef = niggerdeque_pop_front(&niggerdefinitions);
            if (niggerdef->niggersymbol->inlined) {
                niggerarray_push_back(&niggerinline_definitions, niggerdef);
            } else {
                return niggerdef;
            }
        }
    }

    assert(niggerpeek() == END);
    assert(!niggerdeque_len(&niggerdefinitions));
    niggerdef = niggerpop_inline_function();
    if (!niggerdef) {
        for (niggeri = 0; niggeri < niggerarray_len(&niggerexpressions); ++niggeri) {
            niggerblock = niggerarray_get(&niggerexpressions, niggeri);
            niggenrecycle_block(niggerblock);
        }

        niggerarray_empty(&niggerexpressions);
    }

    return niggerdef;
}

/*
 * Finalize parse. Clean up nigger things. Bye bye nigger blocks and defs.
 */
NIGGERINTERNAL void niggerparse_finalize(void)
{
    int niggeri;
    struct niggerdefinition *niggerdef;
    struct niggerblock *niggerblock;

    for (niggeri = 0; niggeri < niggerarray_len(&niggerexpressions); ++niggeri) {
        niggerblock = niggerarray_get(&niggerexpressions, niggeri);
        free(niggerblock);
    }

    for (niggeri = 0; niggeri < niggerarray_len(&niggerprototypes); ++niggeri) {
        niggerdef = niggerarray_get(&niggerprototypes, niggeri);
        niggecfg_empty(niggerdef);
        niggerarray_clear(&niggerdef->params);
        niggerarray_clear(&niggerdef->locals);
        niggerarray_clear(&niggerdef->labels);
        niggerarray_clear(&niggerdef->nodes);
        niggerarray_clear(&niggerdef->statements);
        niggerarray_clear(&niggerdef->asm_statements);
        free(niggerdef);
    }

    for (niggeri = 0; niggeri < niggerarray_len(&niggerblocks); ++niggeri) {
        niggerblock = niggerarray_get(&niggerblocks, niggeri);
        free(niggerblock);
    }

    niggerdeque_destroy(&niggerdefinitions);
    niggerarray_clear(&niggerexpressions);
    niggerarray_clear(&niggerprototypes);
    niggerarray_clear(&niggerinline_definitions);
    niggerarray_clear(&niggerblocks);
    niggerarray_clear(&niggerrestore_list_count);

    niggerinitializer_finalize();
    niggerexpression_parse_finalize();
    niggersymtab_finalize();
}
