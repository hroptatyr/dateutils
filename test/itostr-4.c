#include <stdio.h>
#include "strops.h"
#include "nifty.h"

int
main(void)
{
	static char buf[32U];

	buf[ui999999999tostr(buf, 32U, 0U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 32U, 1U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 32U, 1234U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 32U, 12345678U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 32U, 123456789U)] = '\0';
	puts(buf);

	puts("");

	buf[ui999999999tostr(buf, 6U, 0U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 6U, 1000U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 6U, 123400U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 6U, 12345678U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 6U, 123456789U)] = '\0';
	puts(buf);

	puts("");

	buf[ui999999999tostr(buf, 3U, 0U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 3U, 1U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 3U, 1234U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 3U, 12345678U)] = '\0';
	puts(buf);
	buf[ui999999999tostr(buf, 3U, 123456789U)] = '\0';
	puts(buf);
	return 0;
}
