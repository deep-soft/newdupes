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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <libjodycode.h>
#include "libjodycode_check.h"

#include "likely_unlikely.h"
#include "jdupes.h"
#include "version.h"


/* Program-wide behavior flags */
uint64_t flags;

/* Program name from argv[0] for usage info */
char *program_name;

/* Exit status; use exit() codes for setting this */
int exit_status = EXIT_SUCCESS;

/***** End definitions, begin code *****/

/***** Add new functions here *****/


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
	/* Scan duplicates here */
	for (int i = 1; i < argc; i++) {
		printf("file: %s\n", argv[i]);
	}

	exit(exit_status);

show_help:
	fprintf(stderr, "jdupes %s (%s)\n", VER, VERDATE);
	fprintf(stderr, "usage: %s directory-or-file [dirs/files ...]\n", program_name);
	exit(EXIT_FAILURE);
}
