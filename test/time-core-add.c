#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "time-core.h"

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
add_chk(struct dt_t_s tes, struct dt_t_s ref)
{
	int res = 0;

	CHECK(tes.typ != ref.typ,
	      "  TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)tes.typ,
	      (unsigned int)ref.typ);

	if (!ref.dur) {
		CHECK(tes.dur, "  DURATION BIT SET\n");
	} else {
		CHECK(!tes.dur, "  DURATION BIT NOT SET\n");
	}
	if (!ref.neg) {
		CHECK(tes.neg, "  NEGATED BIT SET\n");
	} else {
		CHECK(!tes.neg, "  NEGATED BIT NOT SET\n");
	}

	if (tes.typ == DT_HMS) {
		CHECK_EQ((unsigned int)tes.hms.h, (unsigned int)ref.hms.h,
			 "  HOUR %u ... should be %u\n");
		CHECK_EQ((unsigned int)tes.hms.m, (unsigned int)ref.hms.m,
			 "  MINUTE %u ... should be %u\n");
		CHECK_EQ((unsigned int)tes.hms.s, (unsigned int)ref.hms.s,
			 "  SECOND %u ... should be %u\n");
		/* make sure the padding leaves no garbage */
		CHECK_RES(res, tes.hms.u & ~0x1f3f3f3fffffff,
			  "  PADDING NOT NAUGHT %x\n",
			  (unsigned int)(tes.hms.u & ~0x1f3f3f3fffffff));
	}

	CHECK(tes.carry != ref.carry,
	      "  CARRY DIFFERS %d ... should be %d\n",
	      (signed int)tes.carry,
	      (signed int)ref.carry);
	return res;
}

int
main(void)
{
	int rc = 0;
	struct dt_t_s t;
	int dur;
	struct dt_t_s res;
	struct dt_t_s chk;

	/* 12:34:56 + 17s */
	t = (struct dt_t_s){DT_TUNK};
	t.typ = DT_HMS;
	t.hms.h = 12;
	t.hms.m = 34;
	t.hms.s = 56;

	dur = 17;

	/* should be 12:35:13 */
	chk = (struct dt_t_s){DT_TUNK};
	chk.typ = DT_HMS;
	chk.hms.h = 12;
	chk.hms.m = 35;
	chk.hms.s = 13;

	/* add, then check */
	if (res = dt_tadd_s(t, dur, 0), add_chk(res, chk)) {
		rc = 1;
	}


	/* 12:34:56 + 11*3600s + 25*60s + 4s */
	t = (struct dt_t_s){DT_TUNK};
	t.typ = DT_HMS;
	t.hms.h = 12;
	t.hms.m = 34;
	t.hms.s = 56;

	dur = 11 * 3600 + 25 * 60 + 4;

	/* should be 00:00:00 */
	chk = (struct dt_t_s){DT_TUNK};
	chk.typ = DT_HMS;
	chk.hms.h = 00;
	chk.hms.m = 00;
	chk.hms.s = 00;
	chk.carry = 1;

	/* add, then check */
	if (res = dt_tadd_s(t, dur, 0), add_chk(res, chk)) {
		rc = 1;
	}


	/* 12:34:56 + 11*3600s + 25*60s + 4s on a leap day */
	t = (struct dt_t_s){DT_TUNK};
	t.typ = DT_HMS;
	t.hms.h = 12;
	t.hms.m = 34;
	t.hms.s = 56;

	dur = 11 * 3600 + 25 * 60 + 4;

	/* should be 00:00:00 */
	chk = (struct dt_t_s){DT_TUNK};
	chk.typ = DT_HMS;
	chk.hms.h = 23;
	chk.hms.m = 59;
	chk.hms.s = 60;
	chk.carry = 0;

	/* add, then check */
	if (res = dt_tadd_s(t, dur, 1), add_chk(res, chk)) {
		rc = 1;
	}


	/* 12:34:56 + 11*3600s + 25*60s + 3s on a -leap day */
	t = (struct dt_t_s){DT_TUNK};
	t.typ = DT_HMS;
	t.hms.h = 12;
	t.hms.m = 34;
	t.hms.s = 56;

	dur = 11 * 3600 + 25 * 60 + 3;

	/* should be 00:00:00 */
	chk = (struct dt_t_s){DT_TUNK};
	chk.typ = DT_HMS;
	chk.hms.h = 00;
	chk.hms.m = 00;
	chk.hms.s = 00;
	chk.carry = 1;

	/* add, then check */
	if (res = dt_tadd_s(t, dur, -1), add_chk(res, chk)) {
		rc = 1;
	}


	/* 12:34:56 + 11*3600s + 25*60s + 2s on a -leap day */
	t = (struct dt_t_s){DT_TUNK};
	t.typ = DT_HMS;
	t.hms.h = 12;
	t.hms.m = 34;
	t.hms.s = 56;

	dur = 11 * 3600 + 25 * 60 + 2;

	/* should be 00:00:00 */
	chk = (struct dt_t_s){DT_TUNK};
	chk.typ = DT_HMS;
	chk.hms.h = 23;
	chk.hms.m = 59;
	chk.hms.s = 58;
	chk.carry = 0;

	/* add, then check */
	if (res = dt_tadd_s(t, dur, -1), add_chk(res, chk)) {
		rc = 1;
	}


	/* 12:34:56 + 11*3600s + 25*60s + 3s on a +leap day */
	t = (struct dt_t_s){DT_TUNK};
	t.typ = DT_HMS;
	t.hms.h = 12;
	t.hms.m = 34;
	t.hms.s = 56;

	dur = 11 * 3600 + 25 * 60 + 3;

	/* should be 00:00:00 */
	chk = (struct dt_t_s){DT_TUNK};
	chk.typ = DT_HMS;
	chk.hms.h = 23;
	chk.hms.m = 59;
	chk.hms.s = 59;
	chk.carry = 0;

	/* add, then check */
	if (res = dt_tadd_s(t, dur, 1), add_chk(res, chk)) {
		rc = 1;
	}
	return rc;
}

/* time-core-add.c ends here */
