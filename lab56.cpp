#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <string>

const size_t N = 2;
const size_t SEM_N = 2;

using namespace std;

typedef vector < int > v_int;

#define SEM_LOCK 0
#define SEM_READ 1

void die(const char *message, int code = EXIT_FAILURE) {
	cout << message << endl;
	_exit(code);
}

key_t init_key(const char *filepath) {
	key_t key = ftok(filepath, 0);

	if (key == -1) {
		die("Не удалось сгенерировать ключ.");
	}

	return key;
}

int* init_data_shm(const char *filepath, size_t n) {
	key_t key = init_key(filepath);

	bool is_new = true;
	int shmid = shmget(key, n * sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);

	if (shmid == -1) {
		if (errno != EEXIST) {
			die("Не удалось создать Shared Memory");
		}

		shmid = shmget(key, n * sizeof(int), 0);

		if (shmid == -1) {
			die("Не удалось найти Shared Memory");
		}

		is_new = false;
	}

	int* data = (int *)shmat(shmid, NULL, 0);

	if (data == (int *)-1) {
		die("Не удалось подключиться к Shared Memory");
	}

	if (is_new) {
		memset(data, 0, n * sizeof(int));
	}
}

void rm_data_shm(int *data) {
	if (shmdt(data) == -1) {
		die("Не удалось отключиться от Shared Memory");
	}
}



int semop_buf(int semid, int op, int num) {
	// cout << "call semop_buf(" << semid << ", " << op << ", " << num << ") from pid " << getpid() << endl;

	struct sembuf buf;

	buf.sem_op = op;
	buf.sem_flg = 0;
	buf.sem_num = num;

	if (semop(semid, &buf, 1) == -1) {
		cout << "Не удалось дождаться семафора" << endl;
		return EXIT_FAILURE;
	}

	// cout << "call done" << endl;
}

int init_sem(const char *filepath, size_t n) {
	key_t key = init_key(filepath);

	int semid = semget(key, n, 0666 | IPC_CREAT);

	if (semid == -1) {
		die("Не удалось получить ID набора семафоров");
	}

	return semid;
}

v_int string2v_int(const string &s) {
	v_int a;

	istringstream iss(s);

	for (int t; iss >> t;) {
		a.push_back(t);
	}

	return a;
}

ostream& operator<<(ostream &out, const v_int &a) {
	for (auto it = a.begin(); it != a.end(); ++it) {
		out << *it << " ";
	}

	return out;
}


void dump_buffer(const int *data) {
	cout << "Количество элементов в буфере: " << data[N] << endl;
	cout << "Буфер: ";

	for (size_t i = 0; i < data[N]; ++i) {
		cout << data[i] << " ";
	}

	cout << endl;
}

int main_server(const char *filepath) {
	int *data = init_data_shm(filepath, N + 1);
	int semid = init_sem(filepath, SEM_N);

	for (;;) {
		cout << "Ожидание изменения буфера..." << endl;
		semop_buf(semid, -1, SEM_LOCK);
		semop_buf(semid, 0, SEM_READ);

		cout << "Прочитанный буфер из клиентского приложения" << endl;
		dump_buffer(data);
		data[N] = 0;

		cout << "Обработка буфера..." << endl;
		sleep(3);
		cout << "...завершена" << endl;

		semop_buf(semid, 1, SEM_READ);
	}

	rm_data_shm(data);
}

int main_client(const char *filepath) {
	int *data = init_data_shm(filepath, N + 1);
	int semid = init_sem(filepath, SEM_N);

	for (;;) {
		cout << "Массив: ";

		string line;

		getline(cin, line);

		v_int a = string2v_int(line);

		if (a.size() == 0) {
			cout << "Нечего передавать серверной части. Выход." << endl;
			continue;
		}

		cout << "Введенный массив: " << a << endl;

		for (int len = min(a.size(), N); len > 0; len = min(a.size(), N)) {
			cout << "Ожидание разблокировки буфера для передачи массива..." << endl;
			semop_buf(semid, 0, SEM_LOCK);
			data[N] = len;

			for (size_t i = 0; i < len; ++i) {
				data[i] = a[i];
			}

			dump_buffer(data);
			a.erase(a.begin(), a.begin() + len);
			semop_buf(semid, 1, SEM_LOCK);
			semop_buf(semid, -1, SEM_READ);
		}
	}

	rm_data_shm(data);

	return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "Укажите роль приложения (client, server)." << endl;
		return EXIT_FAILURE;
	}

	if (!strcmp(argv[1], "client")) {
		return main_client(argv[0]);
	}

	if (!strcmp(argv[1], "server")) {
		return main_server(argv[0]);
	}

	cout << "Некорректная роль приложения." << endl;
	return EXIT_FAILURE;
}