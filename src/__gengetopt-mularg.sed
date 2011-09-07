/^gengetopt_strdup (const char \*s);/ {
	s/$/\n#include "__gengetopt-mularg.h"/
}
s/^get_multiple_arg_token/__attribute__((unused)) g_m_a_t/
