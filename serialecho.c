/*
  Used for debugging boards, echo test a serial port with tx/rx jumpered.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <signal.h>


int open_port(const char *port, int baud, struct termios *oldopts);
void run_loop(int fd);
void msleep(int ms);
void register_sig_handler();
void sigint_handler(int sig);

int done;

#ifdef __linux__
#define DEFAULT_PORT "/dev/ttyO0"
#else
#define DEFAULT_PORT "/dev/ttyu1"
#endif

#define DEFAULT_SPEED 115200 

int main(int argc, char **argv)
{
    int opt;
    struct termios oldopts;
    char port[64];
    int speed = DEFAULT_SPEED;
   
    strcpy(port, DEFAULT_PORT);

    while ((opt = getopt(argc, argv, "p:s:h")) != -1) {
        switch (opt) {
        case 'p': 
            if (strlen(optarg) >= sizeof(port)) {
                printf("Port name is too long: %s\n", optarg);
                exit(1);
            }

            strcpy(port, optarg);
            break;

        case 's':
            speed = atoi(optarg);
            break;

        case 'h':
        default:
             printf("Usage: %s [-p <port>] [-s <speed>] [-h]\n", argv[0]);
             printf("  -p <port>   : default %s\n", DEFAULT_PORT);
             printf("  -s <speed>  : default %d\n", DEFAULT_SPEED);
             printf("  -h          : show this help\n");
             exit(1);
        }
    }

    int fd = open_port(port, speed, &oldopts);

    if (fd < 0)
        exit(1);

    register_sig_handler();

    printf("%s @ %d\n", port, speed);

    run_loop(fd);

    tcsetattr(fd, TCSANOW, &oldopts);

    close(fd);

    return 0;
}

void run_loop(int fd)
{
    int len;
    char tx[128];
    char rx[128];

    strcpy(tx, "ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz");

    while (!done) {
        memset(rx, 0, sizeof(rx));

        len = write(fd, tx, strlen(tx));

        if (len != strlen(tx)) {
            perror("write");
            break;
        }

        printf("Wrote (%d): %s\n", (int)strlen(tx), tx);

        msleep(200);

        len = read(fd, rx, strlen(tx));

        if (len < 0) {
            perror("read");
            break;
        }

        printf("Read (%d): %s\n\n", len, rx);

        msleep(1000);
    }
}

void msleep(int ms)
{
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    nanosleep(&ts, NULL);
}

int open_port(const char *port, int baud, struct termios *oldopts)
{
    int fd;
    struct termios opts;

    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // no delay
    fcntl(fd, F_SETFL, 0);

    tcgetattr(fd, &opts);

    if (oldopts)
        memcpy(oldopts, &opts, sizeof(struct termios));

    opts.c_cflag &= ~CSIZE;
    opts.c_cflag &= ~CSTOPB;
    opts.c_cflag &= ~PARENB;
    opts.c_cflag |= CLOCAL | CREAD | CS8;
    opts.c_iflag = IGNPAR;
    opts.c_oflag = 0;
    opts.c_lflag = 0;	

    opts.c_cc[VTIME] = 5;
    opts.c_cc[VMIN] = 0;

    cfsetispeed(&opts, baud);
    cfsetospeed(&opts, baud);

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &opts);

    return fd;
}

void register_sig_handler()
{
    struct sigaction sia;

    bzero(&sia, sizeof sia);
    sia.sa_handler = sigint_handler;

    if (sigaction(SIGINT, &sia, NULL) < 0) {
        perror("sigaction(SIGINT)");
        exit(1);
    } 
}

void sigint_handler(int sig)
{
    done = 1;
}
	
