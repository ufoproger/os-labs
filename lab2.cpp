#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <errno.h>

int main(int argc, char *argv[], char *envp[]) {
	printf("Количество аргументов: %d\n\n", argc);
	printf("Аргументы:\n");

	for (int i = 0; i < argc; ++i) {
		printf("\tАргумент № %d = \"%s\"\n", i, argv[i]);
	}

	printf("Переменные среды:\n");

	for (char **s = envp; *s; ++s) {
		printf("Переменная: %s\n", *s);
	}

	if (argc > 2 && !strcmp(argv[1], "cat")) {
		int res = execlp("/bin/cat", "/bin/cat", argv[2], NULL);
		// int res = execl("lab1", "/bin/cat", "/etc/passwd", NULL);
		printf("error res = %d, %d\n", res, errno);
	}

	return 0;
}