#!/bin/sh

## usage gentexi BINARY [TOOLNAME]
BINARY="${1}"
if test -z "${2}"; then
	BINNAME=$(basename "${BINARY}")
else
	BINNAME="${2}"
fi
shift

if ! test -x "${BINARY}"; then
	echo "${BINARY} not found, generating dummy" >&2
	cat <<EOF
@node ${BINNAME}
@chapter ${BINNAME}
@cindex invoking @command{${BINNAME}}

This version of dateutils does not contain the ${BINNAME} tool.
EOF
	exit 2
fi

cat <<EOF
@node ${BINNAME}
@chapter ${BINNAME}
@cindex invoking @command{${BINNAME}}

@command{$BINNAME} may be invoked with the following command-line options:

@verbatim
$("${BINARY}" --help)
@end verbatim

EOF

if test "${#}" -eq 0; then
	exit 0
fi

echo "@section Examples"
for i; do
	echo
	echo "@example"
	sed '/^#!/d; /ends here$/d; /^[ \t]*$/d;
s/@/@@/g; s/{/@{/g; s/}/@}/g' "${i}"
	echo "@end example"
done

exit 0
