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
	if (root->type == DEX_CONJ &&
	    (root->left->type == DEX_DISJ && root->right->type == DEX_DISJ)) {
		/* lift, (a|b)&(c|d) -> (a&c)|(a&d)|(b&c)|(b&d) */
		dexpr_t a = root->left->left;
		dexpr_t b = root->left->right;
		dexpr_t c = root->right->left;
		dexpr_t d = root->right->right;

		/* start the rearrangement */
		root->type = DEX_DISJ;
		root->left->type = DEX_CONJ;
		root->left->left = a;
		root->left->right = c;

		root->right->type = DEX_DISJ;
		root->right->left = make_dexpr(DEX_CONJ);
		root->right->left->left = a;
		root->right->left->right = d;

		root->right->right = make_dexpr(DEX_DISJ);
		root->right->right->left = make_dexpr(DEX_CONJ);
		root->right->right->left->left = b;
		root->right->right->left->right = c;
		/* right side, finalise the right branches with a CONJ */
		root->right->right->right = make_dexpr(DEX_CONJ);
		root->right->right->right->left = b;
		root->right->right->right->right = d;

		/* now dnf'ify the leaves */
		__dnf(root->left);
		__dnf(root->right->left);
		__dnf(root->right->right->left);
		__dnf(root->right->right->right);
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
		__simplify(root);
		__pr(root, 0);
		free_dexpr(root);
	}
	return 0;
}

/* dexpr.c ends here */
