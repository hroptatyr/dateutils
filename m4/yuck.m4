dnl yuck.m4 --- yuck goodies
dnl
dnl Copyright (C) 2013 Sebastian Freundt
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

AC_DEFUN([AX_CHECK_YUCK], [dnl
	AC_ARG_WITH([included-yuck], [dnl
AS_HELP_STRING([--with-included-yuck], [
Use included copy of the yuck command line parser generator
instead of the system-wide one.])], [with_included_yuck="${withval}"], [$1])

	if test "${with_included_yuck}" != "yes"; then
		PKG_CHECK_EXISTS([yuck >= 0.0], [have_yuck="yes"], [have_yuck="no"])
		AC_MSG_CHECKING([for yuck])
		AC_MSG_RESULT([${have_yuck}])
	fi
	AM_CONDITIONAL([HAVE_YUCK], [test "${have_yuck}" = "yes"])
])dnl AX_CHECK_YUCK

dnl yuck.m4 ends here
