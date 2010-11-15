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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fts.h>

#define AFS_VERSION "0.0.2"

typedef struct _total
{
	off64_t bytes;
	off64_t files;
} TOTAL;

void print_usage(const char *script)
{
	fprintf(stderr, "Usage: %s [-hvV] <dir>\n", script);
}

int main(int argc, char **argv)
{
	char **path;
	int opt;
	FTS *ftsd;
	FTSENT *ent;
	struct stat64 s;
	char verbose=0;
	TOTAL t;
	
	// handle arguments
	while ((opt = getopt(argc, argv, "+hvV")) != -1)
		switch (opt)
		{
			case 'v':
				verbose=1;
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
		path=(char **) malloc(sizeof(char *));
		path[0]=argv[optind];
	}

	if (verbose)
		printf("\nSize       File name\n");

	// start traversing
	t.files=t.bytes=0;
	ftsd = fts_open(path, FTS_PHYSICAL|FTS_NOSTAT|FTS_NOCHDIR, NULL);
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
				}
			}
			else
				perror(ent->fts_path);
	fts_close(ftsd);
	if (errno != 0)
	{
		perror("");
		return 1;
	}

	// results
	printf("\nPath: %s\n", path[0]);
	printf("Total: %llu bytes, %llu files.\n", t.bytes, t.files);
	if (t.files>0)
		printf("Average: %llu\n", (t.bytes/t.files));
	free(path);

	return 0;
}
