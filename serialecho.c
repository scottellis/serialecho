/*
 * Copyright (C) 2015 Jumpnow Technologies, LLC - http://jumpnowtek.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * Echo test a serial port with rx and tx jumpered.
 * Used for debugging boards.
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


void register_sig_handler();
void sigint_handler(int sig);
int open_port(const char *device, int speed, struct termios *oldopts);
void run_test(int fd);
void msleep(int ms);

volatile int done;

#ifdef __linux__
#define DEFAULT_PORT "/dev/ttyO4"
#else
#define DEFAULT_PORT "/dev/ttyu2"
#endif

#define DEFAULT_SPEED 115200

void usage(char *argv_0)
{
	printf("\nUsage: %s [-d <device>] [-s <speed>] [-h]\n", argv_0);
	printf("  -d <device>    Serial device, default is /dev/ttyO4\n");
	printf("  -s <speed>     Speed, default is 115200\n");
	printf("  -h             Show this help\n\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int opt;
	char device[64];
	int speed;
	struct termios oldopts;
  
	strcpy(device, DEFAULT_PORT);
	speed = DEFAULT_SPEED;

	while ((opt = getopt(argc, argv, "d:s:h")) != -1) {
		switch (opt) {
		case 'd':
			if (strlen(optarg) > sizeof(device) - 1) {
				printf("Device name too long: %s\n", optarg);
				usage(argv[0]);
			}

			strcpy(device, optarg);
			break;

		case 's':
			speed = strtol(optarg, NULL, 0);
			
			if (speed < 0 || speed > 3000000) {
				printf("Invalid speed: %s\n", optarg);
				usage(argv[0]);
			}

			break;

		case 'h':
		default:
			usage(argv[0]);
			break;
		}
	}

	int fd = open_port(device, speed, &oldopts);

	if (fd < 0)
		exit(1);

	run_test(fd);

	tcsetattr(fd, TCSANOW, &oldopts);

	close(fd);

	return 0;
}

void run_test(int fd)
{
	int len, txlen, pos;
	int retries;
	char tx[64];
	char rx[64];

	strcpy(tx, "ABCDEFJHIJKLMNOPQRSTUVWXYZ1234567890abcdefjhijklmnopqrstuvwxyz");
	txlen = strlen(tx);

	printf("\n--- ctrl-c to stop ---\n");

	while (!done) {
		memset(rx, 0, sizeof(rx));

		len = write(fd, tx, txlen);

		if (len != txlen) {
			perror("write");
			break;
		}

		printf("\nWrote: %s\n", tx);

		pos = 0;
		retries = 0;

		while (pos < txlen && !done) { 
			msleep(75);

			len = read(fd, rx + pos, txlen - pos);

			if (len < 0) {
				perror("read");
				done = 1;
				break;
			}
	
			pos += len;	

			if (pos < txlen)
				printf("Partial read: %d bytes\n", len);

			if (retries++ > 10) {
				printf("Giving up\n");
				done = 1;
			}
		}

		if (!done) {
			printf("Read : %s\n", rx);
			msleep(500);
		}
	}
}

int open_port(const char *device, int speed, struct termios *oldopts)
{
	int fd;
	struct termios opts;

	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

	if (fd < 0) {
		perror("open");
		return -1;
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

	cfsetispeed(&opts, speed);
	cfsetospeed(&opts, speed);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &opts);

	return fd;
}

void msleep(int ms)
{
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
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

