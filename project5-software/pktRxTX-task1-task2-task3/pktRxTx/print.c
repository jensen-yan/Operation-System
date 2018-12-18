/**
 * (C) Copyright 2013 Faraday Technology
 * BingYao Luo <bjluo@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif
/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void
print_hex_ascii_line(const unsigned char * payload, int len, int offset, FILE * fp)
{

	int i;
	int gap;
	const unsigned char *ch;

	/* offset */
	if (fp)
		fprintf(fp, "%05x   ", offset);
	else
		printf("%05x   ", offset);

	/* hex */
	ch = payload;
	for (i = 0; i < len; i++) {
		if (fp)
			fprintf(fp, "%02x ", *ch);
		else
			printf("%02x ", *ch);

		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7) {
			if (fp)
				fprintf(fp, "  ");
			else
				printf(" ");
		}
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8) {
		if (fp)
			fprintf(fp, "  ");
		else
			printf(" ");
	}

	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			if (fp)
				fprintf(fp, "   ");
			else
				printf("   ");
		}
	}
#if 0
	printf("   ");

	/* ascii (if printable) */
	ch = payload;
	for (i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}
#endif
	if (fp)
		fprintf(fp, "\n");
	else
		printf("\n");

	return;
}

/*
 * print packet payload data (avoid printing binary data)
 */
void print_payload(const unsigned char * payload, int len, FILE * fp)
{

	int len_rem = len;
	int line_width = 16;	/* number of bytes per line */
	int line_len;
	int offset = 0;		/* zero-based offset counter */
	const unsigned char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset, fp);
		return;
	}

	/* data spans multiple lines */
	for (;;) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset, fp);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset, fp);
			break;
		}
	}

	return;
}
