#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include <cstring>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const string pathname = "prog1.c";

bool file_exists(string filepath) {
	ifstream f(filepath.c_str());

	return f.good();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "Укажите общий файл!" << endl;
		return EXIT_FAILURE;
	}

	if (!file_exists(argv[1])) {
		cout << "Файл \"" << argv[1] << "\" недоступен!" << endl;
		return EXIT_FAILURE;
	}

	int program_id = 0;

	if (string(argv[0]) != "./a.out") {
		program_id = 1;
	}

	cout << "Сейчас запущена программа # " << (program_id + 1) << endl;

	int *array;
	int shmid;
	bool is_new = true;

	key_t key;

	if ((key = ftok(argv[1], 0)) < 0) {
		cout << "Не удалось сгенерировать ключ" << endl;
		return EXIT_FAILURE;
	}

	if ((shmid = shmget(key, 3 * sizeof(int), 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
		if (errno != EEXIST) {
			cout << "Не удалось создать Shared Memory" << endl;
			return EXIT_FAILURE;
		}

		if ((shmid = shmget(key, 3 * sizeof(int), 0)) < 0) {
			cout << "Не удалось найти Shared Memory" << endl;
		}

		is_new = false;
	}

	if ((array = (int *)shmat(shmid, NULL, 0)) == (int *)(-1)) {
		cout << "Не удалось подключиться к Shared Memory" << endl;
		return EXIT_FAILURE;
	}

	if (is_new) {
		cout << "is new" << endl;
		memset(array, 0, 3 * sizeof(int));
	}

	++array[program_id];
	++array[2];

	cout << "Статистика запуска програм: " << endl;
	cout << "\t# 1: " << array[0] << " (раз)" << endl;
	cout << "\t# 2: " << array[1] << " (раз)" << endl;
	cout << "\t# ИТОГО: " << array[2] << " (раз)" << endl;

	if (shmdt(array) < 0) {
		cout << "Не удалось отключиться от Shared Memory" << endl;
		return EXIT_FAILURE;
	}

	return 0;
}