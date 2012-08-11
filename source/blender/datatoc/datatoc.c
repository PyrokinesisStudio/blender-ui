/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* #define VERBOSE */

#define MAX2(x, y)               ( (x) > (y) ? (x) : (y) )
#define MAX3(x, y, z)             MAX2(MAX2((x), (y)), (z) )

static char *basename(char *string)
{
	char *lfslash, *lbslash;

	lfslash = strrchr(string, '/');
	lbslash = strrchr(string, '\\');
	if (lbslash) lbslash++;
	if (lfslash) lfslash++;

	return MAX3(string, lfslash, lbslash);
}

int main(int argc, char **argv)
{
	FILE *fpin,  *fpout;
	char sizest[256];
	long size;
	int i;

	if (argc < 2) {
		printf("Usage: datatoc <data_file_from> <data_file_to>\n");
		exit(1);
	}

	fpin = fopen(argv[1], "rb");
	if (!fpin) {
		printf("Unable to open input <%s>\n", argv[1]);
		exit(1);
	}

	argv[1] = basename(argv[1]);

	fseek(fpin, 0L,  SEEK_END);
	size = ftell(fpin);
	fseek(fpin, 0L,  SEEK_SET);

	if (argv[1][0] == '.') argv[1]++;

#ifdef VERBOSE
	printf("Making C file <%s>\n", argv[2]);
#endif

	for (i = 0; i < (int)strlen(argv[1]); i++)
		if (argv[1][i] == '.') argv[1][i] = '_';

	sprintf(sizest, "%d", (int)size);

#ifdef VERBOSE
	printf("Input filesize is %d, Output size should be %d\n",
	       (int)size,
	       (int)(((int)size) * 4 +
	             strlen("/* DataToC output of file <> */\n\n") +
	             strlen("char datatoc_[] = {\"") +
	             strlen("\"};\n") +
	             (strlen(argv[1]) * 3) +
	             strlen(sizest) +
	             strlen("int datatoc__size = ;\n") +
	             (((int)(size / 256) + 1) * 5)));
#endif

	fpout = fopen(argv[2], "w");
	if (!fpout) {
		fprintf(stderr, "Unable to open output <%s>\n", argv[2]);
		exit(1);
	}

	fprintf(fpout, "/* DataToC output of file <%s> */\n\n", argv[1]);
	fprintf(fpout, "int datatoc_%s_size = %s;\n", argv[1], sizest);
	fprintf(fpout, "char datatoc_%s[] = {\n", argv[1]);
	while (size--) {
		if (size % 32 == 31) {
			fprintf(fpout, "\n");
		}

		/* fprintf (fpout, "\\x%02x", getc(fpin)); */
		fprintf(fpout, "%3d,", getc(fpin));
	}

	fprintf(fpout, "\n};\n\n");

	fclose(fpin);
	fclose(fpout);
	return 0;
}
