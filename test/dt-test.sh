#!/bin/sh

## should be called by ut-test
if test -z "${testfile}"; then
	echo "`basename ${0}`: no test file given" >&2
	exit 1
fi

## some helper funs
xrealpath()
{
	readlink -f "${1}" 2>/dev/null || \
	realpath "${1}" 2>/dev/null || \
	( cd "`dirname "${1}"`" || exit 1
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

ts_sha1sum()
{
	local file="${1}"
	local tmp

	if ! test -r "${file}"; then
		echo "ts_sha1sum: could not read file '${file}'" >&2
		return 1
	fi

	if tmp="`sha1sum "${file}" 2>/dev/null`"; then
		echo "${tmp}" | (read sum rest; echo "${sum}")
	elif tmp="`sha1 -n "${file}" 2>/dev/null`"; then
		echo "${tmp}" | (read sum rest; echo "${sum}")
	elif tmp="`sha1 -q "${file}" 2>/dev/null`"; then
		echo "${tmp}"
	else
		echo "ts_sha1sum: unable to calculate sha1sums" >&2
		return 1
	fi
	return 0
}

tsp_create_env()
{
	TS_TMPDIR="`basename "${testfile}"`.tmpd"
	rm -rf "${TS_TMPDIR}" || return 1
	mkdir "${TS_TMPDIR}" || return 1

	TS_STDIN="${TS_TMPDIR}/stdin"
	TS_EXP_STDOUT="${TS_TMPDIR}/exp_stdout"
	TS_EXP_STDERR="${TS_TMPDIR}/exp_stderr"
	TS_OUTFILE="${TS_TMPDIR}/tool_outfile"
	TS_EXP_EXIT_CODE="0"
	TS_DIFF_OPTS=""

	tool_stdout="${TS_TMPDIR}/tool_stdout"
	tool_stderr="${TS_TMPDIR}/tool_sterr"
}

myexit()
{
	if test "${1}" = "0"; then
		rm -rf "${TS_TMPDIR}"
	fi
	exit ${1:-1}
}

## setup
fail=0
tsp_create_env || myexit 1

## also set srcdir in case the testfile needs it
if test -z "${srcdir}"; then
	srcdir=`xrealpath \`dirname "${0}"\``
else
	srcdir=`xrealpath "${srcdir}"`
fi

## source the check
. "${testfile}" || myexit 1

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

exec_echo()
{
	local ret
	local husk="${1}"
	shift

	## repeat stdin to &3
	echo "TOOL=${TOOL}" | cat - "${@}" >&3
	echo "TOOL=${TOOL}" | cat - "${@}" | "/bin/sh"
	ret=${?}
	return ${ret}
}

hexdiff()
{
	local file1="${1}"
	local file2="${2}"
	local tmp1=`mktemp "/tmp/tmp.XXXXXXXX"`
	local tmp2=`mktemp "/tmp/tmp.XXXXXXXX"`

	hextool()
	{
		local file="${1}"
		command -p "hexdump" -C "${file}" || \
			command -p "xxd" "${file}" || \
			command -p "od" -A x -v -t x2 "${file}"
	}

	hextool "${file1}" > "${tmp1}"
	hextool "${file2}" > "${tmp2}"
	diff -u "${tmp1}" "${tmp2}"

	rm -f "${tmp1}" "${tmp2}"
}

## check if everything's set
if test -z "${TOOL}"; then
	echo "variable \${TOOL} not set" >&2
	myexit 1
fi

## set finals
if test -x "${builddir}/${TOOL}"; then
	TOOL=`xrealpath "${builddir}/${TOOL}"`
fi

stdin=""
if test -r "${TS_STDIN}"; then
	stdin="${TS_STDIN}"
fi
stdout="${TS_EXP_STDOUT}"
stderr="${TS_EXP_STDERR}"


## check if we used a CMDFILE instead of CMDLINE
if test -n "${CMDFILE}"; then
	exec_echo "${husk}" "${CMDFILE}" 3>&2 \
		> "${tool_stdout}" 2> "${tool_stderr}"
	tool_exit_code=${?}
else
	eval_echo "${husk}" "${TOOL}" "${CMDLINE}" \
		< "${stdin:-/dev/null}" \
		3>&2 \
		> "${tool_stdout}" 2> "${tool_stderr}"
	tool_exit_code=${?}
fi

echo
if test "${TS_EXP_EXIT_CODE}" != "${tool_exit_code}"; then
	echo "test exit code was ${tool_exit_code} (expected: ${TS_EXP_EXIT_CODE})"
	fail=1
	echo
fi

if test -r "${stdout}"; then
	eval diff -u "${TS_DIFF_OPTS}" "${stdout}" "${tool_stdout}" || fail=1
elif test -s "${tool_stdout}"; then
	echo
	echo "test stdout was:"
	cat "${tool_stdout}" >&2
	echo
fi
if test -r "${stderr}"; then
	eval diff -u "${TS_DIFF_OPTS}" "${stderr}" "${tool_stderr}" || fail=1
elif test -s "${tool_stderr}"; then
	echo
	echo "test stderr was:"
	cat "${tool_stderr}" >&2
	echo
fi

## check if we need to hash stuff
if test -n "${REFFILE}"; then
	## check for differing files
	if ! diff -q "${REFFILE}" "${TS_OUTFILE}"; then
		## failed due to diff -q returning non-nil
		fail=1
		hexdiff "${REFFILE}" "${TS_OUTFILE}"
	fi
elif test -n "${TS_OUTFILE_SHA1}"; then
	if sum="`ts_sha1sum "${TS_OUTFILE}"`"; then
		fail=1
		for i in ${TS_OUTFILE_SHA1}; do
			if test "${sum}" = "${i}"; then
				fail=0
			fi
		done
		test "${fail}" = "1" && cat <<EOF >&2
outfile (${TS_OUTFILE}) hashes do not match:
SHOULD BE: ${TS_OUTFILE_SHA1}
ACTUAL:    ${sum}
EOF
	else
		fail=1
	fi
fi

myexit ${fail}

## dt-test.sh ends here
