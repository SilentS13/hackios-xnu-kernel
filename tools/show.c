/*
 * show -- print the beginning of a file, because iOS has no cat.
 *
 *   show [-n LINES] FILE
 *
 * Written for the vili port: spindump can sample a process on this kernel but cannot write its
 * report to /dev/console (ENOTSUP) and stops to ask on stdin where to put it. With -o FILE it
 * writes the report, and this program puts it on the panel.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char ** argv)
{
	const char * path = NULL;
	long max = 0;
	char line[4096];

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
			max = strtol(argv[++i], NULL, 10);
		} else if (path == NULL) {
			path = argv[i];
		}
	}
	if (path == NULL) {
		fprintf(stderr, "usage: show [-n LINES] FILE\n");
		return 2;
	}
	FILE * f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "show: %s: cannot open\n", path);
		return 1;
	}
	for (long n = 0; max == 0 || n < max; n++) {
		if (fgets(line, sizeof(line), f) == NULL) {
			break;
		}
		fputs(line, stdout);
	}
	fclose(f);
	return 0;
}
