/* -*- c -*- */
changequote`'changequote([,])dnl
changecom([#])dnl
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#if defined HAVE_VERSION_H
# include "version.h"
#endif	/* HAVE_VERSION_H */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
changecom([])dnl
ifdef([YUCK_HEADER], [dnl
#include "YUCK_HEADER"
])dnl
changecom([#])dnl

#if defined __INTEL_COMPILER
# pragma warning (disable:177)
# pragma warning (disable:111)
#elif defined __GNUC__
# if __GNUC__ > 4 || __GNUC__ == 4 &&  __GNUC_MINOR__ >= 6
#  pragma GCC diagnostic push
# endif	 /* GCC version */
# pragma GCC diagnostic ignored "-Wunused-label"
# pragma GCC diagnostic ignored "-Wunused-variable"
# pragma GCC diagnostic ignored "-Wunused-function"
#endif	/* __INTEL_COMPILER */
pushdef([DEFUN], ifdef([YUCK_HEADER], [], [static]))[]dnl


static inline bool
yuck_streqp(const char *s1, const char *s2)
{
	return !strcmp(s1, s2);
}

/* for multi-args */
static inline char**
yuck_append(char **array, size_t n, char *val)
{
	if (!(n % 16U)) {
		/* resize */
		array = realloc(array, (n + 16U) * sizeof(*array));
	}
	array[[n]] = val;
	return array;
}

static enum yuck_cmds_e yuck_parse_cmd(const char *cmd)
{
	if (0) {
		;
	}foreachq([__CMD], yuck_cmds(), [ else if (yuck_streqp(cmd, "yuck_cmd_string(defn([__CMD]))")) {
		return yuck_cmd(defn([__CMD]));
	}]) else {
		/* error here? */
		fprintf(stderr, "YUCK_UMB_STR: invalid command `%s'\n\
Try `--help' for a list of commands.\n", cmd);
	}
	return (enum yuck_cmds_e)-1;
}


