dnl compiler.m4 --- compiler magic
dnl
dnl Copyright (C) 2005-2008, 2012 Sebastian Freundt
dnl Copyright (c) 2005 Steven G. Johnson
dnl Copyright (c) 2005 Matteo Frigo
dnl
dnl Author: Sebastian Freundt <hroptatyr@sxemacs.org>
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl
dnl 3. Neither the name of the author nor the names of any contributors
dnl    may be used to endorse or promote products derived from this
dnl    software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
dnl IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
dnl DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
dnl FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
dnl CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
dnl SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
dnl BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
dnl WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
dnl OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
dnl IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl
dnl This file is part of SXEmacs.

AC_DEFUN([SXE_DEBUGFLAGS], [dnl
	## distinguish between different compilers, no?
	SXE_CHECK_COMPILER_FLAGS([-g])
	SXE_CHECK_COMPILER_FLAGS([-g3])

	AC_PATH_PROG([DBX], [dbx])
	if test -n "$ac_cv_path_DBX"; then
		SXE_CHECK_COMPILER_FLAGS([-gstabs])
		SXE_CHECK_COMPILER_FLAGS([-gstabs3])
		SXE_CHECK_COMPILER_FLAGS([-gxcoff])
		SXE_CHECK_COMPILER_FLAGS([-gxcoff3])
	fi

	AC_PATH_PROG([GDB], [gdb])
	if test -n "$ac_cv_path_GDB"; then
		SXE_CHECK_COMPILER_FLAGS([-ggdb])
		SXE_CHECK_COMPILER_FLAGS([-ggdb3])
	fi

	AC_PATH_PROG([SDB], [sdb])
	if test -n "$ac_cv_path_SDB"; then
		SXE_CHECK_COMPILER_FLAGS([-gcoff])
		SXE_CHECK_COMPILER_FLAGS([-gcoff3])
	fi

	## final evaluation
	debugflags=""
	## gdb
	if test "$sxe_cv_c_flags__ggdb3" = "yes"; then
		debugflags="$debugflags -ggdb3"
	elif test "$sxe_cv_c_flags__ggdb" = "yes"; then
		debugflags="$debugflags -ggdb"
	fi
	## stabs
	if test "$sxe_cv_c_flags__gstabs3" = "yes"; then
		debugflags="$debugflags -gstabs3"
	elif test "$sxe_cv_c_flags__gstabs" = "yes"; then
		debugflags="$debugflags -gstabs"
	fi
	## coff
	if test "$sxe_cv_c_flags__gcoff3" = "yes"; then
		debugflags="$debugflags -gcoff3"
	elif test "$sxe_cv_c_flags__gcoff" = "yes"; then
		debugflags="$debugflags -gcoff"
	fi
	## xcoff
	if test "$sxe_cv_c_flags__gxcoff3" = "yes"; then
		debugflags="$debugflags -gxcoff3"
	elif test "$sxe_cv_c_flags__gxcoff" = "yes"; then
		debugflags="$debugflags -gxcoff"
	fi

	if test -z "debugflags" -a \
		"$sxe_cv_c_flags__g" = "yes"; then
		debugflags="$debugflags -g"
	fi

	SXE_CHECK_COMPILER_FLAGS([-ftime-report])
	SXE_CHECK_COMPILER_FLAGS([-fmem-report])
	SXE_CHECK_COMPILER_FLAGS([-fvar-tracking])
	SXE_CHECK_COMPILER_FLAGS([-save-temps])

	#if test "$sxe_cv_c_flags__ggdb3" = "yes" -a \
	#	"$sxe_cv_c_flags__fvar_tracking" = "yes"; then
	#	debugflags="$debugflags -fvar-tracking"
	#fi

	AC_MSG_CHECKING([for preferred debugging flags])
	AC_MSG_RESULT([${debugflags}])
])dnl SXE_DEBUGFLAGS

