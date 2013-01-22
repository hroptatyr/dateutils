## first parameter may point to a matlab root or the matlab binary
AC_DEFUN([SXE_CHECK_MATLAB], [dnl
	pushdef([mroot], [$1])
	foo=$(mktemp)

	SXE_MSG_CHECKING([for matlab root])

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
	fi 2>/dev/null | sed "/^MATLAB=/!d; s/^MATLAB=//" > "${foo}"

	read line <<< $(head -n1 "${foo}")
	if test -x "${line}"; then
		sxe_cv_matlabroot="${line}"
		MATLABROOT="${sxe_cv_matlabroot}"
		AC_SUBST([MATLABROOT])
	else
		sxe_cv_matlabroot="no"
	fi

	SXE_MSG_RESULT([${sxe_cv_matlabroot}])

	rm -f -- "${foo}"
	popdef([mroot])
])dnl SXE_CHECK_MATLAB

dnl sxe-matlab.m4 ends here
