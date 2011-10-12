/^gengetopt_strdup (const char \*s);/ {
	s/$/\n#include "__gengetopt-mularg.h"/
}
s/^get_multiple_arg_token/__attribute__((unused)) g_m_a_t/

## while we're at it, save some time by not strdup()ing strings
s/override, 0/override, 1/
s/free_string.*_orig.*//
