/*
  File: worstbit.c

  Copyright (C) 2003 Andreas Gruenbacher <a.gruenbacher@bestbits.at> 
  modified by Bertera Pietro for manipulating getfacl dumps
    
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int high_water_alloc(void **buf, size_t *bufsize, size_t newsize)
{
#define CHUNK_SIZE  256
    /*
     * Goal here is to avoid unnecessary memory allocations by
     * using static buffers which only grow when necessary.
     * Size is increased in fixed size chunks (CHUNK_SIZE).
     */
    if (*bufsize < newsize) {
        void *newbuf;

        newsize = (newsize + CHUNK_SIZE-1) & ~(CHUNK_SIZE-1);
        newbuf = realloc(*buf, newsize);
        if (!newbuf)
            return 1;

        *buf = newbuf;
        *bufsize = newsize;
    }
    return 0;
}

char *unquote(char *str)
{
	unsigned char *s, *t;

	if (!str)
		return str;

	for (s = (unsigned char *)str; *s != '\0'; s++)
		if (*s == '\\')
			break;
	if (*s == '\0')
		return str;

#define isoctal(c) \
	((c) >= '0' && (c) <= '7')

	t = s;
	do {
		if (*s == '\\' &&
		    isoctal(*(s+1)) && isoctal(*(s+2)) && isoctal(*(s+3))) {
			*t++ = ((*(s+1) - '0') << 6) +
			       ((*(s+2) - '0') << 3) +
			       ((*(s+3) - '0')     );
			s += 3;
		} else
			*t++ = *s;
	} while (*s++ != '\0');

	return str;
}

const char *quote(const char *str)
{
    static char *quoted_str;
    static size_t quoted_str_len;
    const unsigned char *s;
    char *q;
    size_t nonpr;

    if (!str)
        return str;

    for (nonpr = 0, s = (unsigned char *)str; *s != '\0'; s++)
        if (!isprint(*s) || isspace(*s) || *s == '\\' || *s == '=')
            nonpr++;
    if (nonpr == 0)
        return str;

    if (high_water_alloc((void **)&quoted_str, &quoted_str_len,
                 (s - (unsigned char *)str) + nonpr * 3 + 1))
        return NULL;
    for (s = (unsigned char *)str, q = quoted_str; *s != '\0'; s++) {
        if (!isprint(*s) || isspace(*s) || *s == '\\' || *s == '=') {
            *q++ = '\\';
            *q++ = '0' + ((*s >> 6)    );
            *q++ = '0' + ((*s >> 3) & 7);
            *q++ = '0' + ((*s     ) & 7);
        } else
            *q++ = *s;
    }
    *q++ = '\0';

    return quoted_str;
}


int main (int argc, char **argv)
{
    char *a;
    int i;

    if ((argv[1] == "-h") || (argc != 3)) {
        fprintf (stderr, "Usage: %s [-quh]\"string\"\n", argv[0]);
        fprintf (stderr, "\t-q\tquote string\n");
        fprintf (stderr, "\t-u\tunquote string\n");
        fprintf (stderr, "\t-h\tprint this help\n");
        fprintf (stderr, "\t print unquoted or quote getfacl string\n");
        exit(-1);
    }
    
    if (strcmp (argv[1], "-u") == 0) {
        printf("%s\n", unquote(argv[2]));
        exit(0);
    }
    if (strcmp (argv[1], "-q") == 0) {
        printf("%s\n", quote(argv[2]));
        exit(0);
    }
}
