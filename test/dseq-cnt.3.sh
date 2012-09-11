#!/bin/sh

BEG="1917-12-31"
END="4095-12-31"

TOOLDIR="$(pwd)/../src"

DSEQ="${TOOLDIR}/dseq"
GDATE="date"

foo=`mktemp "/tmp/tmp.XXXXXXXXXX"`
bar=`mktemp "/tmp/tmp.XXXXXXXXXX"`

"${DSEQ}" "${BEG}" +1y "${END}" -f '%F	%a' > "${foo}"
for y in `seq 1917 4095`; do
	"${GDATE}" -d "${y}-12-31" '+%F	%a'
done > "${bar}"

diff "${foo}" "${bar}"
rc=${?}

rm -f "${foo}" "${bar}"

exit ${rc}
