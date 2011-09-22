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

echo "[EXAMPLES]"
for i; do
	. "${i}"

	## double check we're in the right tool
	if test "${TOOL}" != "${BINNAME}"; then
		continue
	fi

	echo
	genex "${BINARY}" "${BINNAME}" "${CMDLINE}" "${stdin}" | \
		sed 's/^/  /'
	echo
done

myexit 0
