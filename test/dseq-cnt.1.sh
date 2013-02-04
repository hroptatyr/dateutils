#!/bin/sh

BEG="1917-01-01"
END="4095-12-31"

TOOLDIR="$(pwd)/../src"

DSEQ="${TOOLDIR}/dseq"
DDIFF="${TOOLDIR}/ddiff"

cnt_seq="`"${DSEQ}" "${BEG}" "${END}" | wc -l`" || exit 1
cnt_seq="$(( ${cnt_seq} - 1 ))" || exit 1
cnt_diff="`"${DDIFF}" "${BEG}" "${END}"`" || exit 1

if test "${cnt_seq}" != "${cnt_diff}"; then
	echo "'${cnt_seq}' != '${cnt_diff}'"
	exit 1
fi