DEFUN int yuck_parse(yuck_t tgt[[static 1U]], int argc, char *argv[[]])
{
	char *op;
	char **args;
	int i;

	/* we'll have at most this many args */
	memset(tgt, 0, sizeof(*tgt));
	tgt->args = args = calloc(argc, sizeof(*tgt->args));
	tgt->nargs = 0U;
ifdef([YUCK_MAX_POSARGS], [], [define([YUCK_MAX_POSARGS], [(size_t)-1])])dnl
	for (i = 1; i < argc && tgt->nargs < YUCK_MAX_POSARGS; i++) {
		op = argv[[i]];

		switch (*op) {
		case '-':
			/* could be an option */
			switch (*++op) {
			default:
				/* could be glued into one */
				for (; *op; op++) {
					yield(shortopt, *op);
				}
				break;
			case '-':
				if (*++op == '\0') {
					i++;
					yield(dashdash);
					break;
				}
				yield(longopt, op);
				break;
			case '\0':
				goto plain_dash;
			}
			break;
		default:
		plain_dash:
			yield(arg, op);
			break;
		}
	}
	if (i < argc) {
		op = argv[[i]];

		if (*op++ == '-' && *op++ == '-' && !*op) {
			/* another dashdash, filter out */
			i++;
		}
	}
	/* has to be here as the max_pargs condition might drive us here */
	coroutine(dashdash)
	{
		/* dashdash loop, pile everything on tgt->args
		 * don't check for subcommands either, this is in accordance to
		 * the git tool which won't accept commands after -- */
		for (; i < argc; i++) {
			args[[tgt->nargs++]] = argv[[i]];
		}
	}
	return 0;

	coroutine(longopt)
	{
		/* split into option and arg part */
		char *arg;

		if ((arg = strchr(op, '=')) != NULL) {
			/* \nul this one out */
			*arg++ = '\0';
		}

		switch (tgt->cmd) {
		default:
			yield(yuck_cmd()[_longopt]);
			break;
		foreachq([__CMD], yuck_cmds(), [case yuck_cmd(defn([__CMD])):
			yield(yuck_cmd(defn([__CMD]))[_longopt]);
			break;
		])}
		resume;

dnl TYPE actions
pushdef([yuck_flag_action], [tgt->yuck_slot([$1], [$2])++])dnl
pushdef([yuck_arg_action], [tgt->yuck_slot([$1], [$2]) = arg ?: argv[[++i]]])dnl
pushdef([yuck_arg_opt_action], [tgt->yuck_slot([$1], [$2]) = arg ?: YUCK_OPTARG_NONE])dnl
pushdef([yuck_arg_mul_action], [tgt->yuck_slot([$1], [$2]) =
					yuck_append(
						tgt->yuck_slot([$1], [$2]), tgt->yuck_cnt_slot([$1], [$2])++,
						arg ?: argv[[++i]])])dnl
pushdef([yuck_arg_mul_opt_action], [tgt->yuck_slot([$1], [$2]) =
					yuck_append(
						tgt->yuck_slot([$1], [$2]), tgt->yuck_cnt_slot([$1], [$2])++,
						arg ?: YUCK_OPTARG_NONE)])dnl
pushdef([yuck_auto_action], [/* invoke auto action and exit */
				yuck_auto_[]yuck_canon([$1], [$2])(tgt);
				resume_at(success)])dnl

		foreachq([__CMD], yuck_umbcmds(), [coroutine(yuck_cmd(defn([__CMD]))[_longopt])
		{
			if (0) {
				;
			}dnl
foreachq([__IDN], yuck_idents(defn([__CMD])), [ dnl
pushdef([long], yuck_long(defn([__IDN]), defn([__CMD])))[]dnl
ifelse(defn([long]), [], [divert(-1)])dnl
else if (yuck_streqp(op, "defn([long])")) {
popdef([long])[]dnl
dnl now simply expand yuck_foo_action:
				yuck_option_action(defn([__IDN]), defn([__CMD]));
			}dnl
divert[]dnl
]) else {
ifelse(defn([__CMD]), [], [dnl
ifdef([YOPT_ALLOW_UNKNOWN_DASHDASH], [dnl
				/* just treat it as argument then */
				resume_at(arg);
], [dnl
				/* grml */
				fprintf(stderr, "YUCK_UMB_STR: unrecognized option `--%s'\n", op);
				resume_at(failure);
])dnl
], [dnl
				resume_at(yuck_cmd()[_longopt]);
])dnl
			}
			resume;
		}
		])
popdef([yuck_flag_action])dnl
popdef([yuck_arg_action])dnl
popdef([yuck_arg_mul_action])dnl
popdef([yuck_arg_opt_action])dnl
popdef([yuck_arg_mul_opt_action])dnl
popdef([yuck_auto_action])dnl
	}

	coroutine(shortopt)
	{
		char *arg = op + 1U;

		switch (tgt->cmd) {
		default:
			yield(yuck_cmd()[_shortopt]);
			break;
		foreachq([__CMD], yuck_cmds(), [case yuck_cmd(defn([__CMD])):
			yield(yuck_cmd(defn([__CMD]))[_shortopt]);
			break;
		])}
		resume;

dnl TYPE actions
pushdef([yuck_flag_action], [tgt->yuck_slot([$1], [$2])++])dnl
pushdef([yuck_arg_action], [tgt->yuck_slot([$1], [$2]) = *arg
					? (op += strlen(arg), arg)
					: argv[[++i]]])dnl
pushdef([yuck_arg_opt_action], [tgt->yuck_slot([$1], [$2]) = *arg
					? (op += strlen(arg), arg)
					: YUCK_OPTARG_NONE])dnl
pushdef([yuck_arg_mul_action], [tgt->yuck_slot([$1], [$2]) =
					yuck_append(
						tgt->yuck_slot([$1], [$2]),
						tgt->yuck_cnt_slot([$1], [$2])++,
						*arg ? (op += strlen(arg), arg) : argv[[++i]])])dnl
pushdef([yuck_arg_mul_opt_action], [tgt->yuck_slot([$1], [$2]) =
					yuck_append(
						tgt->yuck_slot([$1], [$2]),
						tgt->yuck_cnt_slot([$1], [$2])++,
						*arg ? (op += strlen(arg), arg) : YUCK_OPTARG_NONE)])dnl
pushdef([yuck_auto_action], [/* invoke auto action and exit */
				yuck_auto_[]yuck_canon([$1], [$2])(tgt);
				resume_at(success)])dnl

		foreachq([__CMD], yuck_umbcmds(), [coroutine(yuck_cmd(defn([__CMD]))[_shortopt])
		{
			switch (*op) {
			default:
				divert(1);
ifdef([YOPT_ALLOW_UNKNOWN_DASH], [dnl
				resume_at(arg);
], [dnl
				fprintf(stderr, "YUCK_UMB_STR: invalid option -%c\n", *op);
				resume_at(failure);
])dnl

ifdef([YUCK_SHORTS_HAVE_NUMERALS], [
				/* [yuck_shorts()] (= yuck_shorts())
				 * has numerals as shortopts
				 * don't allow literal treatment of numerals */divert(-1)])
			case '0' ... '9':;
				/* literal treatment of numeral */
				resume_at(arg);

				divert(2);
				resume_at(yuck_cmd()[_shortopt]);

				divert(0);
				ifelse(defn([__CMD]), [], [select_divert(1)], [select_divert(2)])dnl
divert[]dnl

foreachq([__IDN], yuck_idents(defn([__CMD])), [dnl
pushdef([short], yuck_short(defn([__IDN]), defn([__CMD])))dnl
ifelse(defn([short]), [], [divert(-1)])dnl
			case 'defn([short])':
popdef([short])dnl
dnl
dnl now simply expand yuck_foo_action:
				yuck_option_action(defn([__IDN]), defn([__CMD]));
				break;
divert[]dnl
])dnl
			}
			resume;
		}
		])
popdef([yuck_flag_action])dnl
popdef([yuck_arg_action])dnl
popdef([yuck_arg_opt_action])dnl
popdef([yuck_arg_mul_action])dnl
popdef([yuck_arg_mul_opt_action])dnl
popdef([yuck_auto_action])dnl
	}

	coroutine(arg)
	{
		if (tgt->cmd || !YUCK_NCMDS) {
			args[[tgt->nargs++]] = argv[[i]];
		} else {
			/* ah, might be an arg then */
			if ((tgt->cmd = yuck_parse_cmd(op)) > YUCK_NCMDS) {
				return -1;
			}
		}
		resume;
	}

	coroutine(failure)
	{
		exit(EXIT_FAILURE);
	}

	coroutine(success)
	{
		exit(EXIT_SUCCESS);
	}
}

