#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "strops.h"
#include "dt-locale.h"
#include "dexpr.h"
#include "dexpr-parser.h"

#if !defined STANDALONE
# if defined __INTEL_COMPILER
#  pragma warning (disable:2259)
# endif	/* __INTEL_COMPILER */
# include "dexpr-parser.c"
# if defined __INTEL_COMPILER
#  pragma warning (default:2259)
# endif	/* __INTEL_COMPILER */

# if defined __INTEL_COMPILER
#  pragma warning (disable:2557)
# endif	/* __INTEL_COMPILER */
# include "dexpr-scanner.c"
# if defined __INTEL_COMPILER
#  pragma warning (default:2557)
# endif	/* __INTEL_COMPILER */
#endif	/* !STANDALONE */

static void
free_dexpr(dexpr_t root)
{
/* recursive free :( */
	switch (root->type) {
	case DEX_CONJ:
	case DEX_DISJ:
		if (root->left != NULL) {
			free_dexpr(root->left);
			free(root->left);
		}
		if (root->right != NULL) {
			free_dexpr(root->right);
			free(root->right);
		}
		break;
	case DEX_UNK:
	case DEX_VAL:
	default:
		break;
	}
	return;
}

static __attribute__((unused)) void
__pr_val(struct dexkv_s *kv)
{
	switch (kv->sp.spfl) {
	case DT_SPFL_N_DCNT_MON:
		fputs("%d ", stdout);
		break;
	case DT_SPFL_N_MON:
	case DT_SPFL_S_MON:
		fputs("%b ", stdout);
		break;
	case DT_SPFL_N_YEAR:
		fputs("%Y ", stdout);
		break;
	case DT_SPFL_N_DCNT_WEEK:
	case DT_SPFL_S_WDAY:
		fputs("%a ", stdout);
		break;
	case DT_SPFL_N_WCNT_MON:
		fputs("%c ", stdout);
		break;
	case DT_SPFL_N_DCNT_YEAR:
		fputs("%D ", stdout);
		break;
	case DT_SPFL_N_WCNT_YEAR:
		fputs("%C ", stdout);
		break;
	default:
		break;
	}

	switch (kv->op) {
	case OP_LT:
		fputs("< ", stdout);
		break;
	case OP_LE:
		fputs("<= ", stdout);
		break;
	case OP_GT:
		fputs("> ", stdout);
		break;
	case OP_GE:
		fputs(">= ", stdout);
		break;
	case OP_NE:
		fputs("!= ", stdout);
		break;
	case OP_EQ:
	default:
		fputs("== ", stdout);
		break;
	}

	switch (kv->sp.spfl) {
	case DT_SPFL_N_STD: {
		char buf[32];
		dt_strfdt(buf, sizeof(buf), NULL, kv->d);
		fputs(buf, stdout);
		break;
	}
	case DT_SPFL_N_DCNT_MON:
		fprintf(stdout, "%02d", kv->s);
		break;
	case DT_SPFL_N_MON:
	case DT_SPFL_S_MON:
		if (kv->s >= 0 && kv->s <= 12) {
			fputs(dut_abbr_mon[kv->s], stdout);
		}
		break;
	case DT_SPFL_N_YEAR:
		fprintf(stdout, "%04d", kv->s);
		break;
	case DT_SPFL_N_DCNT_WEEK:
	case DT_SPFL_S_WDAY:
		if (kv->s >= 0 && kv->s <= 7) {
			fputs(dut_abbr_wday[kv->s], stdout);
		}
		break;
	case DT_SPFL_N_WCNT_MON:
	case DT_SPFL_N_WCNT_YEAR:
		fprintf(stdout, "%02d", kv->s);
		break;
	case DT_SPFL_N_DCNT_YEAR:
		fprintf(stdout, "%03d", kv->s);
		break;
	default:
		break;
	}
	return;
}

