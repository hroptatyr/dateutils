
AC_DEFUN([GIT_VERSION_MAGIC], [dnl
## initially generate version.mk here because only GNU make can do this
## at make time
AC_MSG_CHECKING([git-version-gen])
if  "${srcdir}/git-version-gen" "${srcdir}" version.mk 2>/dev/null \
    && git_version="`head -n 1 version.mk`" ;then
  AC_MSG_RESULT([${git_version}])
else
  AC_MSG_FAILURE([failed])
fi

dnl remove version info from config header
AH_BOTTOM([
#/**/undef/**/ VERSION
#/**/undef/**/ PACKAGE_VERSION
#/**/undef/**/ PACKAGE_STRING
])
])
