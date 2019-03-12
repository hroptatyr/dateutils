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
conv_chk(struct dt_dt_s tes, struct dt_dt_s ref)
{
	int res = 0;

	CHECK(tes.typ != ref.typ,
	      "  TYPE DIFFERS %u ... should be %u\n",
	      (unsigned int)tes.typ,
	      (unsigned int)ref.typ);

	if (!ref.xxx) {
		CHECK(tes.xxx, "  FORMER DURATION BIT SET\n");
	} else {
		CHECK(!tes.xxx, "  FORMER DURATION BIT NOT SET\n");
	}
	if (!ref.neg) {
		CHECK(tes.neg, "  NEGATED BIT SET\n");
	} else {
		CHECK(!tes.neg, "  NEGATED BIT NOT SET\n");
	}

	if (tes.typ == DT_SEXY || tes.typ == DT_SEXYTAI) {
		/* make sure the padding leaves no garbage */
		CHECK_RES(1, tes.sexy != ref.sexy,
			  "  VALUES DIFFER %u v %u\n",
			  (unsigned int)tes.sexy, (unsigned int)ref.sexy);
	}
	return res;
}

int
main(void)
{
	int rc = 0;
	struct dt_dt_s t;
	struct dt_dt_s res;
	struct dt_dt_s chk;

	/* conv, then check */
	t = (struct dt_dt_s){DT_UNK};
	t.sandwich = 1;
	t.d.typ = DT_YMD;
	t.d.ymd.y = 2012;
	t.d.ymd.m = 6;
	t.d.ymd.d = 30;

	t.t.typ = DT_HMS;
	t.t.hms.h = 23;
	t.t.hms.m = 59;
	t.t.hms.s = 59;

	chk = (struct dt_dt_s){DT_UNK};
	chk.typ = DT_SEXY;
	chk.sexy = 1341100799;

	if (res = dt_dtconv(DT_SEXY, t), conv_chk(res, chk)) {
		rc = 1;
	}

	/* conv, then check */
	t = (struct dt_dt_s){DT_UNK};
	t.sandwich = 1;
	t.d.typ = DT_YMD;
	t.d.ymd.y = 2012;
	t.d.ymd.m = 7;
	t.d.ymd.d = 1;

	t.t.typ = DT_HMS;
	t.t.hms.h = 00;
	t.t.hms.m = 00;
	t.t.hms.s = 00;

	chk = (struct dt_dt_s){DT_UNK};
	chk.typ = DT_SEXY;
	chk.sexy = 1341100800;

	if (res = dt_dtconv(DT_SEXY, t), conv_chk(res, chk)) {
		rc = 1;
	}

#if defined WITH_LEAP_SECONDS
	/* conv, then check */
	t = (struct dt_dt_s){DT_UNK};
	t.sandwich = 1;
	t.d.typ = DT_YMD;
	t.d.ymd.y = 2012;
	t.d.ymd.m = 6;
	t.d.ymd.d = 30;

	t.t.typ = DT_HMS;
	t.t.hms.h = 23;
	t.t.hms.m = 59;
	t.t.hms.s = 59;

	chk = (struct dt_dt_s){DT_UNK};
	chk.typ = DT_SEXYTAI;
	chk.sexy = 1341100799 + 34;

	if (res = dt_dtconv(DT_SEXYTAI, t), conv_chk(res, chk)) {
		rc = 1;
	}

	/* conv, then check */
	t = (struct dt_dt_s){DT_UNK};
	t.sandwich = 1;
	t.d.typ = DT_YMD;
	t.d.ymd.y = 2012;
	t.d.ymd.m = 7;
	t.d.ymd.d = 1;

	t.t.typ = DT_HMS;
	t.t.hms.h = 00;
	t.t.hms.m = 00;
	t.t.hms.s = 00;

	chk = (struct dt_dt_s){DT_UNK};
	chk.typ = DT_SEXYTAI;
	chk.sexy = 1341100800 + 35;

	if (res = dt_dtconv(DT_SEXYTAI, t), conv_chk(res, chk)) {
		rc = 1;
	}
#endif	/* WITH_LEAP_SECONDS */
	return rc;
}

/* dtcore-conv.c ends here */