static __attribute__((unused)) void
__pr(dexpr_t root, size_t ind)
{
	switch (root->type) {
	case DEX_VAL:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		if (root->nega) {
			fputs("!(", stdout);
		}
		__pr_val(root->kv);
		if (root->nega) {
			fputs(")\n", stdout);
		} else {
			fputc('\n', stdout);
		}
		break;

	case DEX_CONJ:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		if (!root->nega) {
			fputs("AND\n", stdout);
		} else {
			fputs("NAND\n", stdout);
		}
		__pr(root->left, ind + 2);
		__pr(root->right, ind + 2);
		break;

	case DEX_DISJ:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		if (!root->nega) {
			fputs("OR\n", stdout);
		} else {
			fputs("NOR\n", stdout);
		}
		__pr(root->left, ind + 2);
		__pr(root->right, ind + 2);
		break;

	case DEX_UNK:
	default:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		if (root->left != NULL) {
			fputs("ROOT\n", stderr);
			__pr(root->left, ind + 2);
			break;
		}
		break;
	}
	return;
}

static __attribute__((unused)) void
__pr_infix(dexpr_t root)
{
	if (root->type == DEX_VAL) {
		__pr_val(root->kv);
		return;
	}

	__pr_infix(root->left);

	switch (root->type) {
	case DEX_CONJ:
		fputs(" && ", stdout);
		break;

	case DEX_DISJ:
		fputs(" || ", stdout);
		break;

	case DEX_VAL:
	case DEX_UNK:
	default:
		/* shouldn't happen :O */
		fputs(" bollocks ", stdout);
		break;
		
	}
	__pr_infix(root->right);

	/* just ascend */
	return;
}


static dexpr_t
make_dexpr(dex_type_t type)
{
	dexpr_t res;

	if ((res = calloc(1, sizeof(*res))) != NULL) {
		res->type = type;
	}
	return res;
}

static dexpr_t
dexpr_copy(const_dexpr_t src)
{
	dexpr_t res;

	if ((res = calloc(1, sizeof(*res))) == NULL) {
		return NULL;
	}
	/* otherwise start by (shallow) copying things */
	memcpy(res, src, sizeof(*res));

	/* deep copy anyone? */
	switch (src->type) {
	case DEX_CONJ:
	case DEX_DISJ:
		res->left = dexpr_copy(src->left);
		res->right = dexpr_copy(src->right);
		break;
	case DEX_VAL:
	case DEX_UNK:
	default:
		break;
	}
	return res;
}

static dexpr_t
dexpr_copy_j(dexpr_t src)
{
/* copy SRC, but only if it's a junction (disjunction or conjunction) */
	if (src->type == DEX_VAL) {
		return (dexpr_t)src;
	}
	return dexpr_copy(src);
}