AC_DEFUN([SXE_WARNFLAGS], [dnl
	## Calculate warning flags.  We separate the flags for warnings from
	## the other flags because we want to force the warnings to be seen
	## by everyone who doesn't specifically override them.

	## by default we want the -Wall level
	SXE_CHECK_COMPILER_FLAGS([-Wall], [warnflags="-Wall"])

	SXE_CHECK_COMPILER_FLAGS([-qinfo], [
		warnflags="${warnflags} -qinfo"])

	SXE_CHECK_COMPILER_FLAGS([-Wextra], [
		warnflags="${warnflags} -Wextra"])

	## Yuck, bad compares have been worth at
	## least 3 crashes!
	## Warnings about char subscripts are pretty
	## pointless, though,
	## and we use them in various places.
	SXE_CHECK_COMPILER_FLAGS([-Wsign-compare], [
		warnflags="$warnflags -Wsign-compare"])
	SXE_CHECK_COMPILER_FLAGS([-Wno-char-subscripts], [
		warnflags="$warnflags -Wno-char-subscripts"])
	SXE_CHECK_COMPILER_FLAGS([-Wundef], [
		warnflags="$warnflags -Wundef"])

	## too much at the moment, we rarely define protos
	#warnflags="$warnflags -Wmissing-prototypes -Wstrict-prototypes"
	SXE_CHECK_COMPILER_FLAGS([-Wpacked], [
		warnflags="$warnflags -Wpacked"])

	## glibc is intentionally not `-Wpointer-arith'-clean.
	## Ulrich Drepper has rejected patches to fix
	## the glibc header files.
	## we dont care
	SXE_CHECK_COMPILER_FLAGS([-Wpointer-arith], [
		warnflags="$warnflags -Wpointer-arith"])

	SXE_CHECK_COMPILER_FLAGS([-Wshadow], [
		warnflags="$warnflags -Wshadow"])

	## our code lacks declarations almost all the time
	SXE_CHECK_COMPILER_FLAGS([-Wmissing-declarations], [
		warnflags="$warnflags -Wmissing-declarations"])
	SXE_CHECK_COMPILER_FLAGS([-Wmissing-prototypes], [
		warnflags="$warnflags -Wmissing-prototypes"])
	SXE_CHECK_COMPILER_FLAGS([-Winline], [
		warnflags="$warnflags -Winline"])
	SXE_CHECK_COMPILER_FLAGS([-Wbad-function-cast], [
		warnflags="$warnflags -Wbad-function-cast"])
	SXE_CHECK_COMPILER_FLAGS([-Wcast-qual], [
		warnflags="$warnflags -Wcast-qual"])
	SXE_CHECK_COMPILER_FLAGS([-Wcast-align], [
		warnflags="$warnflags -Wcast-align"])

	## warn about incomplete switches
	SXE_CHECK_COMPILER_FLAGS([-Wswitch], [
		warnflags="$warnflags -Wswitch"])
	SXE_CHECK_COMPILER_FLAGS([-Wswitch-default], [
		warnflags="$warnflags -Wswitch-default"])
	SXE_CHECK_COMPILER_FLAGS([-Wswitch-enum], [
		warnflags="$warnflags -Wswitch-enum"])

	SXE_CHECK_COMPILER_FLAGS([-Wunused-function], [
		warnflags="$warnflags -Wunused-function"])
	SXE_CHECK_COMPILER_FLAGS([-Wunused-variable], [
		warnflags="$warnflags -Wunused-variable"])
	SXE_CHECK_COMPILER_FLAGS([-Wunused-parameter], [
		warnflags="$warnflags -Wunused-parameter"])
	SXE_CHECK_COMPILER_FLAGS([-Wunused-value], [
		warnflags="$warnflags -Wunused-value"])
	SXE_CHECK_COMPILER_FLAGS([-Wunused], [
		warnflags="$warnflags -Wunused"])
	SXE_CHECK_COMPILER_FLAGS([-Wmaybe-uninitialized], [
		warnflags="${warnflags} -Wmaybe-uninitialized"])

	SXE_CHECK_COMPILER_FLAGS([-Wnopragma], [
		warnflags="$warnflags -Wnopragma"])

	SXE_CHECK_COMPILER_FLAGS([-fdiagnostics-show-option], [
		warnflags="${warnflags} -fdiagnostics-show-option"])

	SXE_CHECK_COMPILER_FLAGS([-Wunknown-pragmas], [
		warnflags="$warnflags -Wunknown-pragmas"])
	SXE_CHECK_COMPILER_FLAGS([-Wuninitialized], [
		warnflags="$warnflags -Wuninitialized"])
	SXE_CHECK_COMPILER_FLAGS([-Wreorder], [
		warnflags="$warnflags -Wreorder"])
	SXE_CHECK_COMPILER_FLAGS([-Wdeprecated], [
		warnflags="$warnflags -Wdeprecated"])

	## icc specific
	SXE_CHECK_COMPILER_FLAGS([-diag-disable 10237], [dnl
		warnflags="${warnflags} -diag-disable 10237"], [
		SXE_CHECK_COMPILER_FLAGS([-wd 10237], [dnl
			warnflags="${warnflags} -wd 10237"])])
	SXE_CHECK_COMPILER_FLAGS([-w2], [
		warnflags="$warnflags -w2"])

	AC_MSG_CHECKING([for preferred warning flags])
	AC_MSG_RESULT([${warnflags}])
])dnl SXE_WARNFLAGS

AC_DEFUN([SXE_OPTIFLAGS], [dnl
	optiflags="-O3"
])dnl SXE_OPTIFLAGS

AC_DEFUN([SXE_FEATFLAGS], [dnl
	## if libtool then
	dnl XFLAG="-XCClinker"
	## default flags for needed features
	SXE_CHECK_COMPILER_FLAGS([-static-intel], [
		ldflags="${ldflags} ${XFLAG} -static-intel"])
	SXE_CHECK_COMPILER_FLAGS([-static-libgcc], [
		ldflags="${ldflags} ${XFLAG} -static-libgcc"])
])dnl SXE_FEATFLAGS


