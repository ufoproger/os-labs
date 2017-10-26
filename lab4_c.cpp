#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <cstdio>

#define PIPE_P2C "/tmp/pipe_p2c"
#define PIPE_C2P "/tmp/pipe_c2p"
#define BUFSIZE 100

void initPipe(const char *pathname) {
    if (mkfifo(pathname, 0777)) {
        printf("%s is not created\n", pathname);
        perror("mkfifo");

        _exit(-1);
    }

    printf("%s is created\n", pathname);
}

void closeFds(int &fdIn, int &fdOut) {
    close(fdIn);
    close(fdOut);
}

int main (int argc, char *argv[]) {
    int fdIn, fdOut, len;
    char buf[BUFSIZE];

    initPipe(PIPE_P2C);
    initPipe(PIPE_C2P);

    switch (fork()) {
        case -1:
            perror("fork");
            return -1;

        case 0: {
			if ((fdIn = open(PIPE_P2C, O_RDONLY)) <= 0) {
			    perror("child: Open pipe for output");
			    _exit(-1);
			}

			for (int i = 0; i < BUFSIZE; ++i) {
				if ((len = read(fdIn, &buf[i], 1)) <= 0) {
					perror("read");
					_exit(-1);
				}

				if (buf[i] == '\n') {
					buf[i] = 0;
					break;
				}
			}

			int count = 0;

			sscanf(buf, "%d", &count);

			int sum = 0;

			for (int j = 0; j < count; ++j) {
				for (int i = 0; i < BUFSIZE; ++i) {
					read(fdIn, &buf[i], 1);

					if (buf[i] == '\n') {
						buf[i] = 0;
						break;
					}
				}

				int t;
				sscanf(buf, "%d", &t);
				sum += t;
			}

			remove(PIPE_P2C);

	        if ((fdOut = open(PIPE_C2P, O_WRONLY)) <= 0) {
		        perror("child: Open pipe for output");
		        _exit(-1);
		    }

		    sprintf(buf, "%d", sum);

		    write(fdOut, buf, strlen(buf));

            closeFds(fdIn, fdOut);
            break;
        }

        default:
	        if ((fdOut = open(PIPE_P2C, O_WRONLY)) <= 0) {
		        perror("parent: Open pipe for output");
		        _exit(-1);
		    }

		    sprintf(buf, "%d\n", argc - 1);
		    write(fdOut, buf, strlen(buf));

		    for (int i = 1; i < argc; ++i) {
		    	sprintf(buf, "%s\n", argv[i]);
			    write(fdOut, buf, strlen(buf));		    	
		    }

			if ((fdIn = open(PIPE_C2P, O_RDONLY)) <= 0) {
			    perror("parent: Open pipe for output");
			    _exit(-1);
			}

			read(fdIn, buf, BUFSIZE);
			remove(PIPE_C2P);
			printf("Сумма %s\n", buf);

            closeFds(fdIn, fdOut);
            break;
    }

    return 0;
}