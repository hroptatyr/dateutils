#!/bin/sh

BEG="1917-01-01"
END="4095-01-01"

TOOLDIR="$(pwd)/../src"

DSEQ="${TOOLDIR}/dseq"
GDATE="date"

foo=`mktemp "/tmp/tmp.XXXXXXXXXX"`
bar=`mktemp "/tmp/tmp.XXXXXXXXXX"`

"${DSEQ}" "${BEG}" +1y "${END}" -f '%F	%a' > "${foo}"
for y in `seq 1917 4095`; do
	"${GDATE}" -d "${y}-01-01" '+%F	%a'
done > "${bar}"

diff "${foo}" "${bar}"
rc=${?}

rm -f "${foo}" "${bar}"

exit ${rc}