DEFUN void yuck_free(yuck_t tgt[[static 1U]])
{
	if (tgt->args != NULL) {
		/* free despite const qualifier */
		free(tgt->args);
	}
	/* free mulargs */
	switch (tgt->cmd) {
		void *ptr;
	default:
		break;
pushdef([action], [dnl
		ptr = tgt->yuck_slot([$1], [$2]);
		if (ptr != NULL) {
			free(ptr);
		}
])dnl
dnl TYPE actions
pushdef([yuck_flag_action], [])dnl
pushdef([yuck_arg_action], [])dnl
pushdef([yuck_arg_opt_action], [])dnl
pushdef([yuck_arg_mul_action], defn([action]))dnl
pushdef([yuck_arg_mul_opt_action], defn([action]))dnl
pushdef([yuck_auto_action], [])dnl
foreachq([__CMD], yuck_umbcmds(), [dnl
	case yuck_cmd(defn([__CMD])):
foreachq([__IDN], yuck_idents(defn([__CMD])), [dnl
yuck_option_action(defn([__IDN]), defn([__CMD]));
		break;
])[]dnl
])[]dnl
popdef([action])dnl
popdef([yuck_flag_action])dnl
popdef([yuck_arg_action])dnl
popdef([yuck_arg_opt_action])dnl
popdef([yuck_arg_mul_action])dnl
popdef([yuck_arg_mul_opt_action])dnl
popdef([yuck_auto_opt_action])dnl
		break;
	}
	return;
}

