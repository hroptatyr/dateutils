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

int
main(int argc, char *argv[])
{
	dexpr_t root;

	for (int i = 1; i < argc; i++) {
		root = NULL;
		dexpr_parse(&root, argv[i], strlen(argv[i]));
		__pr(root, 0);
		free_dexpr(root);
	}
	return 0;
}

/* dexpr.c ends here */
