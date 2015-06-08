#!/bin/sh

## usage gentexi BINARY [TOOLNAME]
if test -x "${1}"; then
	BINARY="${1}"
	if test -n "${2}"; then
		BINNAME="${2}"
		shift
	else
		BINNAME=$(basename "${BINARY}")
	fi
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
