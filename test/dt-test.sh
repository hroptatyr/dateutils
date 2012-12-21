#!/bin/bash

## should be called by ut-test
if test -z "${testfile}"; then
	exit 1
fi

## helper funs that might be of use in the test files
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
tool_stdout=`mktemp "/tmp/tmp.XXXXXXXXXX"`
tool_stderr=`mktemp "/tmp/tmp.XXXXXXXXXX"`

## source the check
. "${testfile}" || fail=1

rm_if_not_src()
{
	file="${1}"
	srcd="${2:-${srcdir}}"
	dirf="`dirname "${file}"`"

	if test "${dirf}" -ef "${srcd}"; then
		## treat as precious source file
		:
	elif test "`pwd -P || pwd`" -ef "${srcd}"; then
		## treat as precious source file
		:
	else
		rm -vf -- "${file}"
	fi
}

myexit()
{
	rm_if_not_src "${stdin}" "${srcdir}"
	rm_if_not_src "${stdout}" "${srcdir}"
	rm_if_not_src "${stderr}" "${srcdir}"
	rm -f -- "${tool_stdout}" "${tool_stderr}"
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
	local tmpf

	echo -n ${@} >&3
	if test "/dev/stdin" -ef "/dev/null"; then
		echo >&3
	else
		echo "<<EOF" >&3
		tmpf=`mktemp "/tmp/tmp.XXXXXXXXXX"`
		tee "${tmpf}" >&3
		echo "EOF" >&3
	fi

	eval ${@} < "${tmpf:-/dev/null}"
	ret=${?}
	rm -f -- "${tmpf}"
	return ${ret}
}

## check if everything's set
if test -z "${TOOL}"; then
	echo "variable \${TOOL} not set" >&2
	myexit 1
fi

## set finals
if test -x "${builddir}/${TOOL}"; then
	TOOL="`xrealpath "${builddir}/${TOOL}"`"
fi
if test -z "${srcdir}"; then
	srcdir="`xrealpath "\`dirname "${0}"\`"`"
else
	srcdir="`xrealpath "${srcdir}"`"
fi

stdin="`find_file "${stdin}"`"
stdout="`find_file "${stdout}"`"
stderr="`find_file "${stderr}"`"

eval_echo "${husk}" "${TOOL}" "${CMDLINE}" \
	< "${stdin:-/dev/null}" \
	3>&2 \
	> "${tool_stdout}" 2> "${tool_stderr}" || fail=${?}

echo
if test "${EXPECT_EXIT_CODE}" = "${fail}"; then
	fail=0
fi

if test -r "${stdout}"; then
	diff -u "${stdout}" "${tool_stdout}" || fail=1
elif test -s "${tool_stdout}"; then
	echo
	echo "test stdout was:"
	cat "${tool_stdout}" >&2
	echo
fi
if test -r "${stderr}"; then
	diff -u "${stderr}" "${tool_stderr}" || fail=1
elif test -s "${tool_stderr}"; then
	echo
	echo "test stderr was:"
	cat "${tool_stderr}" >&2
	echo
fi

myexit ${fail}

## dt-test.sh ends here
