/*
AFS - Average File Size
A simple tool that calculates average size for all files in a given path.

Copyright (C) 2010, Vytautas Krakauskas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fts.h>
#include <math.h>

#define AFS_VERSION "0.0.3"

//base 10 exponents for 64bits is [0-19]. Additional possition is for empty files
#define DISTR_SIZE 21
const char units[DISTR_SIZE][11]={"empty",
"1-9  B", "10-99  B", "100-999  B",
"1-9 KB", "10-99 KB", "100-999 KB",
"1-9 MB", "10-99 MB", "100-999 MB",
"1-9 GB", "10-99 GB", "100-999 GB",
"1-9 TB", "10-99 TB", "100-999 TB",
"1-9 PB", "10-99 PB", "100-999 PB",
"1-9 EB", "10-99 EB"};

typedef struct _minmax
{
	char min, max;
} MINMAX;

typedef struct _total
{
	off64_t bytes;
	off64_t files;
} TOTAL;

void print_usage(const char *script)
{
	fprintf(stderr, "Usage: %s [-hvdV] <dir>\n", script);
}

int main(int argc, char **argv)
{
	char **path;
	int opt;
	char verbose=0;
	char distrib=0;
	
	// handle arguments
	while ((opt = getopt(argc, argv, "+hvdV")) != -1)
		switch (opt)
		{
			case 'v':
				verbose=1;
				break;
			case 'd':
				distrib=1;
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			case 'V':
				printf("Average File Size v%s \n", AFS_VERSION);
				return 0;
			default:
				print_usage(argv[0]);
				return 1;
		}
	if (optind >= argc)
	{
		print_usage(argv[0]);
		return 1;
	}
	else
	{
		path=(char **) malloc(sizeof(char *)*2);
		path[0]=argv[optind];
		path[1]=NULL;
	}

	if (verbose)
		printf("\nSize       File name\n");

	// start traversing
	FTS *ftsd;
	FTSENT *ent;
	struct stat64 s;
	TOTAL t;
	off64_t distr[DISTR_SIZE];
	char idx;
	MINMAX mm;

	mm.min=DISTR_SIZE;
	mm.max=0;
	bzero(&distr, sizeof(distr));
	t.files=t.bytes=0;
	ftsd = fts_open(path, FTS_PHYSICAL|FTS_NOSTAT|FTS_NOCHDIR, NULL);
	if (errno != 0)
	{
		perror("");
		return 1;
	}
	while ((ent = fts_read(ftsd)) != NULL)
		if (ent->fts_info == FTS_NSOK)
			if (lstat64(ent->fts_path, &s) == 0)
			{
				if (S_ISREG(s.st_mode))
				{
					if (verbose)
						printf("%10llu %s\n", s.st_size, ent->fts_path);
					t.bytes+=s.st_size;
					t.files++;

					//File size distribution
					if (distrib)
					{
						if (s.st_size > 0)
							idx=log10l(s.st_size)+1;
						else
							idx=0;
						distr[idx]++;
						if (mm.max < idx) mm.max=idx;
						if (mm.min > idx) mm.min=idx;
					}
				}
			}
			else
				perror(ent->fts_path);
	if (errno != 0)
	{
		perror("");
		return 1;
	}
	if (fts_close(ftsd) != 0) perror("FTS_CLOSE");

	// results
	printf("\nPath: %s\n", path[0]);
	printf("Total: %llu bytes, %llu files.\n", t.bytes, t.files);
	if (t.files>0)
		printf("Average: %llu\n", (t.bytes/t.files));
	free(path);

	if (distrib)
	{
		printf("File size distribution:\n");
		int i;
		for (i=mm.min; i<=mm.max; i++)
			printf("%10s: %4.1f%% (%llu files)\n", units[i], (float) (distr[i]*100)/t.files, distr[i]);
	}
	return 0;
}
