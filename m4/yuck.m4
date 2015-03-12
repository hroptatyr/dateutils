dnl yuck.m4 --- yuck goodies
dnl
dnl Copyright (C) 2013-2015 Sebastian Freundt
dnl
dnl Author: Sebastian Freundt <hroptatyr@fresse.org>
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
dnl This file is part of yuck.

AC_DEFUN([AX_CHECK_M4_BUFFERS], [dnl
	AC_MSG_CHECKING([for m4 with sufficient capabilities])

	AC_ARG_VAR([M4], [full path to the m4 tool])
	probe_M4="${M4:-m4}"
	if ${probe_M4} >/dev/null 2>&1 \
		-Dx='y y y y y y y y y y y y y y y y' \
		-Dy='z z z z z z z z z z z z z z z z' \
                -Dz='0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0' <<'EOF'
[define(`foo', x)]
EOF
	then
		## ah well done
		AC_MSG_RESULT([${probe_M4}])
		M4="${probe_M4}"
	else
		## check if a little buffer massage solves the problem
		probe_M4="${M4:-m4} -B16384"
		if ${probe_M4} >/dev/null 2>&1 \
			-Dx='y y y y y y y y y y y y y y y y' \
			-Dy='z z z z z z z z z z z z z z z z' \
			-Dz='0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0' <<'EOF'
[define(`foo', x)]
EOF
		then
			## very well then, let's use -B
			AC_MSG_RESULT([${probe_M4}])
			M4="${probe_M4}"
		else
			AC_MSG_WARN([m4 on this machine might suffer from big buffers.])
			M4="${M4:-m4}"
		fi
	fi

	AC_DEFINE_UNQUOTED([YUCK_M4], ["${M4}"], [m4 value used for yuck build])
])dnl AX_CHECK_M4_BUFFERS

AC_DEFUN([AX_CHECK_YUCK], [dnl
	AC_ARG_WITH([included-yuck], [dnl
AS_HELP_STRING([--with-included-yuck], [
Use included copy of the yuck command line parser generator
instead of the system-wide one.])], [with_included_yuck="${withval}"], [$1])

	AC_REQUIRE([AX_CHECK_M4_BUFFERS])
	if test "${with_included_yuck}" != "yes"; then
		AC_PATH_PROG([YUCK], [yuck])
		AC_ARG_VAR([YUCK], [full path to the yuck tool])

		if test -n "${YUCK}"; then
			## see what m4 they used back then
			YUCK_M4=`${YUCK} config --m4 2>/dev/null`
			M4="${YUCK_M4:-${M4}}"
		fi
	fi
	AM_CONDITIONAL([HAVE_YUCK], [dnl
		test "${with_included_yuck}" != "yes" -a -n "${YUCK}"])

	## further requirement is either getline() or fgetln()
	AC_CHECK_FUNCS([getline])
	AC_CHECK_FUNCS([fgetln])
])dnl AX_CHECK_YUCK

AC_DEFUN([AX_YUCK_SCMVER], [dnl
## initially generate version.mk and yuck.version here
## because only GNU make can do this at make time
	pushdef([vfile], [$1])

	AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT])
	AC_LANG_PUSH([C])
	AC_PROG_CC_C99
	## use our yuck-scmver tool
	AC_MSG_CHECKING([for stipulated version files])
	save_CPPFLAGS="${CPPFLAGS}"
	CPPFLAGS="-I${srcdir}/src -I${ac_aux_dir} ${CPPFLAGS}"
	AC_RUN_IFELSE([AC_LANG_SOURCE([[
#define CONFIGURE
#define _XOPEN_SOURCE	600
#define VERSION_FILE	"${srcdir}/.version"
#include "yuck-scmver.c"
]])], [STIP_VERSION=`./conftest$EXEEXT`], [AC_MSG_RESULT([none])], [dnl
		AC_MSG_RESULT([impossible, cross-compiling])
		if test -f "[]vfile[]" -o \
			-f "${srcdir}/[]vfile[]" -o \
			-f "${srcdir}/.version"; then
			AC_MSG_NOTICE([
Files that (possibly) mandate versions have been detected.
These are `]vfile[' or `${srcdir}/]vfile[' or `${srcdir}/.version'.
However, their contents cannot be automatically checked for integrity
due to building for a platform other than the current one
(cross-compiling).

I will proceed with the most conservative guess for the stipulated
version, which is `${VERSION}'.

If that appears to be wrong, or needs overriding, please edit the
aforementioned files manually.

Also note, even though this project comes with all the tools to
perform a successful bootstrap for any of the files above, should
they go out of date or be deleted, they don't support cross-builds.
			])
		fi
	])
	CPPFLAGS="${save_CPPFLAGS}"
	AC_LANG_POP([C])

	if test -n "${STIP_VERSION}"; then
		VERSION="${STIP_VERSION}"
	fi
	## also massage version.mk file
	if test -f "[]vfile[]" -a ! -w "[]vfile[]"; then
		:
	elif test -f "${srcdir}/[]vfile[]"; then
		## make sure it's in the builddir as well
		cp -p "${srcdir}/[]vfile[]" "[]vfile[]" 2>/dev/null
	elif test -f "${srcdir}/[]vfile[].in"; then
		${M4:-m4} -DYUCK_SCMVER_VERSION="${VERSION}" \
			"${srcdir}/[]vfile[].in" > "[]vfile[]"
	else
		echo "VERSION = ${VERSION}" > "[]vfile[]"
	fi
	## make sure .version is generated (for version.mk target in GNUmakefile)
	if test -f "${srcdir}/.version"; then
		cp -p "${srcdir}/.version" ".version" 2>/dev/null
	else
		echo "v${VERSION}" > ".version" 2>/dev/null
	fi

	popdef([vfile])
])dnl AX_YUCK_SCMVER

dnl yuck.m4 ends here
