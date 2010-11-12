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
#include <unistd.h>
#include <string.h>
#include <ftw.h>

#define AFS_VERSION "0.0.1"

static int verbose;
static struct total
{
	off64_t size;
	off_t files;
} t;

int fn(const char *fpath, const struct stat64 *sb, int typeflag, struct FTW *ftwbuf)
{
	// files only
	if (typeflag == FTW_F)
	{
		if (verbose)
			printf("%10llu %s\n", sb->st_size, fpath);
		t.size+=sb->st_size;
		t.files++;
	}
	return 0;
}

void print_usage(const char *script)
{
	fprintf(stderr, "Usage: %s [-hvV] <dir>\n", script);
}

int main(int argc, char **argv)
{
	char *path;
	int opt;
	
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
		path=argv[optind];

	if (verbose)
		printf("\nSize       File name\n");

	// start traversing
	if (nftw64(path, &fn, 1, FTW_PHYS) == -1)
	{
		perror(path);
		return 1;
	}
	
	// results
	printf("\nPath: %s\n", path);
	printf("Total: %llu bytes, %lu files.\n", t.size, t.files);
	printf("Average: %llu\n", (t.size/t.files));

	return 0;
}
