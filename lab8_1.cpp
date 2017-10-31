#include <iostream>
#include <sstream>

#include <unistd.h>
#include <signal.h>

using namespace std;

const size_t MAX_LEN = 10;

volatile bool sync_flag = false;

int string2int(const string &s) {
	int number;
	istringstream iss(s);

	iss >> number;

	return number;
}

string int2string(int number) {
	ostringstream oss;

	oss << number;

	return oss.str();
}

void die(const char *message, int status = EXIT_FAILURE) {
	cout << message << endl;
	_exit(status);
}

void pdie(const char *s, int status = EXIT_FAILURE) {
	perror(s);
	_exit(status);
}

void sigusr1_handler(int signo) {
	sync_flag = true;
}

void pipe_inc_number(int *fd, pid_t other_pid, bool start = false) {
	sigset_t mask, oldmask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGUSR1);
	signal(SIGUSR1, sigusr1_handler);

	char s[MAX_LEN];
	int size;

	for (;;) {
		int number;
		
		sigprocmask(SIG_BLOCK, &mask, &oldmask);

		if (start) {
			start = false;
			number = 1;
		}
		else {
			while (!sync_flag) {
				sigsuspend(&oldmask);
			}

			int size = read(fd[0], s, MAX_LEN);

			if (size > 0) {
				number = string2int(s);
			}
		}

		sync_flag = false;
		
		cout << "PID = " << getpid() << ", число " << number << "." << endl;

		string ss = int2string(number + 1);
		size = write(fd[1], ss.c_str(), ss.size());

		kill(other_pid, SIGUSR1);
		sleep(1);
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
		
	}
}

int main() {
	int fd[2];
	
	if(pipe(fd) < 0) {
		pdie("pipe");
	}

	pid_t pid = fork();

	switch (pid) {
		case -1:
			pdie("fork");

		// Child
		case 0:
			cout << "Child pid = " << getpid() << "." << endl;
			pipe_inc_number(fd, getppid(), true);
			break;

		// Parent
		default:
			cout << "Parent pid = " << getpid() << "." << endl;
			pipe_inc_number(fd, pid);
			break;
	}

	close(fd[0]);
	close(fd[1]);

	return 0;
}