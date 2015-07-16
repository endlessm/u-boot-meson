#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define READ_SIZE	(32 * 1024)
#define START_CHECK	(512)
#define CHECK_SIZE	(8 * 1024)
#define CHECK_HEAD	(0x1b0)
#define SUM_ADDR	(0x1b8)

int main(int argc, char *argv[])
{
	int i, fd;
	unsigned char spl[READ_SIZE];
	unsigned short *p, sum = 0;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if (read(fd, spl, READ_SIZE) < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	
	for (p = (unsigned short *) spl;
			p < (unsigned short *) (spl + CHECK_HEAD); p++)
		sum ^= *p;

	for (p = (unsigned short *) (spl + START_CHECK);
			p < (unsigned short *) (spl + CHECK_SIZE); p++)
		sum ^= *p;

	if (lseek(fd, SUM_ADDR, SEEK_SET) < 0) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	if (write(fd, &sum, sizeof(sum)) < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	if (close(fd) < 0) {
		perror("close");
		exit(EXIT_FAILURE);
	}

	printf("checksum 0x%x written at offset 0x%x (%s)\n", sum, SUM_ADDR, argv[1]);

	return EXIT_SUCCESS;
}
