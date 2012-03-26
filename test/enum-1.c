#include "dt-core.h"

int
main(void)
{
	int res = 0;

	if (DT_SANDWICH != DT_NTYP) {
		fprintf(stderr, "DT_SANDWICH %hu != DT_NTYP %hu\n",
			DT_SANDWICH, DT_NTYP);
		res = 1;
	}
	if (DT_SANDWICH_D != DT_SANDWICH + DT_ONLY_D) {
		fprintf(stderr, "DT_SANDWICH_D %hu != DT_SANDWICH + 1 %hu\n",
			DT_SANDWICH_D, DT_SANDWICH + DT_ONLY_D);
		res = 1;
	}
	if (DT_SANDWICH_T != DT_SANDWICH + DT_ONLY_T) {
		fprintf(stderr, "DT_SANDWICH_T %hu != DT_SANDWICH + 2 %hu\n",
			DT_SANDWICH_T, DT_SANDWICH + DT_ONLY_T);
		res = 1;
	}
	if (DT_PACK <= DT_NTYP) {
		fprintf(stderr, "DT_PACK %hu <= DT_NTYP %hu\n",
			DT_PACK, DT_NTYP);
		res = 1;
	}
	if (DT_SEXY <= DT_NTYP) {
		fprintf(stderr, "DT_SEXY %hu <= DT_NTYP %hu\n",
			DT_SEXY, DT_NTYP);
		res = 1;
	}
	return res;
}

/* enum-1.c ends here */
