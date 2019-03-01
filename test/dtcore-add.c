#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "dt-core.h"

#define CHECK_RES(rc, pred, args...)		\
	if (pred) {				\
		fprintf(stderr, args);		\
		res = rc;			\
	}

#define CHECK(pred, args...)			\
	CHECK_RES(1, pred, args)

#define CHECK_EQ(slot, val, args...)		\
	CHECK(slot != val, args, slot, val)

static int
add_d_only(void)
{
	static const char str[] = "2012-03-28";
	struct dt_dt_s d;
	struct dt_dtdur_s dur;
	int res = 0;

	/* 2012-03-28 (using no format) */
	fprintf(stderr, "testing %s +1d ...\n", str);
	d = dt_strpdt(str, NULL, NULL);

	/* prep the duration */
	dur.d = dt_make_ddur(DT_DURD, 1);
	dur.t = (struct dt_t_s){DT_TUNK};

	/* the actual addition */
	d = dt_dtadd(d, dur);

	CHECK(d.d.typ != DT_YMD,
	      "  TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.d.typ,
	      (unsigned int)DT_YMD);
	CHECK(d.t.u,
	      "  TIME COMPONENT NOT NAUGHT %" PRIu64 "\n",
	      (uint64_t)d.t.u);
	CHECK(d.xxx, "  FORMER DURATION BIT SET\n");
	CHECK(d.neg, "  NEGATED BIT SET\n");
	CHECK(d.t.dur, "  TIME DURATION BIT SET\n");
	CHECK(d.t.neg, "  TIME NEGATED BIT SET\n");

	CHECK_EQ((unsigned int)d.d.ymd.y, 2012U,
		 "  YEAR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.m, 3U,
		 "  MONTH %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.d, 29U,
		 "  DAY %u ... should be %u\n");
	/* make sure the padding leaves no garbage */
	CHECK_RES(res, d.d.ymd.u & ~0x1fffff,
		  "  PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.d.ymd.u & ~0x1fffff));
	return res;
}

static int
add_t_only(void)
{
	static const char str[] = "12:34:56";
	struct dt_dt_s d;
	struct dt_dtdur_s dur = {(dt_dtdurtyp_t)DT_DURUNK};
	int res = 0;

	/* 2012-03-28 (using no format) */
	fprintf(stderr, "testing %s +1h ...\n", str);
	d = dt_strpdt(str, NULL, NULL);

	/* prep the duration */
	dur.durtyp = DT_DURS;
	dur.dv = 3600;

	/* the actual addition */
	d = dt_dtadd(d, dur);

	CHECK(d.typ != DT_SANDWICH_UNK,
	      "  TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.typ,
	      (unsigned int)DT_SANDWICH_UNK);
	CHECK(d.t.typ != DT_HMS,
	      "  TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.t.typ,
	      (unsigned int)DT_HMS);
	CHECK(d.d.u,
	      "  DATE COMPONENT NOT NAUGHT %" PRIu64 "\n",
	      (uint64_t)d.d.u);
	CHECK(d.xxx, "  FORMER DURATION BIT SET\n");
	CHECK(d.neg, "  NEGATED BIT SET\n");
	CHECK(d.t.dur, "  TIME DURATION BIT SET\n");
	CHECK(d.t.neg, "  TIME NEGATED BIT SET\n");

	CHECK_EQ((unsigned int)d.t.hms.h, 13U,
		 "  HOUR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.m, 34U,
		 "  MINUTE %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.s, 56U,
		 "  SECOND %u ... should be %u\n");
	/* make sure the padding leaves no garbage */
	CHECK_RES(res, d.t.hms.u & ~0x1f3f3f3fffffff,
		  "  PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.t.hms.u & ~0x1f3f3f3fffffff));
	return res;
}

static int
dt_add_d(void)
{
	static const char str[] = "2012-03-28T12:34:56";
	struct dt_dt_s d;
	struct dt_dtdur_s dur;
	int res = 0;

	fprintf(stderr, "testing %s +1d ...\n", str);
	d = dt_strpdt(str, NULL, NULL);

	/* prep the duration */
	dur.d = dt_make_ddur(DT_DURD, 1);
	dur.t = (struct dt_t_s){DT_TUNK};

	/* the actual addition */
	d = dt_dtadd(d, dur);

	CHECK(d.d.typ != DT_YMD,
	      "  TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.d.typ,
	      (unsigned int)DT_YMD);
	CHECK(d.xxx, "  FORMER DURATION BIT SET\n");
	CHECK(d.neg, "  NEGATED BIT SET\n");
	CHECK(d.t.dur, "  TIME DURATION BIT SET\n");
	CHECK(d.t.neg, "  TIME NEGATED BIT SET\n");

	CHECK_EQ((unsigned int)d.d.ymd.y, 2012U,
		 "  YEAR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.m, 3U,
		 "  MONTH %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.d, 29U,
		 "  DAY %u ... should be %u\n");

	CHECK_EQ((unsigned int)d.t.hms.h, 12U,
		 "  HOUR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.m, 34U,
		 "  MINUTE %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.s, 56U,
		 "  SECOND %u ... should be %u\n");

	/* make sure the padding leaves no garbage */
	CHECK_RES(res, d.d.ymd.u & ~0x1fffff,
		  "  PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.d.ymd.u & ~0x1fffff));
	CHECK_RES(res, d.t.hms.u & ~0x1f3f3f3fffffff,
		  "  TIME PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.t.hms.u & ~0x1f3f3f3fffffff));
	return res;
}

static int
dt_add_t(void)
{
	static const char str[] = "2012-03-28T23:12:01";
	struct dt_dt_s d;
	struct dt_dtdur_s dur = {(dt_dtdurtyp_t)DT_DURUNK};
	int res = 0;

	fprintf(stderr, "testing %s +1h ...\n", str);
	d = dt_strpdt(str, NULL, NULL);

	/* prep the duration */
	dur.durtyp = DT_DURS;
	dur.dv = 3600;

	/* the actual addition */
	d = dt_dtadd(d, dur);

	CHECK(d.d.typ != DT_YMD,
	      "  DATE TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.d.typ,
	      (unsigned int)DT_YMD);
	CHECK(d.t.typ != DT_HMS,
	      "  TIME TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.t.typ,
	      (unsigned int)DT_HMS);
	CHECK(d.xxx, "  FORMER DURATION BIT SET\n");
	CHECK(d.neg, "  NEGATED BIT SET\n");
	CHECK(d.t.dur, "  TIME DURATION BIT SET\n");
	CHECK(d.t.neg, "  TIME NEGATED BIT SET\n");

	CHECK_EQ((unsigned int)d.d.ymd.y, 2012U,
		 "  YEAR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.m, 3U,
		 "  MONTH %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.d, 29U,
		 "  DAY %u ... should be %u\n");

	CHECK_EQ((unsigned int)d.t.hms.h, 00U,
		 "  HOUR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.m, 12U,
		 "  MINUTE %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.s, 01U,
		 "  SECOND %u ... should be %u\n");

	/* make sure the padding leaves no garbage */
	CHECK_RES(res, d.d.ymd.u & ~0x1fffff,
		  "  PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.d.ymd.u & ~0x1fffff));
	CHECK_RES(res, d.t.hms.u & ~0x1f3f3f3fffffff,
		  "  TIME PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.t.hms.u & ~0x1f3f3f3fffffff));
	return res;
}

static int
dt_add_dt(void)
{
	static const char str[] = "2012-03-28T23:55:55";
	struct dt_dt_s d;
	struct dt_dtdur_s dur = {(dt_dtdurtyp_t)DT_DURUNK};
	int res = 0;

	fprintf(stderr, "testing %s +1d1h ...\n", str);
	d = dt_strpdt(str, NULL, NULL);

	/* prep the duration */
	dur.d = dt_make_ddur(DT_DURD, 1);
	/* addition 1 */
	d = dt_dtadd(d, dur);
	/* duration 2 */
	dur.durtyp = DT_DURH;
	dur.dv = 1;
	/* addition 2 */
	d = dt_dtadd(d, dur);

	CHECK(d.d.typ != DT_YMD,
	      "  DATE TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.d.typ,
	      (unsigned int)DT_YMD);
	CHECK(d.t.typ != DT_HMS,
	      "  TIME TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)d.t.typ,
	      (unsigned int)DT_HMS);
	CHECK(d.xxx, "  FORMER DURATION BIT SET\n");
	CHECK(d.neg, "  NEGATED BIT SET\n");
	CHECK(d.t.dur, "  TIME DURATION BIT SET\n");
	CHECK(d.t.neg, "  TIME NEGATED BIT SET\n");

	CHECK_EQ((unsigned int)d.d.ymd.y, 2012U,
		 "  YEAR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.m, 3U,
		 "  MONTH %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.d.ymd.d, 30U,
		 "  DAY %u ... should be %u\n");

	CHECK_EQ((unsigned int)d.t.hms.h, 00U,
		 "  HOUR %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.m, 55U,
		 "  MINUTE %u ... should be %u\n");
	CHECK_EQ((unsigned int)d.t.hms.s, 55U,
		 "  SECOND %u ... should be %u\n");

	/* make sure the padding leaves no garbage */
	CHECK_RES(res, d.d.ymd.u & ~0x1fffff,
		  "  PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.d.ymd.u & ~0x1fffff));
	CHECK_RES(res, d.t.hms.u & ~0x1f3f3f3fffffff,
		  "  TIME PADDING NOT NAUGHT %x\n",
		  (unsigned int)(d.t.hms.u & ~0x1f3f3f3fffffff));
	return res;
}

int
main(void)
{
	int res = 0;

	/* we just assume the parser works */
	if (add_d_only() != 0) {
		res = 1;
	}

	if (add_t_only() != 0) {
		res = 1;
	}

	if (dt_add_d() != 0) {
		res = 1;
	}

	if (dt_add_t() != 0) {
		res = 1;
	}

	if (dt_add_dt() != 0) {
		res = 1;
	}

	return res;
}

/* dtcore-add.c ends here */
