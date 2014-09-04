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
	AC_SUBST([MATLABPATH])
	AC_MSG_RESULT([${MATLABPATH}])

	AC_MSG_CHECKING([for matlab mex file extension])
	sxe_cv_mexext=`"${MATLABROOT}/bin/mexext" 2>/dev/null`
	MEXEXT="${sxe_cv_mexext:-mex}"
	AC_SUBST([MEXEXT])
	AC_MSG_RESULT([${sxe_cv_mexext:-mex (assumed)}])

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
		CPPFLAGS="${CPPFLAGS} ${matlab_CFLAGS}"
		AC_CHECK_HEADER([mex.h])
		sxe_cv_matlab_mex_h="${ac_cv_header_mex_h}"
		unset ac_cv_header_mex_h
		CPPFLAGS="${save_CPPFLAGS}"
	fi

	rm -f -- "${foo}"
])dnl SXE_CHECK_MATLAB

AC_DEFUN([SXE_CHECK_OCTAVE], [dnl
	## prep the octave extension path, this is twofold
	AC_PATH_PROG([OCTAVE_CONFIG], [octave-config])
	if test -n "${OCTAVE_CONFIG}"; then
		octave_CFLAGS=-I`"${OCTAVE_CONFIG}" -p OCTINCLUDEDIR`
		octave_LIBS=-L`"${OCTAVE_CONFIG}" -p OCTLIBDIR`
		AC_MSG_CHECKING([for octave toolbox path])
		OCTAVEPATH=`"${OCTAVE_CONFIG}" -p LOCALOCTFILEDIR`
		AC_SUBST([OCTAVEPATH])
		AC_MSG_RESULT([${OCTAVEPATH}])
	fi

	save_CPPFLAGS="${CPPFLAGS}"
	CPPFLAGS="${CPPFLAGS} ${octave_CFLAGS}"
	AC_CHECK_HEADERS([mex.h])
	AC_CHECK_HEADERS([octave/mex.h])
	if test "${ac_cv_header_mex_h}" = "yes"; then
		sxe_cv_octave_mex_h="yes"
	elif test "${ac_cv_header_octave_mex_h}" = "yes"; then
		sxe_cv_octave_mex_h="yes"
	fi
	unset ac_cv_header_mex_h
	CPPFLAGS="${save_CPPFLAGS}"

	if test "${sxe_cv_octave_mex_h}" = "yes"; then
		have_octave="yes"
	else
		have_octave="no"
	fi
])dnl SXE_CHECK_OCTAVE

dnl sxe-matlab.m4 ends here
