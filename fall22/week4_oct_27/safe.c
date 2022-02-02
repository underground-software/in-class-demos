#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>

int main(int argc, char **argv)
{
	if(2 != argc)
		errx(1, "usage %s <filename>", argv[0]);
	int fd;
	if(0 > (fd = open(argv[1], O_RDONLY)))
		err(1, "couldn't open %s", argv[1]);
	struct stat stat_data;
	if(0 > fstat(fd, &stat_data))
		err(1, "unable to stat %s", argv[1]);
	if(0 == stat_data.st_uid)
		errx(1, "File %s is owned by root", argv[1]);
	char buf[32];
	snprintf(buf, sizeof buf, "/proc/self/fd/%i", fd);
	execl("/bin/cat", "cat", buf, NULL);
}
