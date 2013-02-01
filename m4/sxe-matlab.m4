## first parameter may point to a matlab root or the matlab binary
AC_DEFUN([SXE_CHECK_MATLAB], [dnl
	pushdef([mroot], [$1])
	foo=`mktemp`

	AC_MSG_CHECKING([for matlab root])

	## assume no matlab
	sxe_cv_matlabroot="no"

	if test "[]mroot[]" = "yes"; then
		matlab -e
	elif readlink -e "[]mroot[]" >/dev/null; then
		if test -d "[]mroot[]"; then
			"[]mroot[]/bin/matlab" -e
		elif test -x "[]mroot[]"; then
			"[]mroot[]" -e
		fi
	elif command -v "[]mroot[]" >/dev/null; then
		"[]mroot[]" -e
	else
		matlab -e
	fi 2>/dev/null | grep "MATLAB" > "${foo}"

	## source that
	source "${foo}"

	if test -x "${MATLAB}"; then
		sxe_cv_matlabroot="${MATLAB}"
		MATLABROOT="${sxe_cv_matlabroot}"
		AC_SUBST([MATLABROOT])
	fi

	AC_MSG_RESULT([${sxe_cv_matlabroot}])

	save_CPPFLAGS="${CPPFLAGS}"
	CPPFLAGS="${CPPFLAGS} -I${MATLABROOT}/extern/include"
	AC_CHECK_HEADERS([mex.h])
	if test "${ac_cv_header_mex_h}"; then
		matlab_CFLAGS="-I${MATLABROOT}/extern/include"
		AC_SUBST([matlab_CFLAGS])
	fi
	CPPFLAGS="${save_CPPFLAGS}"

	rm -f -- "${foo}"
	popdef([mroot])
])dnl SXE_CHECK_MATLAB

dnl sxe-matlab.m4 ends here
