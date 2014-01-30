changequote([,])dnl
divert([-1])

## little DSL for yuck-version
define([YUCK_SCMVER_VERSION], [dnl
YUCK_SCMVER_VTAG[]dnl
ifelse(defn([YUCK_SCMVER_DIST]), [0], [], [dnl
[.]YUCK_SCMVER_SCM[]YUCK_SCMVER_DIST[.]YUCK_SCMVER_RVSN[]dnl
ifdef([YUCK_SCMVER_FLAG_DIRTY], [.dirty])[]dnl
])[]dnl
])

divert[]dnl
