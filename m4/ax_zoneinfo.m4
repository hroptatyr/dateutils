# ===========================================================================
#      http://www.gnu.org/software/autoconf-archive/ax_zoneinfo.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_ZONEINFO([])
#
# DESCRIPTION
#
#   This macro finds compiled zoneinfo files.  If successful it will define
#   HAVE_ZONEINFO per:
#
#     AC_DEFINE([HAVE_ZONEINFO], [1], [...])
#
#   and have the variable TZDIR point to the zoneinfo directory as per
#
#     AC_SUBST([TZDIR])
#     AC_DEFINE_UNQUOTED([TZDIR], [/path/to/zic/files], [...])
#
#   Moreover, if leap second fix-ups are available it will, similarly, define
#   HAVE_ZONEINFO_RIGHT and populate a variable TZRDIR.
#
#
# LICENSE
#
#   Copyright (c) 2012 Sebastian Freundt <freundt@fresse.org>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([AX_TZFILE], [dnl
	dnl not totally necessary (yet), as we can simply inspect the tzfiles
	dnl ourselves, but it certainly helps
	AC_CHECK_HEADER([tzfile.h])
])dnl AX_TZFILE

AC_DEFUN([AX_ZONEINFO], [dnl
	dnl we consider a zoneinfo directory properly populated when it
	dnl provides UTC or UCT or Universal or Zulu

	pushdef([check_tzdir], [dnl
		pushdef([dir], $]1[)dnl
		test -n []dir[] && test -d []dir[] dnl
		popdef([dir])dnl
	])dnl check_tzdir

	pushdef([check_tzfile], [dnl
		dnl check_tzfile([FILE], [IF-FOUND], [IF-NOT-FOUND])
		pushdef([file], $]1[)
		pushdef([if_found], $]2[)
		pushdef([if_not_found], $]3[)

		if test -n []file[] && test -f []file[] && \
			test -s []file[] && test -r []file[] && \
			test `head -c5 []file[]` = "TZif2"; then
				:
				if_found
		else
			:
			if_not_found
		fi

		popdef([file])
		popdef([if_found])
		popdef([if_not_found])
	])dnl check_tzfile

	dnl try /etc/localtime first, sometimes it's a link into TZDIR
	if test -L "/etc/localtime"; then
		TZDIR_cand="`readlink /etc/localtime` ${TZDIR_cand}"
	fi

	dnl oh, how about we try and check if there is a TZDIR already
	if check_tzdir(["${TZDIR}"]); then
		## bingo
		TZDIR_cand="${TZDIR} ${TZDIR_cand}"
	fi

	dnl often there's a tzselect util which contains the TZDIR path
	AC_PATH_PROG([TZSELECT], [tzselect])
	if test -n "${ac_cv_path_TZSELECT}"; then
		dnl snarf the value
		valtmp=$(mktemp)
		strings "${ac_cv_path_TZSELECT}" | \
			grep -F 'TZDIR=' > "${valtmp}"
		. "${valtmp}"
		TZDIR_cand="${TZDIR} ${TZDIR_cand}"
	fi

	dnl lastly, append the usual suspects
	TZDIR_cand="${TZDIR_cand} \
/usr/share/zoneinfo \
/usr/lib/zoneinfo \
/usr/local/etc/zoneinfo \
"

	dnl go through our candidates
	AC_CACHE_CHECK([for TZDIR], [ax_cv_zoneinfo_tzdir], [dnl
		for c in ${TZDIR_cand}; do
			ax_cv_zoneinfo_utc=""
			for f in "UTC" "UCT" "Universal" "Zulu"; do
				check_tzfile(["${c}/${f}"], [
					dnl ACTION-IF-FOUND
					ax_cv_zoneinfo_utc="${c}/${f}"
					break
				])
			done
			if test -n "${ax_cv_zoneinfo_utc}"; then
				ax_cv_zoneinfo_tzdir="${c}"
				break
			fi
		done
	])dnl ax_cv_tzdir

	TZDIR="${ax_cv_zoneinfo_tzdir}"
	AC_SUBST([TZDIR])

	if check_tzdir(["${ax_cv_zoneinfo_tzdir}"]); then
		AC_DEFINE([HAVE_ZONEINFO], [1], [dnl
Define when zoneinfo directory has been present during configuration.])
		AC_DEFINE_UNQUOTED([TZDIR], ["${ax_cv_zoneinfo_tzdir}"], [
Configuration time zoneinfo directory.])
	fi

	popdef([check_tzfile])
	popdef([check_tzdir])
])dnl AX_ZONEINFO

