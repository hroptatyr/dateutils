#!/bin/sh

## usage gentexi BINARY
if test -x "${1}"; then
	BINARY="${1}"
	BINNAME=$(basename "${BINARY}")
else
	echo "${1} not a binary" >&2
	exit 1
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
