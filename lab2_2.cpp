#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

int main(int argc, char *argv[]) {
	printf("Программа запущена.\n");

	printf("argc = %d, argv:", argc);

	for (int i = 0; i < argc; ++i) {
		printf(" \"%s\"", argv[i]);
	}

	printf("\n");
	printf("pid = %d\n", getpid());

	int fd = open("test.txt", O_CREAT | O_APPEND | O_RDWR, S_IRWXU | S_IRWXG);

	printf("fd = %d\n", fd);

	if (argc > 1 && !strcmp(argv[1], "call_exec")) {
		printf("call_exec\n");
		char s_fd[10];
		sprintf(s_fd, "%d", fd);

		execlp(argv[0], argv[0], "fd", s_fd, NULL);
	}


	if (argc > 2 && !strcmp(argv[1], "fd")) {
		sscanf(argv[2], "%d", &fd);
		printf("Установка fd = %d\n", fd);

	}

	ssize_t res = write(fd, "child\n", 6);
	printf("res = %ld\n", res);

	printf("Программа завершается.\n");

	return 0;
}