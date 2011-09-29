#!/bin/sh

sed '
/^@section/ {s/@section /[/; s/$/]/}
/^@verbatim/d
/^@end verbatim/d
' "${1}"
