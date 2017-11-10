#include <semaphore.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

const size_t N = 2;

int *data;

typedef vector < int > v_int;

// producer - производитель,
// consumer - потребитель
sem_t mutex_producer, mutex_consumer;

void die(const char *message, int status = EXIT_FAILURE) {
	cout << message << endl;
	exit(status);
}

v_int string2v_int(const string &s) {
	istringstream iss(s);
	v_int a;

	for (int t; iss >> t;) {
		a.push_back(t);
	}

	return a;
}

ostream& operator<<(ostream &out, const v_int &a) {
	for (auto item: a) {
		out << item << " ";
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

void *consumer_thread(void *arg) {
	for (;;) {
		sem_wait(&mutex_consumer);

		cout << endl;
		cout << "CONSUMER:" << endl;
		cout << "=========" << endl;
		cout << "Прочитанный буфер из клиентского приложения" << endl;

		dump_buffer(data);
		
		cout << "Обработка буфера..." << endl;

		for (size_t i = 0; i < data[N]; ++i) {
			data[i] *= 2;
		}

		sleep(3);
		cout << "...завершена" << endl;

		sem_post(&mutex_producer);
	}

	return NULL;
}

int main()
{
	data = new int[N + 1];

	if (data == NULL) {
		die("Не удалось выделить память для массива");
	}

	data[N] = 0;

	sem_init(&mutex_producer, 0, 1);
	sem_init(&mutex_consumer, 0, 0);

	pthread_t consumer;
	int result = pthread_create(&consumer, (pthread_attr_t *)NULL, consumer_thread, NULL);

	if(result != 0) {
		die("Error on thread create");
	}

	for (;;) {
		cout << "Массив: ";

		string line;

		getline(cin, line);

		v_int a = string2v_int(line);

		if (!a.size()) {
			cout << "Нечего передавать серверной части. Выход." << endl;
			pthread_detach(consumer);
			break;
		}

		cout << "Введенный массив: " << a << endl;

		for (int len = min(a.size(), N); len > 0; len = min(a.size(), N)) {
			cout << endl;
			cout << "PRODUCER:" << endl;
			cout << "=========" << endl;

			sem_wait(&mutex_producer);

			data[N] = len;

			for (size_t i = 0; i < len; ++i) {
				data[i] = a[i];
			}

			dump_buffer(data);
			a.erase(a.begin(), a.begin() + len);

			sem_post(&mutex_consumer);
			sem_wait(&mutex_producer);

			cout << endl;
			cout << "PRODUCER:" << endl;
			cout << "=========" << endl;

			dump_buffer(data);

			sem_post(&mutex_producer);
		}
	}

	pthread_join(consumer, (void **)NULL);

	return 0;
}