#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dexpr.h"
#include "dexpr-parser.h"

static void
free_dexpr(dexpr_t root)
{
/* recursive free :( */
	if (root->left) {
		free_dexpr(root->left);
		free(root->left);
	}
	if (root->value) {
		switch (root->type) {
		case DEX_VAL:
		case DEX_NOT:
			free(root->value);
			break;
		case DEX_CONJ:
		case DEX_DISJ:
			free_dexpr(root->right);
			free(root->right);
			break;
		case DEX_UNK:
		default:
			break;
		}
	}
	return;
}

static void
__dnf(dexpr_t root)
{
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
		fprintf(stdout, "VAL %p\n", root->value);
		break;
	case DEX_NOT:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		fprintf(stdout, "NOT %p\n", root->value);
		break;

	case DEX_CONJ:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		fprintf(stdout, "AND\n");
		__pr(root->left, ind + 2);
		__pr(root->right, ind + 2);
		break;

	case DEX_DISJ:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		fprintf(stdout, "OR\n");
		__pr(root->left, ind + 2);
		__pr(root->right, ind + 2);
		break;

	case DEX_UNK:
	default:
		for (size_t i = 0; i < ind; i++) {
			fputc(' ', stdout);
		}
		fprintf(stdout, "UNK\n");
		break;
	}
	return;
}

int
main(int argc, char *argv[])
{
	struct dexpr_s root[1];

	for (int i = 1; i < argc; i++) {
		memset(root, 0, sizeof(root));
		dexpr_parse(root, argv[i], strlen(argv[i]));
		__pr(root, 0);
		free_dexpr(root);
	}
	return 0;
}

/* dexpr.c ends here */
