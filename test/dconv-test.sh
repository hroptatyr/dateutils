#!/bin/bash

## should be called by ut-test
if test -z "${testfile}"; then
	exit 1
fi

xrealpath()
{
	readlink -f "${1}" 2>/dev/null || \
	realpath "${1}" 2>/dev/null || \
	(
		cd "`dirname "${1}"`" || exit 1
		tmp_target="`basename "${1}"`"
		# Iterate down a (possible) chain of symlinks
		while test -L "${tmp_target}"; do
			tmp_target="`readlink "${tmp_target}"`"
			cd "`dirname "${tmp_target}"`" || exit 1
			tmp_target="`basename "${tmp_target}"`"
		done
		echo "`pwd -P || pwd`/${tmp_target}"
	) 2>/dev/null
}

## setup
fail=0

## source the check
. "${testfile}" || fail=1
orig=`mktemp "/tmp/tmp.XXXXXXXXXX"`
conv=`mktemp "/tmp/tmp.XXXXXXXXXX"`

myexit()
{
	rm -f -- "${orig}" "${conv}"
	## maybe there's profiling info
	if test -r "gmon.out"; then
		runnm="gmon-`basename "${testfile}"`.${$}.out"
		mv "gmon.out" "${runnm}"
	fi
	exit ${1:-1}
}

find_file()
{
	file="${1}"

	if test -z "${file}"; then
		:
	elif test -r "${file}"; then
		echo "${file}"
	elif test -r "${builddir}/${file}"; then
		xrealpath "${builddir}/${file}"
	elif test -r "${srcdir}/${file}"; then
		xrealpath "${srcdir}/${file}"
	fi
}

eval_echo()
{
	local ret

	echo ${@} >&3
	eval ${@}
	ret=${?}
	return ${ret}
}

## set finals
if test -x "${builddir}/dseq"; then
	DSEQ="`xrealpath "${builddir}/dseq"`"
fi
if test -x "${builddir}/dconv"; then
	DCONV="`xrealpath "${builddir}/dconv"`"
fi
if test -z "${srcdir}"; then
	srcdir="`xrealpath "\`dirname "${0}"\`"`"
else
	srcdir="`xrealpath "${srcdir}"`"
fi

eval_echo "${HUSK}" "\"${DSEQ}\" \"${BEG}\" \"${END}\" -f \"${SRC}\"" \
	3>&2 \
	> "${orig}" || fail=${?}

eval_echo "${HUSK}" "\"${DCONV}\" -f \"${TGT}\" | \"${DCONV}\" -f \"${SRC}\"" \
	< "${orig}" \
	3>&2 \
	> "${conv}" || fail=${?}

echo

diff -u "${orig}" "${conv}" || fail=1

myexit ${fail}

## dconv-test.sh ends here
