#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <string>

using namespace std;

typedef vector < int > v_int;

#define MTYPE_PING 1
#define MTYPE_PONG 2

struct buf {
	long mtype;
	char mtext[5];
};

struct DATA_AB {
	long mtype = MTYPE_PING;
	int a;
	int b;
};

struct DATA_AB_OP {
	long mtype = MTYPE_PONG;
	int a;
	int b;
	int op_sum;
	int op_multi;
};

ostream& operator<<(ostream& out, const struct DATA_AB &s) {
	return out << "DATA_AB(mtype = " << s.mtype << ", a = " << s.a << ", b = " << s.b << ")";
}

ostream& operator<<(ostream& out, const struct DATA_AB_OP &s) {
	return out << "DATA_AB_OP(mtype = " << s.mtype << ", a = " << s.a << ", b = " << s.b << ", op: " << s.op_sum << ", " << s.op_multi << ")";
}

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

int init_msg(const char *filepath) {
	int msqid = msgget(init_key(filepath), 0666 | IPC_CREAT);

	if (msqid == -1) {
		perror("msgget");
		die("Не удалось инициализировать очередь сообщений.");
	}


	cout << "msqid = " << msqid << endl;
	return msqid;
}

int main_server(const char *filepath) {
	int msqid = init_msg(filepath);

	for (;;) {
		struct DATA_AB s;
		struct DATA_AB_OP ss;

		if (msgrcv(msqid, &s, sizeof(s) - sizeof(long), MTYPE_PING, 0) == -1) {
			sleep(1);
			continue;
		}

		cout << "Принято: " << s << endl;
		ss.a = s.a;
		ss.b = s.b;
		ss.op_sum = ss.a + ss.b;
		ss.op_multi = ss.a * ss.b;

		cout << "Отравка: " << ss << endl;

		if (msgsnd(msqid, (void*)&ss, sizeof(ss) - sizeof(long), 0) == -1) {
			perror("msgsnd");
			die("Ошибка отправки сообщения клиенту.");
		}		
	}
}

int main_client(const char *filepath) {
	int msqid = init_msg(filepath);

	for (;;) {
		struct DATA_AB s;
		struct DATA_AB_OP ss;

		cout << "Введите 2 целых числа: ";
		cin >> s.a >> s.b;
		cout << "Введено: " << s << endl;

		if (msgsnd(msqid, (void*)&s, sizeof(s) - sizeof(long), 0) == -1) {
			perror("msgsnd");
			die("Ошибка отправки сообщения серверу.");
		}

		if (msgrcv(msqid, &ss, sizeof(ss) - sizeof(long), MTYPE_PONG, 0) == -1) {
			die("error");
		}

		cout << "Результат: " << ss << endl;
		cout << "Сумма: " << ss.op_sum << endl;
		cout << "Произведение: " << ss.op_multi << endl;
	}

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