##### http://autoconf-archive.cryp.to/ax_check_compiler_flags.html
## renamed the prefix to SXE_
AC_DEFUN([SXE_CHECK_COMPILER_FLAGS], [dnl
dnl SXE_CHECK_COMPILER_FLAGS([flag], [do-if-works], [do-if-not-works])
	AC_MSG_CHECKING([whether _AC_LANG compiler accepts $1])

	dnl Some hackery here since AC_CACHE_VAL can't handle a non-literal varname:
	SXE_LANG_WERROR([push+on])
	AS_LITERAL_IF([$1], [
		AC_CACHE_VAL(AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1), [
			sxe_save_FLAGS=$[]_AC_LANG_PREFIX[]FLAGS
			_AC_LANG_PREFIX[]FLAGS="$1"
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
				AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="yes",
				AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="no")
			_AC_LANG_PREFIX[]FLAGS=$sxe_save_FLAGS])], [
		sxe_save_FLAGS=$[]_AC_LANG_PREFIX[]FLAGS
		_AC_LANG_PREFIX[]FLAGS="$1"
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
			eval AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="yes",
			eval AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="no")
		_AC_LANG_PREFIX[]FLAGS=$sxe_save_FLAGS])
	eval sxe_check_compiler_flags=$AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)
	SXE_LANG_WERROR([pop])

	AC_MSG_RESULT([$sxe_check_compiler_flags])
	if test "$sxe_check_compiler_flags" = "yes"; then
		m4_default([$2], :)
	else
		m4_default([$3], :)
	fi
])dnl SXE_CHECK_COMPILER_FLAGS


AC_DEFUN([SXE_CHECK_CFLAGS], [dnl
	dnl #### This may need to be overhauled so that all of SXEMACS_CC's flags
	dnl are handled separately, not just the xe_cflags_warning stuff.

	## Use either command line flag, environment var, or autodetection
	SXE_DEBUGFLAGS
	SXE_WARNFLAGS
	SXE_OPTIFLAGS
	SXE_FEATFLAGS
	SXE_CFLAGS="$debugflags $featflags $optiflags $warnflags"

	## unset the werror flag again
	SXE_LANG_WERROR([off])

	CFLAGS="$SXE_CFLAGS ${ac_cv_env_CFLAGS_value}"
	AC_MSG_CHECKING([for preferred CFLAGS])
	AC_MSG_RESULT([${CFLAGS}])

	AC_MSG_NOTICE([
If you wish to ADD your own flags you want to stop here and rerun the
configure script like so:
  configure CFLAGS=<to-be-added-flags>

You can always override the determined CFLAGS, partially or totally,
using
  make -C <directory> CFLAGS=<your-own-flags> [target]
or
  make CFLAGS=<your-own-flags> [target]
respectively
		])

	LDFLAGS="${ldflags} ${ac_cv_env_LDFLAGS_value}"
	AC_MSG_CHECKING([for preferred LDFLAGS])
	AC_MSG_RESULT([${LDFLAGS}])

])dnl SXE_CHECK_CFLAGS

AC_DEFUN([SXE_CHECK_CC], [dnl
dnl SXE_CHECK_CC([STANDARDS])
dnl standards are flavours supported by the compiler chosen with AC_PROG_CC
	pushdef([stds], m4_default([$1], [gnu11 c11 gnu99 c99]))

	AC_REQUIRE([AC_PROG_CC])

	for i in []stds[]; do
		SXE_CHECK_COMPILER_FLAGS([-std="${i}"], [
			std="-std=${i}"
			break
		])
	done

	AC_MSG_CHECKING([for preferred CC std])
	AC_MSG_RESULT([${std}])
	CC="${CC} ${std}"

	popdef([stds])
])dnl SXE_CHECK_CC

AC_DEFUN([SXE_CHECK_ANON_STRUCTS], [
	AC_MSG_CHECKING([whether C compiler can cope with anonymous structures])
	AC_LANG_PUSH([C])

	## backup our CFLAGS and unset it
	save_CFLAGS="${CFLAGS}"
	CFLAGS=""

	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
union __test_u {
	int i;
	struct {
		char c;
		char padc;
		short int pads;
	};
};
	]], [[
	union __test_u tmp = {.c = '4'};
	]])], [
		sxe_cv_have_anon_structs="yes"
	], [
		sxe_cv_have_anon_structs="no"
	])
	AC_MSG_RESULT([${sxe_cv_have_anon_structs}])

	## restore CFLAGS
	CFLAGS="${save_CFLAGS}"

	if test "${sxe_cv_have_anon_structs}" = "yes"; then
		AC_DEFINE([HAVE_ANON_STRUCTS], [1], [
			Whether c1x anon structs work])
		$1
		:
	else
		$2
		:
	fi
	AC_LANG_POP()
])dnl SXE_CHECK_ANON_STRUCTS


dnl sxe-compiler.m4 ends here
