#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <ctime>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <iostream>

using namespace std;

#define MAX_LEN 128

void die(const char *message, int status = -1) {
	printf("%s\n", message);
	_exit(status);
}

void semop_buf(int semid, short op, int num) {
	cout << "pid = " << getpid() << ", semid = " << semid << ", op = " << op << ", num = " << num << endl;
	struct sembuf buf;

	buf.sem_op = op;
	buf.sem_flg = 0;
	buf.sem_num = num;

	if (semop(semid, &buf, 1) < 0) {
		cout << "Can't wait for condition" << endl;
		_exit(EXIT_FAILURE);
	}

	return;

	if (op > 0) {
		cout << "Set condition" << endl;
	}
	else if (op < 0) {
		cout << "Condition is present" << endl;
	}
	else {
		cout << "Condition!" << endl;
	}
}

int main(int argc, char *argv[]) {
	int fd[2], result;
	size_t size;
	char s[MAX_LEN];
	
	if (argc < 2) {
		die("Укажите слово для передачи");
	}

	if(pipe(fd) < 0) {
		die("Can’t create pipe");
	}

	int semid;
	char pathname[] = "lab6_4.cpp";
	key_t key;

	if ((key = ftok(pathname, 0)) < 0) {
		cout << "Не удалось сгенерировать ключ" << endl;
		return EXIT_FAILURE;
	}
	
	printf("fd: %d, %d\n", fd[0], fd[1]);


	if ((semid = semget(key, 2, 0666 | IPC_CREAT)) < 0) {
		cout << "Не удалось получить semid" << endl;
		return EXIT_FAILURE;
	}

	semop_buf(semid, 1, 0);
	semop_buf(semid, 1, 1);

	switch (fork()) {
		case -1:
			die("Can’t fork child");
			break;

		case 0:
			semop_buf(semid, 0, 0);

			size = read(fd[0], s, MAX_LEN);

			if (size < 0) {
				die("Can’t read string");
			}
			s[size] = 0;

			cout << "Прочитано: '" << s << "'" << endl;

			sprintf(s, "%s%d", s, (int)time(NULL));


			size = write(fd[1], s, strlen(s));
			semop_buf(semid, -1, 1);

			if (size != strlen(s)) {
				die("Не удалось передать родительскому процессу");
			}
			close(fd[0]);
			close(fd[1]);
			break;

		default:
			size = write(fd[1], argv[1], strlen(argv[1]));
			semop_buf(semid, -1, 0);

			if (size != strlen(argv[1])) {
				die("Can’t write all string");
			}

			semop_buf(semid, 0, 1);
			size = read(fd[0], s, MAX_LEN);
			s[size] = 0;

			if (size < 0) {
				die("Не удалось получить от дочернего процесса");
			}

			cout << "Прочитано: '" << s << "'" << endl;

			printf("Parent exit\n");
			close(fd[0]);
			close(fd[1]);

			break;
	}

	return 0;
}
