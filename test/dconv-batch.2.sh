#!/bin/sh

BEG="1917"
END="4095"

TOOLDIR="$(pwd)/../src"

DCONV="${TOOLDIR}/dconv"
GDATE="date"

foo=`mktemp "/tmp/tmp.XXXXXXXXXX"`
bar=`mktemp "/tmp/tmp.XXXXXXXXXX"`

for y in `seq ${BEG} ${END}`; do
	"${DCONV}" "${y}-12-31" -f '%F	%a'
done > "${foo}"
for y in `seq ${BEG} ${END}`; do
	"${GDATE}" -d "${y}-12-31" '+%F	%a'
done > "${bar}"

diff "${foo}" "${bar}"
rc=${?}

rm -f "${foo}" "${bar}"

exit ${rc}