DEFUN void yuck_auto_usage(const yuck_t src[[static 1U]])
{
	switch (src->cmd) {
	default:
	YUCK_NOCMD:
		puts("Usage: YUCK_UMB_STR [[OPTION]]...dnl
ifelse(yuck_cmds(), [], [], [ COMMAND])[]dnl
ifelse(defn([YUCK_UMB_POSARG]), [], [], [ defn([YUCK_UMB_POSARG])])\n\
ifelse(yuck_umb_desc(), [], [], [dnl
\n\
yuck_C_literal(yuck_umb_desc())\n\
])dnl
");
		break;
foreachq([__CMD], yuck_cmds(), [
	case yuck_cmd(defn([__CMD])):
		puts("Usage: YUCK_UMB_STR dnl
yuck_cmd_string(defn([__CMD]))[]dnl
ifelse(yuck_idents(defn([__CMD])), [], [], [ [[OPTION]]...])[]dnl
ifelse(yuck_cmd_posarg(defn([__CMD])), [], [], [ yuck_cmd_posarg(defn([__CMD]))])\n\
ifelse(yuck_cmd_desc(defn([__CMD])), [], [], [dnl
\n\
yuck_C_literal(yuck_cmd_desc(defn([__CMD])))\n\
])dnl
");
		break;
])
	}

#if defined yuck_post_usage
	yuck_post_usage(src);
#endif	/* yuck_post_usage */
	return;
}

DEFUN void yuck_auto_help(const yuck_t src[[static 1U]])
{
	yuck_auto_usage(src);

ifelse(yuck_cmds(), [], [], [dnl
	if (src->cmd == YUCK_NOCMD) {
		/* also output a list of commands */
		puts("COMMAND may be one of:\n\
foreachq([__CMD], yuck_cmds(), [yuck_C_literal(yuck_cmd_line(defn([__CMD])))\n\
])");
	}
])dnl

	/* leave a not about common options */
	if (src->cmd == YUCK_NOCMD) {
ifelse(yuck_cmds(), [], [dnl
		;
], [dnl
		puts("\
Options accepted by all commands:");
])dnl
ifelse(yuck_cmds(), [], [], [dnl
	} else {
		puts("\
Common options:\n\
foreachq([__IDN], yuck_idents([]), [dnl
yuck_C_literal(backquote([yuck_option_help_line(defn([__IDN]), [])]))[]dnl
])dnl
");
])dnl
	}

	switch (src->cmd) {
	default:foreachq([__CMD], yuck_umbcmds(), [
	case yuck_cmd(defn([__CMD])):
		puts("\
ifelse(defn([__CMD]), [], [], [dnl
ifelse(yuck_idents(defn([__CMD])), [], [], [dnl
Command-specific options:\n\
])dnl
])dnl
foreachq([__IDN], yuck_idents(defn([__CMD])), [dnl
yuck_C_literal(backquote([yuck_option_help_line(defn([__IDN]), defn([__CMD]))]))[]dnl
])dnl
");
		break;
])
	}

#if defined yuck_post_help
	yuck_post_help(src);
#endif	/* yuck_post_help */

#if defined PACKAGE_BUGREPORT
	puts("\n\
Report bugs to " PACKAGE_BUGREPORT);
#endif	/* PACKAGE_BUGREPORT */
	return;
}

DEFUN void yuck_auto_version(const yuck_t src[[static 1U]])
{
	switch (src->cmd) {
	default:
ifdef([YUCK_VERSION], [dnl
		puts("YUCK_UMB_STR YUCK_VERSION");
], [dnl
#if 0

#elif defined package_string
		puts(package_string);
#elif defined package_version
		printf("YUCK_UMB_STR %s\n", package_version);
#elif defined PACKAGE_STRING
		puts(PACKAGE_STRING);
#elif defined PACKAGE_VERSION
		puts("YUCK_UMB_STR " PACKAGE_VERSION);
#elif defined VERSION
		puts("YUCK_UMB_STR " VERSION);
#else  /* !PACKAGE_VERSION, !VERSION */
		puts("YUCK_UMB_STR unknown version");
#endif	/* PACKAGE_VERSION */
])dnl
		break;
	}

#if defined yuck_post_version
	yuck_post_version(src);
#endif	/* yuck_post_version */
	return;
}
popdef([DEFUN])dnl

#if defined __INTEL_COMPILER
# pragma warning (default:177)
# pragma warning (default:111)
#elif defined __GNUC__
# if __GNUC__ > 4 || __GNUC__ == 4 &&  __GNUC_MINOR__ >= 6
#  pragma GCC diagnostic pop
# endif	 /* GCC version */
#endif	/* __INTEL_COMPILER */
changequote`'dnl