static void
__dnf(dexpr_t root)
{
/* recursive __dnf'er */
	switch (root->type) {
	case DEX_CONJ: {
		/* check if one of the children is a disjunction */
		dex_type_t rlt = root->left->type;
		dex_type_t rrt = root->right->type;

		if (rlt == DEX_DISJ && rrt == DEX_DISJ) {
			/* complexestest case
			 * (a|b)&(c|d) -> (a&c)|(a&d)|(b&c)|(b&d) */
			dexpr_t a;
			dexpr_t b;
			dexpr_t c;
			dexpr_t d;

			/* get the new idea of b and c */
			a = root->left->left;
			b = root->left->right;
			c = root->right->left;
			d = root->right->right;

			/* now reuse what's possible */
			root->type = DEX_DISJ;
			root->left->type = DEX_CONJ;
#if 0
			/* silly assignment, a comes from left->left */
			root->left->left = a;
#endif	/* 0 */
			root->left->right = c;

			root->right->type = DEX_DISJ;
			root->right->left = make_dexpr(DEX_CONJ);
			root->right->left->left = dexpr_copy_j(a);
			root->right->left->right = d;

			root->right->right = make_dexpr(DEX_DISJ);
			root->right->right->left = make_dexpr(DEX_CONJ);
			root->right->right->left->left = b;
			root->right->right->left->right = dexpr_copy_j(c);
			/* right side, finalise the right branches with CONJ */
			root->right->right->right = make_dexpr(DEX_CONJ);
			root->right->right->right->left = dexpr_copy_j(b);
			root->right->right->right->right = dexpr_copy_j(d);

		} else if (rlt == DEX_DISJ || rrt == DEX_DISJ) {
			/* ok'ish case
			 * a&(b|c) -> a&b|a&c
			 * the other case gets normalised: (a|b)&c -> c&(a|b) */
			dexpr_t a;
			dexpr_t b;
			dexpr_t c;

			/* put the non-DISJ case left */
			if ((a = root->left)->type == DEX_DISJ) {
				a = root->right;
				root->right = root->left;
			}
			/* get the new idea of b and c */
			b = root->right->left;
			c = root->right->right;

			/* turn into disjoint */
			root->type = DEX_DISJ;

			/* inflate left branch */
			root->left = make_dexpr(DEX_CONJ);
			root->left->left = a;
			root->left->right = b;

			/* rearrange this node now, reuse the right disjoint */
			root->right->type = DEX_CONJ;
			root->right->left = a;
			root->right->right = c;
		}
		/* fallthrough! */
	}
	case DEX_DISJ:
		/* nothing to be done other than a quick descent */
		__dnf(root->left);
		__dnf(root->right);

		/* upon ascent fixup double OR's */
		if (root->left->type == DEX_DISJ &&
		    root->right->type == DEX_DISJ) {
			/*      |             |
			 *    /   \          / \
			 *   |     |    ~>  a   |
			 *  / \   / \          / \
			 * a   b c   d        b   |
			 *                       / \
			 *                      c   d */
			dexpr_t i = root->left;
			dexpr_t j = root->right;
			dexpr_t a = i->left;
			dexpr_t b = i->right;
			dexpr_t c = j->left;

			root->left = a;
			root->right = i;
			i->left = b;
			i->right = j;
			j->left = c;

		} else if (root->left->type == DEX_DISJ) {
			/*     |           |
			 *    / \         / \
			 *   |   c   ~>  a   |
			 *  / \             / \
			 * a   b           b   c */
			dexpr_t i = root->left;
			dexpr_t c = root->right;
			dexpr_t a = i->left;
			dexpr_t b = i->right;

			root->left = a;
			root->right = i;
			i->left = b;
			i->right = c;
		}
		break;

	case DEX_VAL:
	case DEX_UNK:
	default:
		/* can't do anything to get the DNF going */
		break;
	}
	return;
}

static void
__nega_kv(struct dexkv_s *kv)
{
/* assume the parent dexpr has the nega flag set, negate KV */
	kv->op = ~kv->op;
	return;
}

static void
__denega(dexpr_t root)
{
	dexpr_t left;
	dexpr_t right;

	if (root->nega) {
		/* negate */
		root->nega = 0;

		switch (root->type) {
		case DEX_CONJ:
			/* !(a&b) -> !a | !b */
			root->type = DEX_DISJ;
			break;
		case DEX_DISJ:
			/* !(a|b) -> !a & !b */
			root->type = DEX_DISJ;
			break;
		case DEX_VAL:
			__nega_kv(root->kv);
			/* fallthrough */
		case DEX_UNK:
		default:
			return;
		}

		if ((left = root->left) != NULL) {
			left->nega = ~left->nega;
		}
		if ((right = root->right) != NULL) {
			right->nega = ~right->nega;
		}
	} else {
		switch (root->type) {
		case DEX_CONJ:
		case DEX_DISJ:
			left = root->left;
			right = root->right;
			break;
		case DEX_VAL:
		case DEX_UNK:
		default:
			return;
		}
	}
	/* descend */
	if (left != NULL) {
		__denega(left);
	}
	if (right != NULL) {
		__denega(right);
	}
	return;
}

static void
dexpr_simplify(dexpr_t root)
{
	__denega(root);
	__dnf(root);
	return;
}

static int
__cmp(struct dt_dt_s stream, struct dt_dt_s cell)
{
/* special promoting/demoting version of dt_dtcmp()
 * if CELL is d-only or t-only, demote STREAM */
	if (dt_sandwich_only_d_p(cell)) {
		return dt_dcmp(stream.d, cell.d);
	} else if (dt_sandwich_only_t_p(cell)) {
		return dt_tcmp(stream.t, cell.t);
	}
	return dt_dtcmp(stream, cell);
}


