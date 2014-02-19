#!/bin/zsh

## This script turns geonames data into a map IATA->ZONENAME
usage()
{
	cat <<'EOF'
Usage: snarf-iata.sh --icao|--iata [COUNTRIES-FILE] [ALTERNAMES-FILE]
EOF
}

case "${1}" in
("--icao")
	code="icao"
	shift
	;;
("--iata")
	code="iata"
	shift
	;;
(*)
	usage >&2
	exit 1
	;;
esac

if test -r "${1}"; then
	ALLCN="${1}"
else
	ALLCN_needs_downloading="yes"
	needs_downloading="yes"
fi

if test -r "${2}"; then
	ALTNM="${2}"
else
	ALTNM_needs_downloading="yes"
	needs_downloading="yes"
fi

if test "${needs_downloading}" = "yes"; then
	echo "downloading raw data ..." >&2
	if test "${ALLCN_needs_downloading}" = "yes"; then
		curl -qgsLO "http://download.geonames.org/export/dump/allCountries.zip"
		ALLCN="allCountries.zip"
	fi
	if test "${ALTNM_needs_downloading}" = "yes"; then
		curl -qgsLO "http://download.geonames.org/export/dump/alternateNames.zip"
		ALTNM="alternateNames.zip"
	fi
fi

join -t'	' \
	<(bsdtar Oxf "${ALTNM}" alternateNames.txt | \
		grep -F "	${code}	" | cut -f 2,4 | sort) \
	<(bsdtar Oxf "${ALLCN}" allCountries.txt | \
		grep -F '	S	AIR' | cut -f 1,18 | sort) \
	| cut -f2- | sort

