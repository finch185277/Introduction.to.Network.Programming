#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 2048 // == server SEND_MAX

int main(int argc, char**argv) {
	if(argc != 3) {
		printf("Usage: %s <SERVER IP> <SERVER PORT>\n", argv[0]);
		exit(0);
	}
	struct sockaddr_in srv_addr;
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	srv_addr.sin_port = htons(atoi(argv[2]));

	int sock_fd;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

	fd_set r_set;
	FD_ZERO(&r_set);
	int max_fd = (sock_fd > STDIN_FILENO ? sock_fd : STDIN_FILENO) + 1;

	for(;;) {
		FD_SET(STDIN_FILENO, &r_set);
		FD_SET(sock_fd, &r_set);
		select(max_fd, &r_set, NULL, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO, &r_set)) {
			char send_buf[LINE_MAX];
			int n = read(STDIN_FILENO, send_buf, LINE_MAX - 1);
			send_buf[n] = '\0';
			write(sock_fd, send_buf, n);
		}

		if(FD_ISSET(sock_fd, &r_set)) {
			char rec_buf[LINE_MAX];
			int n = read(sock_fd, rec_buf, LINE_MAX - 1);
			if(n == 0) {
				close(sock_fd);
				break;
			}
			rec_buf[n] = '\0';
			fprintf(stdout, "%s", rec_buf);
		}
	}
	return 0;
}
