/*
 * varskel -- apply the data volume skeleton (etc/var.skel) from user land.
 *
 *   varskel [-q]
 *
 * init does this at boot (src/init.c, mount_writable), but it stops at the first entry that
 * fails -- a mkdir that already exists, a chown the filesystem refuses -- and everything after it
 * is never made. On the vili port that left /private/var/tmp missing and put the ByHost migration
 * records where iOS would not look. This does the same work without ever giving up, and says what
 * failed, so the panel shows it.
 *
 * Line format (tab separated), as produced for /redwood/etc/var.skel:
 *   d  MODE UID GID PATH
 *   l  TARGET LINK
 *   f  MODE UID GID PATH SOURCE-IN-THE-IMAGE
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX 16384

static char body[MAX];

static long number(const char * s, int base)
{
	return strtol(s, NULL, base);
}

static int copy_file(const char * from, const char * to, mode_t mode, uid_t uid, gid_t gid)
{
	int in = open(from, O_RDONLY, 0);
	if (in < 0) {
		return -1;
	}
	int out = open(to, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (out < 0) {
		close(in);
		return -1;
	}
	char buf[8192];
	ssize_t n;
	while ((n = read(in, buf, sizeof(buf))) > 0) {
		ssize_t at = 0;
		while (at < n) {
			ssize_t w = write(out, buf + at, (size_t) (n - at));
			if (w <= 0) {
				close(in);
				close(out);
				return -1;
			}
			at += w;
		}
	}
	close(in);
	close(out);
	chmod(to, mode);
	chown(to, uid, gid);
	return 0;
}

int
main(int argc, char ** argv)
{
	int quiet = argc > 1 && strcmp(argv[1], "-q") == 0;
	const char * path = "/redwood/etc/var.skel";
	int dirs = 0, files = 0, links = 0, bad = 0;
	int in = open(path, O_RDONLY, 0);
	if (in < 0) {
		printf("varskel: %s: cannot open\n", path);
		return 1;
	}
	size_t total = 0;
	ssize_t n;
	while (total < sizeof(body) - 1 && (n = read(in, body + total, sizeof(body) - 1 - total)) > 0) {
		total += (size_t) n;
	}
	close(in);
	body[total] = 0;

	for (char * line = body; *line; ) {
		char * end = strchr(line, '\n');
		char * f[6];
		int count = 0;
		if (end != NULL) {
			*end = 0;
		}
		for (char * at = line; count < 6; ) {
			f[count++] = at;
			char * tab = strchr(at, '\t');
			if (tab == NULL) {
				break;
			}
			*tab = 0;
			at = tab + 1;
		}
		if (count == 5 && f[0][0] == 'd') {
			mode_t mode = (mode_t) number(f[1], 8);
			if (mkdir(f[4], mode) != 0 && errno != EEXIST) {
				printf("varskel: mkdir %s: %s\n", f[4], strerror(errno));
				bad++;
			} else {
				dirs++;
			}
			chmod(f[4], mode);
			chown(f[4], (uid_t) number(f[2], 10), (gid_t) number(f[3], 10));
		} else if (count == 3 && f[0][0] == 'l') {
			if (symlink(f[1], f[2]) == 0) {
				links++;
			}
		} else if (count == 6 && f[0][0] == 'f') {
			if (copy_file(f[5], f[4], (mode_t) number(f[1], 8), (uid_t) number(f[2], 10),
			    (gid_t) number(f[3], 10)) == 0) {
				files++;
			} else {
				printf("varskel: %s: %s\n", f[4], strerror(errno));
				bad++;
			}
		}
		if (end == NULL) {
			break;
		}
		line = end + 1;
	}
	if (!quiet) {
		printf("varskel: %d directories, %d files, %d links, %d failed\n", dirs, files, links, bad);
	}
	return bad == 0 ? 0 : 1;
}
