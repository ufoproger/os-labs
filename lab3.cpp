#include <sys/types.h>
#include <ctime>
#include <unistd.h>
#include <stdio.h>
#include <cstring>

#define MAX_LEN 128

void die(const char *message, int status = -1) {
	printf("%s\n", message);
	_exit(status);
}

int main(int argc, char *argv[]) {
	int fd[2], fd2[2], result;
	size_t size;
	char s[MAX_LEN];
	
	if (argc < 2) {
		die("Укажите слово для передачи");
	}

	if(pipe(fd) < 0) {
		die("Can’t create pipe");
	}
	
	if(pipe(fd2) < 0) {
		die("Can’t create pipe2");
	}

	printf("fd: %d, %d\n", fd[0], fd[1]);

	switch (fork()) {
		case -1:
			die("Can’t fork child");
			break;

		case 0:
			close(fd[1]);
			close(fd2[0]);
			size = read(fd[0], s, MAX_LEN);

			if (size < 0) {
				die("Can’t read string");
			}

			printf("Прочитано: %s\n", s);

			sprintf(s, "%s%d", s, (int)time(NULL));

			size = write(fd2[1], s, strlen(s));

			if (size != strlen(s)) {
				die("Не удалось передать родительскому процессу");
			}

			close(fd[0]);
			close(fd2[1]);
			break;

		default:
			close(fd[0]);
			close(fd2[1]);

			size = write(fd[1], argv[1], strlen(argv[1]));
			
			if (size != strlen(argv[1])) {
				die("Can’t write all string");
			}

			size = read(fd2[0], s, MAX_LEN);

			if (size < 0) {
				die("Не удалось получить от дочернего процесса");
			}

			printf("Прочитано: %s\n", s);

			printf("Parent exit\n");
			close(fd[1]);
			close(fd2[0]);

			break;
	}

	return 0;
}