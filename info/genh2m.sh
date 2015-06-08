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

echo "[EXAMPLES]"
if test "${#}" -eq 0; then
	exit 0
fi

for i; do
	echo
	sed '/^#!/d; /ends here$/d; /^[ \t]*$/d;
s/@/@@/g; s/{/@{/g; s/}/@}/g
s/\\/\\\\/g
s/^/  /' "${i}"
	echo
done

exit 0
