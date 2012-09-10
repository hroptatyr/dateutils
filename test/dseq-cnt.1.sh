#!/bin/sh

BEG="1917-01-01"
END="4095-12-31"

TOOLDIR="$(pwd)/../src"

DSEQ="${TOOLDIR}/dseq"
DDIFF="${TOOLDIR}/ddiff"

foo=`mktemp "/tmp/tmp.XXXXXXXXXX"`
bar=`mktemp "/tmp/tmp.XXXXXXXXXX"`

"${DSEQ}" "${BEG}" "${END}" | head -n-1 | wc -l > "${foo}"
"${DDIFF}" "${BEG}" "${END}" > "${bar}"

diff "${foo}" "${bar}"
rc=${?}

rm -f "${foo}" "${bar}"

exit ${rc}
