/* -*- c -*- */
changequote`'changequote([,])dnl
changecom([#])dnl
#if !defined INCLUDED_yuck_h_
#define INCLUDED_yuck_h_

#include <stddef.h>

#define YUCK_OPTARG_NONE	((void*)0x1U)

enum yuck_cmds_e {
pushdef([last], yuck_cmd())pushdef([first], defn([last]))[]dnl
	/* value used when no command was specified */
	first = 0U,

	/* actual commands */
	foreachq([cmd], yuck_cmds(), [define([last], yuck_cmd(defn([cmd])))[]last,
	])
	/* convenience identifiers */
	YUCK_NOCMD = first,
	YUCK_NCMDS = last
popdef([last])popdef([first])[]dnl
};

define([yuck_slot_predecl], [dnl
yuck_iftype([$1], [$2],
	[arg,mul], [size_t ]yuck_canon([$1], [$2])[_nargs],
	[arg,mul,opt], [size_t ]yuck_canon([$1], [$2])[_nargs],
)dnl
])dnl

define([yuck_slot_decl], [dnl
pushdef([pre], [yuck_slot_predecl([$1], [$2])])dnl
pushdef([ident], [yuck_slot_identifier([$1], [$2])])dnl
yuck_iftype([$1], [$2],
	[flag], [unsigned int ident;],
	[arg], [char *ident;],
	[arg,opt], [char *ident;],
	[arg,mul], [pre; char **ident;],
	[arg,mul,opt], [pre; char **ident;],
	[auto], [/* $1 is handled automatically */])dnl
popdef([pre])dnl
popdef([ident])dnl
])dnl

ifelse(yuck_cmds(), [], [dnl
typedef struct yuck_s yuck_t;
], [dnl
typedef union yuck_u yuck_t;
])dnl

foreachq([cmd], yuck_cmds(), [
/* convenience structure for `cmd' */
struct yuck_cmd_[]defn([cmd])[]_s {
	enum yuck_cmds_e [cmd];

	/* left-over arguments, the command string is never a part of this */
	size_t nargs;
	char **args;
foreachq([slot], yuck_idents(), [
	yuck_slot_decl(defn([slot]))[]dnl
])
foreachq([slot], yuck_idents(defn([cmd])), [
	yuck_slot_decl(defn([slot]), defn([cmd]))[]dnl
])
};
])

ifelse(yuck_cmds(), [], [dnl
/* generic struct */
struct yuck_s {
	enum yuck_cmds_e cmd;

	/* left-over arguments,
	 * the command string is never a part of this */
	size_t nargs;
	char **args;

	/* slots common to all commands */
foreachq([slot], yuck_idents(), [
	yuck_slot_decl(defn([slot]))[]dnl
])
};
], [dnl else
union yuck_u {
	/* generic struct */
	struct {
		enum yuck_cmds_e cmd;

		/* left-over arguments,
		 * the command string is never a part of this */
		size_t nargs;
		char **args;

		/* slots common to all commands */
foreachq([slot], yuck_idents(), [dnl
		yuck_slot_decl(defn([slot]))
])dnl
	};

	/* depending on CMD at most one of the following structs is filled in
	 * if CMD is YUCK_NONE no slots of this union must be accessed */
foreachq([cmd], yuck_cmds(), [dnl
	struct yuck_cmd_[]defn([cmd])[]_s defn([cmd]);
])dnl
};
])

pushdef([DECLF], ifdef([YUCK_HEADER], [extern], [static]))[]dnl
DECLF __attribute__((nonnull(1))) int
yuck_parse(yuck_t*, int argc, char *argv[[]]);
DECLF __attribute__((nonnull(1))) void yuck_free(yuck_t*);

DECLF __attribute__((nonnull(1))) void yuck_auto_help(const yuck_t*);
DECLF __attribute__((nonnull(1))) void yuck_auto_usage(const yuck_t*);
DECLF __attribute__((nonnull(1))) void yuck_auto_version(const yuck_t*);

/* some hooks */
#if defined yuck_post_help
DECLF __attribute__((nonnull(1))) void yuck_post_help(const yuck_t*);
#endif	/* yuck_post_help */

#if defined yuck_post_usage
DECLF __attribute__((nonnull(1))) void yuck_post_usage(const yuck_t*);
#endif	/* yuck_post_usage */

#if defined yuck_post_version
DECLF __attribute__((nonnull(1))) void yuck_post_version(const yuck_t*);
#endif	/* yuck_post_version */
popdef([DECLF])[]dnl

#endif	/* INCLUDED_yuck_h_ */
changequote`'dnl
