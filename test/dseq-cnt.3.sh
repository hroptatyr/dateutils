#!/bin/sh

BEG="1917-12-31"
if test "${have_gdate_2039}" = "yes"; then
	END="4095-01-01"
else
	END="2038-01-01"
fi

if test "${have_gdate}" != "yes"; then
	## SKIP in new automake
	exit 77
fi

TOOLDIR="$(pwd)/../src"

DSEQ="${TOOLDIR}/dseq"
GDATE="date"

foo=`mktemp "/tmp/tmp.XXXXXXXXXX"`
bar=`mktemp "/tmp/tmp.XXXXXXXXXX"`

"${DSEQ}" "${BEG}" +1y "${END}" -f '%F	%a' > "${foo}"
for y in `seq ${BEG/*-/} ${END/*-/}`; do
	"${GDATE}" -d "${y}-12-31" '+%F	%a'
done > "${bar}"

diff "${foo}" "${bar}"
rc=${?}

rm -f "${foo}" "${bar}"

exit ${rc}
