AC_DEFUN([SXE_CHECK_SYS_MMAN], [dnl
	AC_CHECK_HEADERS([sys/mman.h])
])dnl SXE_CHECK_SYS_MMAN

AC_DEFUN([SXE_CHECK_MAP_ANON], [dnl
	AC_MSG_CHECKING([for ANON maps])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 600
#define _BSD_SOURCE
#if defined HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif  /* HAVE_SYS_MMAN_H */
#if defined MAP_ANON
/* good */
#elif defined MAP_ANONYMOUS
/* good too */
#else
# error MAP_ANON | MAP_ANONYMOUS needed
#endif
	]])], [sxe_cv_feat_anon_maps="yes"], [sxe_cv_feat_anon_maps="no"])
	if test "${sxe_cv_feat_anon_maps}" = "no"; then
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#define _POSIX_C_SOURCE 200112L
#define _BSD_SOURCE
#define _XOPEN_SOURCE 600
#define _ALL_SOURCE
#if defined HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif  /* HAVE_SYS_MMAN_H */
#if defined MAP_ANON
/* good */
#elif defined MAP_ANONYMOUS
/* good too */
#else
# error MAP_ANON | MAP_ANONYMOUS needed
#endif
		]])], [
			sxe_cv_feat_anon_maps="yes"
			AC_DEFINE([MAP_ANON_NEEDS_ALL_SOURCE], [1], 
				[MAP_ANON with _ALL_SOURCE])
		], [
			sxe_cv_feat_anon_maps="no"
		])
	fi
	if test "${sxe_cv_feat_anon_maps}" = "no"; then
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#define _POSIX_C_SOURCE 200112L
#define _BSD_SOURCE
#define _XOPEN_SOURCE 600
#define _DARWIN_C_SOURCE
#if defined HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif  /* HAVE_SYS_MMAN_H */
#if defined MAP_ANON
/* good */
#elif defined MAP_ANONYMOUS
/* good too */
#else
# error MAP_ANON | MAP_ANONYMOUS needed
#endif
		]])], [
			sxe_cv_feat_anon_maps="yes"
			AC_DEFINE([MAP_ANON_NEEDS_DARWIN_SOURCE], [1], 
				[MAP_ANON with _DARWIN_C_SOURCE])
		], [
			sxe_cv_feat_anon_maps="no"
		])
	fi
	AC_MSG_RESULT([${sxe_cv_feat_anon_maps}])
])dnl SXE_CHECK_MAP_ANON

AC_DEFUN([SXE_CHECK_MMAP], [dnl
	AC_REQUIRE([SXE_CHECK_SYS_MMAN])
	AC_REQUIRE([SXE_CHECK_MAP_ANON])
])dnl SXE_CHECK_MMAP
