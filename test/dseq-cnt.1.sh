#!/bin/sh

BEG="1917-01-01"
END="4095-12-31"

foo=`mktemp "/tmp/tmp.XXXXXXXXXX"`
bar=`mktemp "/tmp/tmp.XXXXXXXXXX"`

dseq "${BEG}" "${END}" | head -n-1 | wc -l > "${foo}"
ddiff "${BEG}" "${END}" > "${bar}"

diff "${foo}" "${bar}"
rc=${?}

rm -f "${foo}" "${bar}"

exit ${rc}
