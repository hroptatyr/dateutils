#include <stdio.h>
#include "strops.h"
#include "nifty.h"

int
main(void)
{
	static char buf[32U];

	buf[ui99topstr(buf, 32U, 23U, 2U, ' ')] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 1U, 2U, ' ')] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 0U, 2U, ' ')] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 1U, 2U, '0')] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 23U, 2U, '0')] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 23U, 1U, ' ')] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 3U, 1U, ' ')] = '\0';
	puts(buf);

	puts("");

	buf[ui99topstr(buf, 32U, 23U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 1U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 0U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 1U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 23U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 23U, 1U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 32U, 3U, 1U, 0)] = '\0';
	puts(buf);

	puts("");

	buf[ui99topstr(buf, 1U, 23U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 1U, 1U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 1U, 0U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 1U, 1U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 1U, 23U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 1U, 23U, 1U, 0)] = '\0';
	puts(buf);
	buf[ui99topstr(buf, 1U, 3U, 1U, 0)] = '\0';
	puts(buf);
	return 0;
}