AC_DEFUN([AX_ZONEINFO_RIGHT], [dnl
	AC_REQUIRE([AX_ZONEINFO])

	pushdef([check_tzrfile], [dnl
		dnl check_tzrfile([FILE], [IF-FOUND], [IF-NOT-FOUND])
		pushdef([file], $]1[)
		pushdef([if_found], $]2[)
		pushdef([if_not_found], $]3[)

		if test -n []file[] && test -f []file[] && \
			test -s []file[] && test -r []file[]; then
			AC_LANG_PUSH([C])
			AC_RUN_IFELSE(AC_LANG_PROGRAM([
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

/* simplified struct */
struct tzhead {
	char	tzh_magic[4];		/* TZ_MAGIC */
	char	tzh_version[1];		/* '\0' or '2' as of 2005 */
	char	tzh_reserved[15];	/* reserved--must be zero */
	char	tzh_ttisgmtcnt[4];	/* coded number of trans. time flags */
	char	tzh_ttisstdcnt[4];	/* coded number of trans. time flags */
	char	tzh_leapcnt[4];		/* coded number of leap seconds */
	char	tzh_timecnt[4];		/* coded number of transition times */
	char	tzh_typecnt[4];		/* coded number of local time types */
	char	tzh_charcnt[4];		/* coded number of abbr. chars */
};
], [[
	struct tzhead foo;
	int f;

	if ((f = open(argv[1], O_RDONLY, 0644)) < 0) {
		return 1;
	} else if (read(f, &foo, sizeof(foo)) != sizeof(foo)) {
		return 1;
	} else if (close(f) < 0) {
		return 1;
	}

	/* inspect the header */
	if (strcmp(foo.tzh_magic, "TZif")) {
		return 1;
	} else if (!*foo.tzh_version && *foo.tzh_version != '2') {
		return 1;
	} else if (!foo.tzh_leapcnt[0] && !foo.tzh_leapcnt[1] &&
		   !foo.tzh_leapcnt[2] && !foo.tzh_leapcnt[3]) {
		return 2;
	}

	/* otherwise everything's in order */
	return 0;
]], [
			]if_found[
], [
			]if_not_found[]))
			AC_LANG_POP([C])
		else
			:
			[]if_not_found[]
		fi

		popdef([file])
		popdef([if_found])
		popdef([if_not_found])
	])dnl check_tzfile

	TZDIR_cand="${TZDIR} \
${TZDIR}/leapseconds \
${TZDIR}-leaps \
${TZDIR}/right \
${TZDIR}-posix \
${TZDIR}/posix \
"

	dnl go through our candidates
	AC_CACHE_CHECK([for leap second file], [ax_cv_zoneinfo_rutc], [dnl
		__utc_file="`basename '${ax_cv_zoneinfo_utc}'`"
		for c in ${TZDIR_cand}; do
			if test -d "${c}"; then
				c="${c}/${__utc_file}"
			fi
			check_tzrfile(["${c}"], [
				dnl ACTION-IF-FOUND
				ax_cv_zoneinfo_rutc="${c}"
				break
			])
		done
		ax_cv_zoneinfo_tzrdir="`dirname '${c}'`"
	])dnl ax_cv_tzdir

	TZRDIR="${ax_cv_zoneinfo_tzrdir}"
	AC_SUBST([TZRDIR])

	if test -n "${ax_cv_zoneinfo_tzrdir}"; then
		AC_DEFINE([HAVE_ZONEINFO], [1], [dnl
Define when zoneinfo directory has been present during configuration.])
		AC_DEFINE_UNQUOTED([TZRDIR], ["${ax_cv_zoneinfo_tzrdir}"], [
Configuration time zoneinfo directory.])
	fi

	popdef([check_tzrfile])
])dnl AX_ZONEINFO

dnl ax_zoneinfo.m4 ends here
