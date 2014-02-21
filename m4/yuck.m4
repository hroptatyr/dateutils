dnl yuck.m4 --- yuck goodies
dnl
dnl Copyright (C) 2013-2014 Sebastian Freundt
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

	AC_SUBST([M4])
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
])dnl AX_CHECK_M4_BUFFERS

AC_DEFUN([AX_CHECK_YUCK], [dnl
	AC_ARG_WITH([included-yuck], [dnl
AS_HELP_STRING([--with-included-yuck], [
Use included copy of the yuck command line parser generator
instead of the system-wide one.])], [with_included_yuck="${withval}"], [$1])

	if test "${with_included_yuck}" != "yes"; then
		AC_PATH_PROG([YUCK], [yuck])
		AC_ARG_VAR([YUCK], [full path to the yuck tool])
		AC_MSG_CHECKING([for yuck])
		AC_MSG_RESULT([${YUCK}])
	fi
	AM_CONDITIONAL([HAVE_YUCK], [test -n "${YUCK}"])

	AC_REQUIRE([AX_CHECK_M4_BUFFERS])
	if test -n "${YUCK}"; then
		## see what m4 they used back then
		PKG_CHECK_EXISTS([yuck >= 0.1], [dnl
			_PKG_CONFIG([yuck_m4], [variable=yuck_m4], [yuck])
		])
		M4="${pkg_cv_yuck_m4:-m4}"
	fi
])dnl AX_CHECK_YUCK

AC_DEFUN([AX_YUCK_SCMVER], [dnl
## initially generate version.mk and yuck.version here
## because only GNU make can do this at make time
	pushdef([vfile], [$1])

	AC_MSG_CHECKING([for stipulated version files])
	if test -f "${srcdir}/.version"; then
		## transform reference version
		VERSION=`"${AWK}" -F'-' 'NR == 1 {
PRE = substr([$]1, 1, 1);
VER = substr([$]1, 2);
if (PRE == "v" || PRE == "V") {
	SCM = substr([$]3, 1, 1);
	if (SCM == "g") {
		VER = VER ".git" [$]2 "." substr([$]3, 2);
	}
	if ([$]4) {
		VER = VER "." [$]4;
	}
	print VER;
}
}' "${srcdir}/.version"`
		AC_MSG_RESULT([.version -> ${VERSION}])
	else
		AC_MSG_RESULT([none])
	fi
	## also massage version.mk file
	if test -f "${srcdir}/[]vfile[]"; then
		## make sure it's in the builddir as well
		cp "${srcdir}/[]vfile[]" "[]vfile[]" 2>/dev/null
	elif test -f "${srcdir}/[]vfile[].in"; then
		${M4:-m4} -DYUCK_SCMVER_VERSION="${VERSION}" \
			"${srcdir}/[]vfile[].in" > "[]vfile[]"
	else
		echo "VERSION = ${VERSION}" > "[]vfile[]"
	fi

	popdef([vfile])
])dnl AX_YUCK_SCMVER

dnl yuck.m4 ends here
