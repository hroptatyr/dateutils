changequote`'changequote([,])dnl
divert([-1])

## this is a little domain language for the yuck processor

# foreachq(x, [item_1, item_2, ..., item_n], stmt)
#   quoted list, alternate improved version
define([foreachq], [ifelse([$2], [], [],
	[pushdef([$1])_$0([$1], [$3], [], $2)popdef([$1])])])
define([_foreachq], [ifelse([$#], [3], [],
	[define([$1], [$4])$2[]$0([$1], [$2],
		shift(shift(shift($@))))])])

#### append $2 to $1, separated by $3.
define([append], [define([$1], ifdef([$1], [defn([$1])[$3]])[$2])])
## like append, but append only non-empty arguments
define([append_ne], [ifelse([$2], [], [], [append([$1], [$2], [$3])])])
## like append_ne, but append only non-existing arguments
define([append_nene], [ifelse(index([$3]defn([$1])[$3], [$3$2$3]), [-1],
	[append_ne([$1], [$2], [$3])])])

define([appendq], [define([$1], ifdef([$1], [defn([$1])[$3]])dquote([$2]))])
## like appendq, but append only non-empty arguments
define([appendq_ne], [ifelse([$2], [], [], [append([$1], dquote([$2]), [$3])])])
## like appendq_ne, but append only non-existing arguments
define([appendq_nene], [ifelse(index([$3]defn([$1])[$3], [$3]dquote($2)[$3]), [-1],
	[appendq_ne([$1], [$2], [$3])])])

define([first_nonnil], [ifelse([$#], [0], [], [$1], [],
	[first_nonnil(shift($@))], [], [], [$1])])
define([first], [_first($*)])
define([_first], [$1])
define([second], [_second($*)])
define([_second], [$2])
define([thirds], [_thirds($*)])
define([_thirds], [quote(shift(shift($@)))])

define([quote], [ifelse([$#], [0], [], [[$*]])])
define([dquote], [[$@]])
define([equote], [dquote($*)])

define([backquote], [_$0([$1], [(], -=<{($1)}>=-, [}>=-])])
define([_backquote], [dnl
ifelse([$4], [}>=-], [dnl
changequote([-=<{$2], [)}>=-])$3changequote([, ])], [dnl else
$0([$1], [($2], -=<{($2$1)}>=-, [}>=-])_ignore$2])])

define([_ignore])

define([_splice], [ifelse(eval([$#] > [3]), [0], [[$1], [$2], [$3]], [[$1], [$2], [$3], _splice([$1], shift(shift(shift($@))))])])

define([cond], [ifelse([$#], [0], [], [$#], [1], [$1], [_$0($@)])])
define([_cond], [dnl
ifelse([$1], [$2], [$3],
	[$#], [3], [],
	[$#], [4], [$4],
	[$0([$1], shift(shift(shift($@))))])])

define([downcase], [dnl
translit([$1], [ABCDEFGHIJKLMNOPQRSTUVWXYZ], [abcdefghijklmnopqrstuvwxyz])[]dnl
])

define([upcase], [dnl
translit([$1], [abcdefghijklmnopqrstuvwxyz], [ABCDEFGHIJKLMNOPQRSTUVWXYZ])[]dnl
])

## select a diversion, clearing all other diversions
define([select_divert], [divert[]undivert($1)[]divert(-1)[]undivert[]divert(0)])


define([yuck_set_version], [dnl
	define([YUCK_VERSION], [$1])
])

## yuck_set_umbrella([ident], [umbrella], [[posarg]])
define([yuck_set_umbrella], [dnl
	define([YUCK_UMB], [$1])
	define([YUCK_UMB_STR], [$2])
	define([YUCK_UMB_POSARG], [$3])
])

## yuck_set_umbrella_desc([ident], [desc])
define([yuck_set_umbrella_desc], [dnl
	define([YUCK_UMB_$1_desc], [$2])

	## define yuck_add_inter here, corresponding to IDENT
	define([YUCK_INTER_CMD], [YUCK_UMB_$1_desc])
])

## yuck_set_umbrella_max_posargs([ident], [N])
define([yuck_set_umbrella_max_posargs], [dnl
	define([YUCK_UMB_$1_max_posargs], [$2])
	define([YUCK_MAX_POSARGS], [$2])
])

## yuck_add_command([ident], [command], [[posarg]])
define([yuck_add_command], [dnl
	define([YUCK_CMD], [$1])
	append_nene([YUCK_ALL_CMDS], [$1], [,])
	define([YUCK_$1_STR], [$2])
	define([YUCK_POSARG_$1], [$3])
])

## yuck_set_command_desc([ident], [desc])
define([yuck_set_command_desc], [dnl
	define([YUCK_CMD_$1_desc], [$2])

	## define yuck_add_inter here, corresponding to IDENT
	define([YUCK_INTER_CMD], [YUCK_CMD_$1_desc])
])

## yuck_set_umbrella_max_posargs([ident], [N])
define([yuck_set_command_max_posargs], [dnl
	define([YUCK_CMD_$1_max_posargs], [$2])
])

## yuck_add_inter([desc])
#### append interim DESC to last command desc
define([yuck_add_inter], [dnl
	append(defn([YUCK_INTER_CMD]), [$1], [

])
])

## yuck_add_option([ident], [short], [long], [type])
define([yuck_add_option], [dnl
	## quote the elements of the type arg first
	## before any possible expansion is in scope
	pushdef([type], equote([$4]))
	pushdef([ident], [$1])
	pushdef([cmd], defn([YUCK_CMD]))

	ifelse([$2], [], [],
		index([0123456789], [$2]), [-1], [],
		[dnl else
		define([YUCK_SHORTS_HAVE_NUMERALS], [1])
	])

	ifdef([YUCK_]defn([cmd])[_]defn([ident])[_canon], [], [dnl
		## process only if new
		appendq_ne([YUCK_]defn([cmd])[_I], defn([ident]), [,])

		## forward maps
		define([YUCK_]defn([cmd])[_]defn([ident])[_canon], defn([ident]))
		define([YUCK_]defn([cmd])[_]defn([ident])[_type], defn([type]))

		## reverse maps
		define([YUCK_]defn([cmd])[_]defn([ident])[_short], [$2])
		define([YUCK_]defn([cmd])[_]defn([ident])[_long], [$3])
	])

	popdef([ident])
	popdef([cmd])
	popdef([type])
])

## yuck_set_option_desc([ident], [desc])
define([yuck_set_option_desc], [dnl
	define([YUCK_]defn([YUCK_CMD])[_$1_desc], [$2])
])

## some yopts here, mostly flags
define([yuck_setopt_allow_unknown_dash], [dnl
	define([YOPT_ALLOW_UNKNOWN_DASH])
])

define([yuck_setopt_allow_unknown_dashdash], [dnl
	define([YOPT_ALLOW_UNKNOWN_DASHDASH])
])


## helpers for the m4c and m4h

## yuck_canon([opt], [[cmd]])
define([yuck_canon], [defn([YUCK_$2_$1_canon])])

## yuck_option_type([opt], [[cmd]])
define([yuck_option_type], [defn([YUCK_$2_$1_type])])

## yuck_type([type-spec])
define([yuck_type], [first([$1])])

## yuck_type_name([type-spec])
define([yuck_type_name], [second([$1])])

## yuck_type_sufx([type-spec])
define([yuck_type_sufx], [thirds([$1])])

## yuck_slot_identifier([option], [[cmd]])
define([yuck_slot_identifier], [dnl
pushdef([canon], yuck_canon([$1], [$2]))dnl
pushdef([type], yuck_option_type([$1], [$2]))dnl
dnl
defn([canon])[_]yuck_type(defn([type]))[]dnl
cond(yuck_type_sufx(defn([type])), [mul], [s], [mul,opt], [s])[]dnl
dnl
popdef([canon])dnl
popdef([type])dnl
])

## yuck_cnt_slot([option], [[cmd]])
define([yuck_cnt_slot], [dnl
pushdef([type], yuck_option_type([$1], [$2]))dnl
ifelse(yuck_type(defn([type])), [arg], [dnl
ifelse(first(yuck_type_sufx(defn([type]))), [mul], [dnl
pushdef([idn], [yuck_canon([$1], [$2])[_nargs]])dnl
ifelse([$2], [], [idn], [$2.idn])[]dnl
popdef([idn])dnl
])[]dnl
])[]dnl
popdef([type])dnl
])

## yuck_slot([option], [[cmd]])
define([yuck_slot], [dnl
pushdef([idn], yuck_slot_identifier([$1], [$2]))dnl
dnl
ifelse([$2], [], defn([idn]), [$2.]defn([idn]))[]dnl
dnl
popdef([idn])dnl
])

## yuck_iftype([opt], [cmd], [type], [body], [[type], [body]]...)
define([yuck_iftype], [dnl
pushdef([type], yuck_option_type([$1], [$2]))dnl
pushdef([tsuf], yuck_type_sufx(defn([type])))dnl
pushdef([res], yuck_type(defn([type])))dnl
append_ne([res], defn([tsuf]), [,])[]dnl
[]ifelse(_splice(defn([res]), shift(shift($@))))[]dnl
popdef([tsuf])dnl
popdef([type])dnl
popdef([res])dnl
])

## yuck_umbcmds(), umbrella + commands
define([yuck_umbcmds], [ifdef([YUCK_ALL_CMDS], [[,]defn([YUCK_ALL_CMDS])], dquote([[]]))])

## yuck_cmds(), just the commands
define([yuck_cmds], [defn([YUCK_ALL_CMDS])])

## yuck_cmd([command])
define([yuck_cmd], [upcase(defn([YUCK_UMB]))[_CMD_]ifelse([$1], [], [NONE], [upcase([$1])])])

## yuck_cmd_string
define([yuck_cmd_string], [defn([YUCK_]$1[_STR])])

## yuck_cmd_posarg
define([yuck_cmd_posarg], [defn([YUCK_POSARG_]$1)])

## yuck_umb_desc([[umb]]) getter for the umbrella description
define([yuck_umb_desc], [defn([YUCK_UMB_]ifelse([$1], [], defn([YUCK_UMB]), [$1])[_desc])])

## yuck_cmd_desc([cmd]) getter for the command description
define([yuck_cmd_desc], [defn([YUCK_CMD_$1_desc])])

## yuck_idents([cmd])
define([yuck_idents], [defn([YUCK_$1_I])])

## yuck_short([ident], [[cmd]])
define([yuck_short], [defn([YUCK_$2_$1_short])])

## yuck_long([ident], [[cmd]])
define([yuck_long], [defn([YUCK_$2_$1_long])])

## yuck_option_desc([ident], [[cmd]])
define([yuck_option_desc], [defn([YUCK_$2_$1_desc])])

## type actions
define([_yuck_option_action], [dnl
pushdef([type], yuck_option_type([$1], [$2]))dnl
pushdef([prim], yuck_type(defn([type])))dnl
pushdef([sufx], yuck_type_sufx(defn([type])))dnl
quote([yuck_]defn([prim])ifelse(defn([sufx]), [], [], [_]translit(defn([sufx]), [,], [_]))[_action][(quote([$1]), quote([$2]))])dnl
popdef([type])dnl
popdef([prim])dnl
popdef([sufx])dnl
])dnl
define([yuck_expand], [$1])
define([yuck_option_action], [yuck_expand(_$0([$1], [$2]))])

## yuck_option_help_lhs([ident], [[cmd]])
define([yuck_option_help_lhs], [dnl
pushdef([s], [backquote([yuck_short([$1], [$2])])])dnl
pushdef([l], [backquote([yuck_long([$1], [$2])])])dnl
pushdef([type], yuck_option_type([$1], [$2]))dnl
pushdef([prel], ifelse(l, [], [], [=]))dnl
pushdef([yuck_arg_action], [defn([prel])[]yuck_type_name(yuck_option_type([$1], [$2]))])dnl
pushdef([yuck_arg_opt_action], [defn([prel])[]yuck_type_name(yuck_option_type([$1], [$2]))])dnl
pushdef([yuck_arg_mul_action], [defn([prel])[]yuck_type_name(yuck_option_type([$1], [$2]))...])dnl
pushdef([yuck_arg_mul_opt_action], [defn([prel])[]yuck_type_name(yuck_option_type([$1], [$2]))...])dnl
[  ]ifelse(s, [], [    ], [-s[]ifelse(l, [], [], [[, ]])])[]dnl
ifelse(l, [], [], [--l])[]dnl
ifelse(yuck_type(defn([type])), [arg], [dnl
ifelse(l, [], [ ], [])backquote([yuck_option_action([$1], [$2])])[]dnl
])[]dnl
popdef([type])dnl
popdef([prel])dnl
popdef([s])dnl
popdef([l])dnl
popdef([yuck_arg_action])dnl
popdef([yuck_arg_opt_action])dnl
popdef([yuck_arg_mul_action])dnl
popdef([yuck_arg_mul_opt_action])dnl
])

define([xleft], [_$0([$1], 0, [$2])])
define([_xleft], [dnl
ifelse(eval(incr(incr([$2])) <= [$3]), [1], [dnl
substr([$1], [$2], 2)[]$0([$1], incr(incr([$2])), [$3])[]dnl
], eval(incr([$2]) <= [$3]), [1], [dnl
substr([$1], [$2], 1)[]dnl
])[]dnl
])

define([xright], [_$0([$1], [$2])])
define([_xright], [dnl
ifelse(eval([$2] >= len([$1])), [0], [dnl
substr([$1], [$2], 2)[]$0([$1], incr(incr([$2])))[]dnl
])[]dnl
])

define([yuck_esc], [backquote([_$0([$1], [$2], [$3], [$4])])])
define([_yuck_esc], [dnl
pushdef([__next_sep], index([$1], [$2]))[]dnl
ifelse([$1], [], [], defn([__next_sep]), [-1], [$4[$1]], [dnl
[$4]xleft([$1], defn([__next_sep]))[$3]dnl
$0(backquote([xright([$1], eval(defn([__next_sep]) + len([$2])))]), [$2], [$3], [$4])[]dnl
])[]dnl
popdef([__next_sep])dnl
])dnl

define([yuck_indent_line], [yuck_esc([$1], [
], [
], [                        ])])

## yuck_option_help_line([ident], [[cmd]])
define([yuck_option_help_line], [dnl
pushdef([lhs], [backquote([yuck_option_help_lhs([$1], [$2])])])dnl
pushdef([desc], [yuck_option_desc([$1], [$2])])dnl
pushdef([indesc], [yuck_indent_line(backquote([desc]))])dnl
pushdef([lenlhs], len(lhs))dnl
ifelse(indesc, [], [lhs],
eval(lenlhs >= 23), [0], [dnl
lhs[]backquote([xright(indesc, lenlhs)])[]dnl
], eval(lenlhs >= 24), [0], [dnl
lhs[]backquote([xright(indesc, decr(lenlhs))])[]dnl
], [dnl
lhs[
]backquote([indesc])[]dnl
])
popdef([lenlhs])dnl
popdef([indesc])dnl
popdef([desc])dnl
popdef([lhs])dnl
])

## yuck_first_line([string])
define([yuck_first_line], [dnl
pushdef([lnlen], [index([$1], [
])])dnl
backquote([ifelse(lnlen, -1, [$1], [xleft([$1], lnlen)])])[]dnl
popdef([lnlen])dnl
])

## yuck_cmd_line([cmd])
define([yuck_cmd_line], [dnl
pushdef([lhs], [backquote([yuck_cmd_string([$1])])])dnl
pushdef([indesc], [dnl
            yuck_first_line(backquote([yuck_cmd_desc([$1])]))])dnl
pushdef([lenlhs], len(lhs))dnl
ifelse(indesc, [], [lhs],
eval(lenlhs >= 11), [0], [dnl
  lhs[]backquote([xright(indesc, lenlhs)])[]dnl
], eval(lenlhs >= 12), [0], [dnl
  lhs[]backquote([xright(indesc, decr(lenlhs))])[]dnl
], [dnl
  lhs
  backquote([indesc])[]dnl
])dnl
popdef([lenlhs])dnl
popdef([indesc])dnl
popdef([lhs])dnl
])

## \n -> \\n\\
define([yuck_esc_newline], [yuck_esc([$1], [
], [\n\
])])

## " -> \"
define([yuck_esc_quote], [yuck_esc([$1], ["], [\"])])dnl "

## \ -> \\
define([yuck_esc_backslash], [yuck_esc([$1], [\], [\\])])dnl

define([yuck_C_literal], [dnl
yuck_esc_newline(yuck_esc_quote(yuck_esc_backslash([$1])))[]dnl
])dnl


## coroutine stuff
define([yield], [goto $1; back_from_$1:])
define([coroutine], [define([this_coru], [$1])$1:])
define([resume], [goto back_from_[]this_coru])
define([resume_at], [goto $1])
define([quit], [goto out])

divert[]dnl
changequote`'dnl
