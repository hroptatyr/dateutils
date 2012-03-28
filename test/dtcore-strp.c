#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "dt-core.h"

static int
test_d_only_no_fmt(void)
{
	static const char str[] = "2012-03-28";
	struct dt_dt_s d;
	int res = 0;

	/* 2012-03-28 (using no format) */
	fprintf(stderr, "testing %s ...\n", str);
	d = dt_strpdt(str, NULL, NULL);

	if (d.typ != DT_SANDWICH_D_ONLY(DT_YMD)) {
		fprintf(stderr, "  TYPE DIFFERS %u ... should be %u\n",
			(unsigned int)d.typ,
			(unsigned int)DT_SANDWICH_D_ONLY(DT_YMD));
		res = 1;
	}
	if (d.t.u) {
		fprintf(stderr, "  TIME COMPONENT NOT NAUGHT %" PRIu64 "\n",
			(uint64_t)d.t.u);
		res = 1;
	}
	if (d.dur) {
		fprintf(stderr, "  DURATION BIT SET\n");
		res = 1;
	}
	if (d.neg) {
		fprintf(stderr, "  NEGATED BIT SET\n");
		res = 1;
	}
	if (d.d.ymd.y != 2012) {
		fprintf(stderr, "  YEAR %u ... should be 2012\n",
			(unsigned int)d.d.ymd.y);
		res = 1;
	}
	if (d.d.ymd.m != 3) {
		fprintf(stderr, "  MONTH %u ... should be 3\n",
			(unsigned int)d.d.ymd.m);
		res = 1;
	}
	if (d.d.ymd.d != 28) {
		fprintf(stderr, "  DAY %u ... should be 28\n",
			(unsigned int)d.d.ymd.d);
		res = 1;
	}
	/* make sure the padding leaves no garbage */
	if (d.d.ymd.u & ~0x1fffff) {
		fprintf(stderr, "  PADDING NOT NAUGHT %u\n",
			(unsigned int)(d.d.ymd.u & ~0x1fffff));
		res = 1;
	}
	return res;
}

static int
test_t_only_no_fmt(void)
{
	static const char str[] = "12:34:56";
	struct dt_dt_s d;
	int res = 0;

	/* 12:34:56 (using no format) */
	fprintf(stderr, "testing %s ...\n", str);
	d = dt_strpdt(str, NULL, NULL);

	if (d.typ != DT_SANDWICH_T_ONLY(DT_HMS)) {
		fprintf(stderr, "  TYPE DIFFERS %u ... should be %u\n",
			(unsigned int)d.typ,
			(unsigned int)DT_SANDWICH_T_ONLY(DT_HMS));
		res = 1;
	}
	if (d.t.typ != DT_HMS) {
		fprintf(stderr, "  TIME TYPE DIFFERS %u ... should be %u\n",
			(unsigned int)d.t.typ,
			(unsigned int)DT_HMS);
		res = 1;
	}
	if (d.d.u) {
		fprintf(stderr, "  DATE COMPONENT NOT NAUGHT %" PRIu64 "\n",
			(uint64_t)d.d.u);
		res = 1;
	}
	if (d.dur) {
		fprintf(stderr, "  DURATION BIT SET\n");
		res = 1;
	}
	if (d.neg) {
		fprintf(stderr, "  NEGATED BIT SET\n");
		res = 1;
	}
	if (d.t.hms.h != 12) {
		fprintf(stderr, "  HOUR %u ... should be 12\n",
			(unsigned int)d.t.hms.h);
		res = 1;
	}
	if (d.t.hms.m != 34) {
		fprintf(stderr, "  MINUTE %u ... should be 34\n",
			(unsigned int)d.t.hms.m);
		res = 1;
	}
	if (d.t.hms.s != 56) {
		fprintf(stderr, "  SECOND %u ... should be 56\n",
			(unsigned int)d.t.hms.s);
		res = 1;
	}
	if (d.t.hms.ns != 0) {
		fprintf(stderr, "  NANOSECOND %u ... should be 0\n",
			(unsigned int)d.t.hms.ns);
		res = 1;
	}
	/* make sure the padding leaves no garbage */
	if (d.t.hms.u & ~0x1f3f3f3fffffff) {
		fprintf(stderr, "  PADDING NOT NAUGHT %u\n",
			(unsigned int)(d.t.hms.u & ~0x1f3f3f3fffffff));
		res = 1;
	}
	return res;
}

int
main(void)
{
	int res = 0;

	if (test_d_only_no_fmt() != 0) {
		res = 1;
	}

	if (test_t_only_no_fmt() != 0) {
		res = 1;
	}

	return res;
}

/* dtcore-strpd.c ends here */
