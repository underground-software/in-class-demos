#include <fcntl.h>
#include <stdio.h>
#include <err.h>

int main(int argc, char **argv)
{
	if(3 != argc)
		errx(1, "Usage: %s file1 file2", argv[0]);
	for(;;)
		renameat2(AT_FDCWD, argv[1], AT_FDCWD, argv[2], RENAME_EXCHANGE);
}
