#!/bin/sh

BEG="1917"
END="4095"
if test "${have_gdate_2039}" != "yes"; then
	BEG="1971"
	END="2038"
fi

if test "${have_gdate}" != "yes"; then
	## SKIP in new automake
	exit 77
fi

TOOLDIR="$(pwd)/../src"

DCONV="${TOOLDIR}/dconv"

foo=`mktemp "/tmp/tmp.XXXXXXXXXX"`
bar=`mktemp "/tmp/tmp.XXXXXXXXXX"`

for y in `seq ${BEG} ${END}`; do
	"${DCONV}" "${y}-01-01" -f '%F	%a'
done > "${foo}"
for y in `seq ${BEG} ${END}`; do
	TZ=UTC LANG=C LC_ALL=C "${GDATE}" -d "${y}-01-01" '+%F	%a'
done > "${bar}"

diff "${foo}" "${bar}"
rc=${?}

rm -f "${foo}" "${bar}"

exit ${rc}
