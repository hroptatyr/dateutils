#!/bin/sh

## usage gentexi BINARY
if test -x "${1}"; then
	BINARY="${1}"
	BINNAME=$(basename "${BINARY}")
	shift
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

if test "${#}" -eq 0; then
	exit 0
fi

## now go for examples
myexit()
{
	rm -f -- "${stdin}" "${stdout}" "${stderr}" "${tool_stdout}" "${tool_stderr}"
	exit ${1:-1}
}

. $(dirname "${0}")"/genex"

echo "@section Examples"
for i; do
	. "${i}"

	## double check we're in the right tool
	if test "${TOOL}" != "${BINNAME}"; then
		continue
	fi

	echo
	echo "@example"
	genex "${BINARY}" "${BINNAME}" "${CMDLINE}" "${stdin}"
	echo "@end example"
done

myexit 0
