## first parameter may point to a matlab root or the matlab binary
AC_DEFUN([SXE_CHECK_MATLAB], [dnl
	foo=`mktemp`

	AC_ARG_VAR([MATLAB], [full path to matlab binary])
	sxe_cv_matlab="${MATLAB:-matlab}"

	AC_MSG_CHECKING([for matlab root])
	## assume no matlab
	sxe_cv_matlabpath="no"
	sxe_cv_matlabroot="no"

	"${sxe_cv_matlab}" -e 2>/dev/null | grep "MATLAB" > "${foo}"

	## source that
	source "${foo}"

	MATLABROOT="${MATLAB}"
	AC_SUBST([MATLABROOT])
	AC_MSG_RESULT([${MATLABROOT}])

	AC_MSG_CHECKING([for matlab toolbox path])
	AC_MSG_RESULT([${MATLABPATH}])

	## now reset *our* idea of what MATLAB should be
	MATLAB="${sxe_cv_matlab}"

	AC_ARG_VAR([matlab_CFLAGS], [include directives for matlab headers])

	if test -n "${matlab_CFLAGS}"; then
		:
	elif test -z "${MATLABROOT}"; then
		## big cluster fuck
		:
	else
		matlab_CFLAGS="-I${MATLABROOT}/extern/include"
	fi
	if test -n "${matlab_CFLAGS}"; then
		save_CPPFLAGS="${CPPFLAGS}"
		CPPFLAGS="${CPPFLAGS} -I${matlab_CFLAGS}"
		AC_CHECK_HEADERS([mex.h])
		CPPFLAGS="${save_CPPFLAGS}"
	fi

	rm -f -- "${foo}"
])dnl SXE_CHECK_MATLAB

dnl sxe-matlab.m4 ends here
