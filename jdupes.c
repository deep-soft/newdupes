/* jdupes (C) 2015-2023 Jody Bruchon <jody@jodybruchon.com>

	 Permission is hereby granted, free of charge, to any person
	 obtaining a copy of this software and associated documentation files
	 (the "Software"), to deal in the Software without restriction,
	 including without limitation the rights to use, copy, modify, merge,
	 publish, distribute, sublicense, and/or sell copies of the Software,
	 and to permit persons to whom the Software is furnished to do so,
	 subject to the following conditions:

	 The above copyright notice and this permission notice shall be
	 included in all copies or substantial portions of the Software.

	 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <libjodycode.h>
#include "libjodycode_check.h"

#include "likely_unlikely.h"
#include "jdupes.h"
#include "version.h"


/* File list head */
file_t *filelist_head = NULL;
file_t *filelist_tail = NULL;

/* Inode tree root */
inode_t *inodetree_root = NULL;

/* Program-wide behavior flags */
uint64_t flags;

/* Program name from argv[0] for usage info */
char *program_name;

/* Exit status; use exit() codes for setting this */
int exit_status = EXIT_SUCCESS;

/***** End definitions, begin code *****/


/***** Add new functions here *****/


char *store_path(char *file)
{
	if (unlikely(file == NULL)) jc_nullptr("store_path");
	fprintf(stderr, "store_path(\"%s\")\n", file);
	return (char *)malloc(strlen(file));
}


inode_t *get_inode(char *file, ino_t st_ino)
{
	if (unlikely(file == NULL)) jc_nullptr("get_inode");
	fprintf(stderr, "get_inode(\"%s\")\n", file);
	return (inode_t *)malloc(sizeof(inode_t));
}


file_t *load_item(file_t *file, char *item, int user_order)
{
	DIR *d;
	struct stat s;

	if (unlikely(file == NULL || item == NULL)) jc_nullptr("load_item");

	fprintf(stderr, "load_item(\"%s\", %d)\n", item, user_order);
	errno = 0;
	if (lstat(item, &s) != 0) goto error_lstat;
	if (S_ISDIR(s.st_mode)) {
		fprintf(stderr, "Is a directory: '%s'\n", item);
		file->type = FT_DIRECTORY;
	} else if (S_ISREG(s.st_mode)) {
		fprintf(stderr, "Is a regular file: '%s'\n", item);
		file->type = FT_REGULAR;
	} else if (S_ISLNK(s.st_mode)) {
		fprintf(stderr, "Is a symbolic link: '%s'\n", item);
		file->type = FT_SYMLINK;
	} else {
		fprintf(stderr, "File type not handled: '%s'\n", item);
		return NULL;
	}

	file->inode = get_inode(item, s.st_ino);
	if (file->inode == NULL) jc_oom("load_item:file->inode");
	file->d_name = store_path(item);
	if (file->d_name == NULL) jc_oom("load_item:file->d_name");

	return file;

error_lstat:
	fprintf(stderr, "error: stat failed on '%s': %s\n", item, strerror(errno));
	return NULL;
}


#ifdef UNICODE
int wmain(int argc, wchar_t **wargv)
#else
int main(int argc, char **argv)
#endif
{
	/* Verify libjodycode compatibility before going further */
	if (libjodycode_version_check(1, 0) != 0) exit(EXIT_FAILURE);

/* Windows buffers our stderr output; don't let it do that */
#ifdef ON_WINDOWS
	if (setvbuf(stderr, NULL, _IONBF, 0) != 0)
	  fprintf(stderr, "warning: setvbuf() failed\n");
#endif

#ifdef UNICODE
	/* Create a UTF-8 **argv from the wide version */
	static char **argv;
	int wa_err;
	argv = (char **)malloc(sizeof(char *) * (size_t)argc);
	if (!argv) jc_oom("main() unicode argv");
	wa_err = jc_widearg_to_argv(argc, wargv, argv);
	if (wa_err != 0) {
	  jc_print_error(wa_err);
	  exit(EXIT_FAILURE);
	}
	/* fix up __argv so getopt etc. don't crash */
	__argv = argv;
	jc_set_output_modes(0x0c);
#endif /* UNICODE */

	/* Is stderr a terminal? If not, we won't write progress to it */
#ifdef ON_WINDOWS
	if (!_isatty(_fileno(stderr))) SETFLAG(flags, F_HIDEPROGRESS);
#else
	if (!isatty(fileno(stderr))) SETFLAG(flags, F_HIDEPROGRESS);
#endif

	program_name = argv[0];
	if (argc == 1) goto show_help;
	filelist_head = (file_t *)calloc(1, sizeof(file_t));
	if (filelist_head == NULL) jc_oom("filelist_head");
	filelist_tail = filelist_head;

	/* Create a list of items to be processed  */
	for (int i = 1; i < argc; i++) {
		file_t *filelist_new;
//		fprintf(stderr, "param: %s\n", argv[i]);
		filelist_new = load_item(filelist_tail, argv[i], i);
		if (filelist_new == NULL) continue;
		filelist_tail = filelist_new;
	}

	exit(exit_status);

show_help:
	fprintf(stderr, "jdupes %s (%s)\n", VER, VERDATE);
	fprintf(stderr, "usage: %s directory-or-file [dirs/files ...]\n", program_name);
	exit(EXIT_FAILURE);
}