static bool
dexkv_matches_p(const_dexkv_t dkv, struct dt_dt_s d)
{
	signed int cmp;
	bool res;

	if (dkv->sp.spfl == DT_SPFL_N_STD) {
		if ((cmp = __cmp(d, dkv->d)) == -2) {
			return false;
		}
		switch (dkv->op) {
		case OP_UNK:
		case OP_EQ:
			res = cmp == 0;
			break;
		case OP_LT:
			res = cmp < 0;
			break;
		case OP_LE:
			res = cmp <= 0;
			break;
		case OP_GT:
			res = cmp > 0;
			break;
		case OP_GE:
			res = cmp >= 0;
			break;
		case OP_NE:
			res = cmp != 0;
			break;
		case OP_TRUE:
			res = true;
			break;
		default:
			res = false;
			break;
		}
		return res;
	}
	/* otherwise it's stuff that uses the S slot */
	switch (dkv->sp.spfl) {
	case DT_SPFL_N_YEAR:
		cmp = dt_get_year(d.d);
		break;
	case DT_SPFL_N_MON:
	case DT_SPFL_S_MON:
		cmp = dt_get_mon(d.d);
		break;
	case DT_SPFL_N_DCNT_MON:
		cmp = dt_get_mday(d.d);
		break;
	case DT_SPFL_N_DCNT_WEEK:
	case DT_SPFL_S_WDAY:
		cmp = dt_get_wday(d.d);
		break;
	case DT_SPFL_N_WCNT_MON:
		/* exotic function, needs extern'ing */
		cmp = dt_get_wcnt_mon(d.d);
		break;
	case DT_SPFL_N_DCNT_YEAR:
		cmp = dt_get_yday(d.d);
		break;
	case DT_SPFL_N_WCNT_YEAR:
		/* %C/%W week count */
		cmp = dt_get_wcnt_year(d.d, dkv->sp.wk_cnt);
		break;
	case DT_SPFL_N_STD:
	default:
		return false;
	}
	/* now do the actual comparison */
	switch (dkv->op) {
	case OP_EQ:
		res = dkv->s == cmp;
		break;
	case OP_LT:
		res = dkv->s < cmp;
		break;
	case OP_LE:
		res = dkv->s <= cmp;
		break;
	case OP_GT:
		res = dkv->s > cmp;
		break;
	case OP_GE:
		res = dkv->s >= cmp;
		break;
	case OP_NE:
		res = dkv->s != cmp;
		break;
	case OP_TRUE:
		res = true;
		break;
	default:
	case OP_UNK:
		res = false;
		break;
	}
	return res;
}

static bool
__conj_matches_p(const_dexpr_t dex, struct dt_dt_s d)
{
	const_dexpr_t a;

	for (a = dex; a->type == DEX_CONJ; a = a->right) {
		if (!dexkv_matches_p(a->left->kv, d)) {
			return false;
		}
	}
	/* rightmost cell might be a DEX_VAL */
	return dexkv_matches_p(a->kv, d);
}

static bool
__disj_matches_p(const_dexpr_t dex, struct dt_dt_s d)
{
	const_dexpr_t o;

	for (o = dex; o->type == DEX_DISJ; o = o->right) {
		if (__conj_matches_p(o->left, d)) {
			return true;
		}
	}
	/* rightmost cell may be a DEX_VAL */
	return __conj_matches_p(o, d);
}

static __attribute__((unused)) bool
dexpr_matches_p(const_dexpr_t dex, struct dt_dt_s d)
{
	return __disj_matches_p(dex, d);
}


#if defined STANDALONE
const char *prog = "dexpr";

int
main(int argc, char *argv[])
{
	dexpr_t root;

	for (int i = 1; i < argc; i++) {
		root = NULL;
		dexpr_parse(&root, argv[i], strlen(argv[i]));
		__pr(root, 0);
		fputc('\n', stdout);
		dexpr_simplify(root);
		__pr(root, 0);
		fputc('\n', stdout);
		/* also print an infix line */
		__pr_infix(root);
		fputc('\n', stdout);
		free_dexpr(root);
	}
	return 0;
}
#endif	/* STANDALONE */

/* dexpr.c ends here */
