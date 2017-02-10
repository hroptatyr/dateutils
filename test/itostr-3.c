#include <stdio.h>
#include "strops.h"
#include "nifty.h"

int
main(void)
{
	static char buf[32U];

	buf[ui9999topstr(buf, 32U, 1234U, 4U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 23U, 4U, ' ')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 10U, 3U, ' ')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 0U, 3U, '0')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 1U, 4U, '0')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 123U, 3U, '0')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 23U, 1U, ' ')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 3U, 4U, ' ')] = '\0';
	puts(buf);

	puts("");

	buf[ui9999topstr(buf, 32U, 1234U, 3U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 23U, 2U, ' ')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 10U, 2U, ' ')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 0U, 2U, ' ')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 1U, 2U, '0')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 123U, 2U, '0')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 23U, 4U, '0')] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 32U, 3U, 1U, ' ')] = '\0';
	puts(buf);

	puts("");

	buf[ui9999topstr(buf, 3U, 1234U, 2U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 1U, 23U, 4U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 2U, 23U, 4U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 3U, 23U, 4U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 4U, 23U, 4U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 1U, 123U, 1U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 1U, 23U, 1U, 0)] = '\0';
	puts(buf);
	buf[ui9999topstr(buf, 1U, 3U, 1U, 0)] = '\0';
	puts(buf);
	return 0;
}
