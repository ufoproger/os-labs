#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

int main() {
	int fd = open("test.txt", O_CREAT | O_APPEND | O_RDWR, S_IRWXU | S_IRWXG);

	if (fd == -1) {
		printf("Ошибка открытия файла!\n");
		return -1;
	}

	printf("fd = %d\n", fd);

	ssize_t res = 0;

	switch (fork()) {
		case -1:
			printf("ERROR #%d\n", errno);
			return -1;

		case 0:
			printf("Дочерний процесс засыпает\n");
			sleep(1);
			printf("Дочерний процесс просыпается\n");
			res = write(fd, "child\n", 6);
			printf("Записано данных в файл (by child): %ld\n", res);

			break;

		default:
			printf("Родительский процесс закрывает fd = %d и засыпает\n", fd);
			close(fd);
			res = write(fd, "parent\n", 6);
			printf("Записано данных в файл (by parent): %ld\n", res);
			sleep(3);

	}

	return 0;
}