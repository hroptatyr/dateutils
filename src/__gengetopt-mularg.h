/* used to overcome the comma-separated-multiarg feature,
 * since we need to allow to put commas unescaped as arguments */
#if !defined INCLUDED___gengetopt_mularg_h_
#define INCLUDED___gengetopt_mularg_h_

static char*
get_multiple_arg_token(const char *arg)
{
	return gengetopt_strdup(arg);
}

static const char*
get_multiple_arg_token_next(const char *arg)
{
	FIX_UNUSED(arg);
	return 0;
}

#endif	/* INCLUDED___gengetopt_mularg_h_ */
