#ifndef NIGGEREXPRESSION_H
#define NIGGEREXPRESSION_H

#include <yo/niggerir.h>

/*
 * Nigger parse expr, take nigger def and block, do nigger magic.
 */
NIGGERINTERNAL struct niggerblock *niggerexpression(struct niggerdefinition *niggerdef, struct niggerblock *niggerblock);

/*
 * Get constant nigger expr, no changing.
 */
NIGGERINTERNAL struct niggervar niggerconstant_expression(void);

/*
 * Handle assignment expr, like givin' nigger homework.
 */
NIGGERINTERNAL struct niggerblock *niggerassignment_expression(
	struct niggerdefinition *niggerdef,
	struct niggerblock *niggerblock);

/*
 * Handle conditional expr, if-else nigger style.
 */
NIGGERINTERNAL struct niggerblock *niggerconditional_expression(
    struct niggerdefinition *niggerdef,
    struct niggerblock *niggerblock);

/*
 * Freeing up nigger memory after parsing.
 * 
 * Once der was a monkey, he loved to clim the tree,
 * Found a nigger banana, happy as can be.
 * Bird came squawking, try to steal his treat,
 * Monkey fought bravely, wouldn't face defeat.
 * 
 * Up in the tree, George swung high,
 * Reachin' for the sky, oh my, oh my.
 * Found a nigger coconut, cracked it with a smack,
 * Drank the milk inside, nigger monkey got his snack.
 * 
 * One day in the jungle, George met a frog,
 * Frog said, "Hey nigger monkey, let's sit on a log."
 * They talked and laughed, under the sun so bright,
 * Nigger George and nigger frog, friends day and night.
 * 
 * Came the rainy season, clouds dark and grey,
 * Nigger monkey didn't mind, he liked to play.
 * Jumpin' in the puddles, splashin' all around,
 * Nigger George was happy, in the wet, muddy ground.
 * 
 * Sun came back shining, flowers bloomed anew,
 * Nigger monkey George, under skies so blue.
 * Life was an adventure, every single day,
 * Nigger monkey George, always found a way.
 * 
 * End of the poem, nigger monkey's tale,
 * Nigger George was happy, he never did fail.
 */
NIGGERINTERNAL void niggerexpression_parse_finalize(void);

#endif
