#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <iostream>
#include <string>

using namespace std;

int main() {
	int semid;
	string pathname = "lab5.cpp";
	key_t key;

	if ((key = ftok(pathname.c_str(), 0)) < 0) {
		cout << "Не удалось сгенерировать ключ" << endl;
		return EXIT_FAILURE;
	}

	if ((semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
		cout << "Не удалось получить semid" << endl;
		return EXIT_FAILURE;
	}

	struct sembuf buf;
	buf.sem_op = -5;
	buf.sem_flg = 0;
	buf.sem_num = 0;

	if (semop(semid, &buf, 1) < 0) {
		cout << "Can't wait for condition" << endl;
		return EXIT_FAILURE;
	}

	cout << "Condition is present" << endl;

	return 0;
}