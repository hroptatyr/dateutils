#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dexpr.h"
#include "dexpr-parser.h"

static void
free_dexpr(dexpr_t root)
{
/* recursive free :( */
	switch (root->type) {
	case DEX_CONJ:
	case DEX_DISJ:
		if (root->left) {
			free_dexpr(root->left);
			free(root->left);
		}
		if (root->right) {
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

static void
__pr_val(struct dexkv_s *kv)
{
	switch (kv->sp.spfl) {
	case DT_SPFL_N_MDAY:
		fputs("%d ", stdout);
		break;
	case DT_SPFL_N_MON:
	case DT_SPFL_S_MON:
		fputs("%b ", stdout);
		break;
	case DT_SPFL_N_YEAR:
		fputs("%Y ", stdout);
		break;
	case DT_SPFL_N_CNT_WEEK:
	case DT_SPFL_S_WDAY:
		fputs("%a ", stdout);
		break;
	case DT_SPFL_N_CNT_MON:
		fputs("%c ", stdout);
		break;
	case DT_SPFL_N_CNT_YEAR:
		fputs("%j ", stdout);
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
		dt_strfd(buf, sizeof(buf), NULL, kv->d);
		fputs(buf, stdout);
		break;
	}
	case DT_SPFL_N_MDAY:
		fprintf(stdout, "%02d", kv->s);
		break;
	case DT_SPFL_N_MON:
	case DT_SPFL_S_MON:
		if (kv->s >= 0 && kv->s <= 12) {
			fputs(__abbr_mon[kv->s], stdout);
		}
		break;
	case DT_SPFL_N_YEAR:
		fprintf(stdout, "%04d", kv->s);
		break;
	case DT_SPFL_N_CNT_WEEK:
	case DT_SPFL_S_WDAY:
		if (kv->s >= 0 && kv->s <= 7) {
			fputs(__abbr_wday[kv->s], stdout);
		}
		break;
	case DT_SPFL_N_CNT_MON:
		fprintf(stdout, "%02d", kv->s);
		break;
	case DT_SPFL_N_CNT_YEAR:
		fprintf(stdout, "%03d", kv->s);
		break;
	default:
		break;
	}
	return;
}

static void
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
		if (root->left) {
			fputs("ROOT\n", stderr);
			__pr(root->left, ind + 2);
			break;
		}
		break;
	}
	return;
}

static void
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
	dexpr_t res = calloc(1, sizeof(struct dexpr_s));
	res->type = type;
	return res;
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
			root->left->right = c;

			root->right->type = DEX_DISJ;
			root->right->left = make_dexpr(DEX_CONJ);
			root->right->left->left = a;
			root->right->left->right = d;

			root->right->right = make_dexpr(DEX_DISJ);
			root->right->right->left = make_dexpr(DEX_CONJ);
			root->right->right->left->left = b;
			root->right->right->left->right = c;
			/* right side, finalise the right branches with CONJ */
			root->right->right->right = make_dexpr(DEX_CONJ);
			root->right->right->right->left = b;
			root->right->right->right->right = d;

		} else if (rlt == DEX_DISJ || rrt == DEX_DISJ) {
			/* ok'ish case
			 * a&(b|c) -> a&b|a&c  or  (a|b)&c -> a&c|b&c */
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

			/* rearrange this node now, reuse the right disjoint */
			root->type = DEX_DISJ;
			root->right->type = DEX_CONJ;
			root->right->left = a;
			root->right->right = c;

			root->left = make_dexpr(DEX_CONJ);
			root->left->left = a;
			root->left->right = b;
		}
		/* fallthrough! */
	}
	case DEX_DISJ:
		/* nothing to be done other than a quick descent */
		__dnf(root->left);
		__dnf(root->right);

		if (root->left->type == DEX_DISJ &&
		    root->left->right->type == DEX_CONJ) {
			/* (a|(b&c)) | d  -> a | (b | c) */
			dexpr_t i = root->left;
			dexpr_t j = root->right;
			dexpr_t a = i->left;
			dexpr_t b = j->right;

			root->left = a;
			root->right = i;
			i->left = b;
			i->right = j;
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

		if ((left = root->left)) {
			left->nega = ~left->nega;
		}
		if ((right = root->right)) {
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
	if (left) {
		__denega(left);
	}
	if (right) {
		__denega(right);
	}
	return;
}

static void
__simplify(dexpr_t root)
{
	__denega(root);
	__dnf(root);
	return;
}


int
main(int argc, char *argv[])
{
	dexpr_t root;

	for (int i = 1; i < argc; i++) {
		root = NULL;
		dexpr_parse(&root, argv[i], strlen(argv[i]));
		__pr(root, 0);
		fputc('\n', stdout);
		__simplify(root);
		__pr_infix(root);
		fputc('\n', stdout);
		free_dexpr(root);
	}
	return 0;
}

/* dexpr.c ends here */
