#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LINE_MAX 1000

void handler(int signo) {
  wait(nullptr);
  return;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <server IP> <server port>\n", argv[0]);
    exit(0);
  }

  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(atoi(argv[2]));

  struct hostent *host = gethostbyname(argv[1]);
  memcpy(&srv_addr.sin_addr, host->h_addr, host->h_length);

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  connect(sock_fd, (const struct sockaddr *)&srv_addr, sizeof(srv_addr));

  signal(SIGCHLD, handler);

  pid_t pid = fork();
  if (pid == 0) { // child process
    for (;;) {
      char buf[LINE_MAX];
      int n = recv(sock_fd, buf, LINE_MAX - 1, 0);
      if (n == 0) // EOF
        break;
      buf[n] = '\0';
      printf("%s\n", buf);
    }
  } else { // parent process
    for (;;) {
      char buf[LINE_MAX];
      int n = read(STDIN_FILENO, buf, LINE_MAX - 1);
      if (n == 0) // EOF
        break;
      buf[n] = '\0';
      write(sock_fd, buf, n);
    }
    char command[LINE_MAX];
    sprintf(command, "kill %d", pid);
    system(command);
  }

  return 0;
}
