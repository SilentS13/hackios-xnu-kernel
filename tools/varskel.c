/*
 * varskel -- put the data volume in order, from user land.
 *
 *   varskel [-q]
 *
 * Two jobs, because init does neither reliably on this port:
 *
 * 1. /private/var. init mounts a ramfs there (src/init.c, mount_writable), but the mount point
 *    does not exist on this root and it does not create it: the mount fails with ENOENT and the
 *    whole skeleton below is then applied to a directory that is not there. One boot of the vili
 *    port showed "df" with no /private/var at all and 148 skeleton entries failing with
 *    "No such file or directory" -- so iOS ran with no data volume, no /tmp, and no ByHost
 *    migration record. This creates the mount point and mounts the ramfs.
 *
 * 2. etc/var.skel. init stops at the first entry that fails; this does not, and says what failed.
 *
 * Line format (tab separated): d MODE UID GID PATH | l TARGET LINK | f MODE UID GID PATH SOURCE
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/mount.h>
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

/* Make sure there is a data volume, and say clearly what happened. */
static void
data_volume(void)
{
	struct stat st;

	/*
	 * The root of this boot is a RAM disk (rd=md0) that XNU mounts read-only, and the image does
	 * not carry /private/var: without a mount point the ramfs mount below cannot work, so try to
	 * remount / writable first. It is memory, so nothing on the phone's flash is at stake.
	 */
	{
		struct statfs sfs;

		if (statfs("/", &sfs) == 0 && (sfs.f_flags & MNT_RDONLY) != 0) {
			if (mount(sfs.f_fstypename, "/", MNT_UPDATE, NULL) == 0) {
				printf("varskel: remounted / read-write (%s)\n", sfs.f_fstypename);
			} else {
				printf("varskel: remount / read-write: %s\n", strerror(errno));
			}
		}
	}
	if (stat("/private/var", &st) != 0) {
		if (stat("/private", &st) != 0) {
			if (mkdir("/private", 0755) != 0 && errno != EEXIST) {
				printf("varskel: mkdir /private: %s\n", strerror(errno));
				return;
			}
		}
		if (mkdir("/private/var", 0755) != 0 && errno != EEXIST) {
			printf("varskel: mkdir /private/var: %s\n", strerror(errno));
			return;
		}
		printf("varskel: made the mount point /private/var\n");
	}
	if (mount("ramfs", "/private/var", 0, NULL) == 0) {
		printf("varskel: mounted a memory filesystem on /private/var\n");
	} else {
		printf("varskel: mount /private/var: %s\n", strerror(errno));
	}
}

int
main(int argc, char ** argv)
{
	int quiet = argc > 1 && strcmp(argv[1], "-q") == 0;
	const char * path = "/redwood/etc/var.skel";
	int dirs = 0, files = 0, links = 0, bad = 0;

	data_volume();

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
				if (!quiet) {
					printf("varskel: mkdir %s: %s\n", f[4], strerror(errno));
				}
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
				if (!quiet) {
					printf("varskel: %s: %s\n", f[4], strerror(errno));
				}
				bad++;
			}
		}
		if (end == NULL) {
			break;
		}
		line = end + 1;
	}
	printf("varskel: %d directories, %d files, %d links, %d failed\n", dirs, files, links, bad);
	return bad == 0 ? 0 : 1;
}
