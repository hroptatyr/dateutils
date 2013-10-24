#!/bin/sh

sed '
/^@section/ {
	s/@section /\[/
	s/$/\]/
}
/^@verbatim/d
/^@end verbatim/d
s/@samp{\([^}]*\)}/"\1"/g
' "${1}"
