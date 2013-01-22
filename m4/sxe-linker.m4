dnl linker.m4 --- linker magic
dnl
dnl Copyright (C) 2005 - 2012 Sebastian Freundt
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

AC_DEFUN([SXE_CHECK_LINKER_FLAGS], [dnl
dnl just like SXE_CHECK_COMPILER_FLAGS but calls the linker
dnl SXE_CHECK_LINKER_FLAGS(<FLAG>, <ACTION-IF-FOUND>, <ACTION-IF-NOT-FOUND>,
dnl     <ADDITIONAL-FLAGS>)
	AC_REQUIRE([SXE_CHECK_COMPILER_XFLAG])

	AC_MSG_CHECKING([whether linker accepts $1])

	dnl Some hackery here since AC_CACHE_VAL can't handle a non-literal varname:
	AC_LANG_WERROR([on])
	AS_LITERAL_IF([$1], [
		AC_CACHE_VAL(AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1), [
			sxe_save_FLAGS=$[]_AC_LANG_PREFIX[]FLAGS
			_AC_LANG_PREFIX[]FLAGS="$4 ${XFLAG} $1"
			AC_LINK_IFELSE([AC_LANG_PROGRAM()],
				AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="yes",
				AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="no")
			_AC_LANG_PREFIX[]FLAGS=$sxe_save_FLAGS])], [
		sxe_save_FLAGS=$[]_AC_LANG_PREFIX[]FLAGS
		_AC_LANG_PREFIX[]FLAGS="$4 ${XFLAG} $1"
		AC_LINK_IFELSE([AC_LANG_PROGRAM()],
			eval AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="yes",
			eval AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)="no")
		_AC_LANG_PREFIX[]FLAGS=$sxe_save_FLAGS])
	eval sxe_check_linker_flags=$AS_TR_SH(sxe_cv_[]_AC_LANG_ABBREV[]_flags_$1)
	AC_LANG_WERROR([off])

	AC_MSG_RESULT([$sxe_check_linker_flags])
	if test "$sxe_check_linker_flags" = "yes"; then
		m4_default([$2], :)
	else
		m4_default([$3], :)
	fi
])dnl SXE_CHECK_LINKER_FLAGS


AC_DEFUN([SXE_CHECK_LD_ZFLAG], [dnl
	pushdef([LD_ZFLAG], [$1])
	pushdef([cv_zflag], [sxe_cv_ld__z_]translit(LD_ZFLAG,[-.=],[___]))

	SXE_CHECK_LINKER_FLAGS([-z ]LD_ZFLAG[])

dnl Use the check that actually calls the compiler/linker to examine the flags
dnl	AC_CACHE_CHECK([whether linker supports -z ]LD_ZFLAG[],
dnl		[]cv_zflag[], [_SXE_CHECK_LD_ZFLAG(LD_ZFLAG)])

	popdef([cv_zflag])
	popdef([LD_ZFLAG])
])dnl SXE_CHECK_LD_ZFLAG

AC_DEFUN([_SXE_CHECK_LD_ZFLAG], [dnl
	## arg1 is the flag to check for
	pushdef([LD_ZFLAG], [$1])
	pushdef([cv_zflag], [sxe_cv_ld__z_]translit(LD_ZFLAG,[-.=],[___]))

	if test "$GCC" = "yes"; then
		if test "($CC -Xlinker --help 2>&1 | \
			grep \"-z []LD_ZFLAG[]\" > /dev/null 2>&1 ) "; then
			cv_zflag="yes"
		else
			cv_zflag="no"
		fi
	elif test -n "$LD"; then
		if test "($LD --help 2>&1 | \
			grep \"-z []LD_ZFLAG[]\" > /dev/null 2>&1 )"; then
			cv_zflag="yes"
		else
			cv_zflag="no"
		fi
	else
		cv_zflag="no"
	fi

	popdef([cv_zflag])
	popdef([LD_ZFLAG])
])dnl _SXE_CHECK_LD_ZFLAG

AC_DEFUN([SXE_LD_EXPORT_DYNAMIC], [dnl
	SXE_CHECK_LINKER_FLAGS([-export-dynamic], [
		LD_EXPORT_DYNAMIC="-export-dynamic"
	])

	AC_SUBST([LD_EXPORT_DYNAMIC])
])dnl SXE_LD_EXPORT_DYNAMIC

AC_DEFUN([SXE_CHECK_LDFLAGS], [dnl
	AC_REQUIRE([SXE_CHECK_COMPILER_XFLAG])

	## relocation
	SXE_CHECK_LD_ZFLAG([combreloc])
	SXE_CHECK_LD_ZFLAG([nocombreloc])
	## symbols
	SXE_CHECK_LD_ZFLAG([defs])
	SXE_CHECK_LD_ZFLAG([muldefs])
	## search paths
	SXE_CHECK_LD_ZFLAG([nodefaultlib])
	## binding policy
	SXE_CHECK_LD_ZFLAG([lazy])
	SXE_CHECK_LD_ZFLAG([now])

	SXE_LD_EXPORT_DYNAMIC

	LDFLAGS="${ldflags} ${ac_cv_env_LDFLAGS_value}"
	AC_MSG_CHECKING([for preferred LDFLAGS])
	AC_MSG_RESULT([${LDFLAGS}])

	AC_MSG_NOTICE([
If you wish to ADD your own flags you want to stop here and rerun the
configure script like so:
  configure LDFLAGS=<to-be-added-flags>

You can always override the determined LDFLAGS, partially or totally,
using
  make -C <directory> LDFLAGS=<your-own-flags> [target]
or
  make LDFLAGS=<your-own-flags> [target]
respectively

NOTE: -C <directory> option is not available on all systems
		])
])dnl SXE_CHECK_LDFLAGS

AC_DEFUN([SXE_PREPEND_LINKER_FLAG], [dnl
	## a convenience function to add such linker flags to variables
	## arg1 is the flag to add (must include -z if applicable)
	## arg2 is the variable whither to prepend
	pushdef([FLAG], [$1])
	pushdef([__FLAG], [-Wl,]patsubst([$1], [ ], [[,]]))
	pushdef([VAR], [$2])

	[]VAR[]="[]__FLAG[] $[]VAR[]"
	if test "$extra_verbose" = "yes"; then
		echo "    Prepending linker flag \"[]__FLAG[]\" to \$[]VAR[]"
	fi

	popdef([VAR])
	popdef([__FLAG])
	popdef([FLAG])
])dnl SXE_PREPEND_LINKER_FLAG

dnl sxe-linker.m4 ends here
