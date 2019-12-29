#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1024
#include <fstream>
#include <string>

void send_file(int fd, std::string file_name) {
  char send_buf[LINE_MAX];
  std::ifstream infile;
  infile.open(file_name, std::ios_base::binary);

  const auto begin = infile.tellg();
  infile.seekg(0, std::ios_base::end);
  const auto end = infile.tellg();
  infile.seekg(0, std::ios_base::beg);
  int file_size = end - begin;

  // send file name
  sprintf(send_buf, "%s", file_name.c_str());
  write(fd, send_buf, sizeof(file_name));
  printf("send name: [%s]\n", file_name.c_str());

  // send file size
  sprintf(send_buf, "%d", file_size);
  write(fd, send_buf, sizeof(send_buf));
  printf("send size: %d\n", file_size);

  // send file content
  char *content = new char[file_size];
  infile.read(content, file_size);
  write(fd, content, file_size);
  printf("send content: %s\n", content);
  delete content;

  return;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    printf("Usage: %s <SERVER IP> <SERVER PORT> <USERNAME>\n", argv[0]);
    exit(0);
  }

  printf("Welcome to the dropbox-like server: %s\n", argv[3]);

  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(atoi(argv[2]));

  struct hostent *host = gethostbyname(argv[1]);
  memcpy(&srv_addr.sin_addr, host->h_addr, host->h_length);

  int sock_fd;
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  int flag = fcntl(sock_fd, F_GETFL, 0);
  fcntl(sock_fd, F_SETFL, flag | O_NONBLOCK);

  flag = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flag | O_NONBLOCK);

  connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
  write(sock_fd, argv[3], sizeof(argv[3]));

  for (;;) {
    char buf[LINE_MAX];
    int n = read(STDIN_FILENO, buf, LINE_MAX - 1);
    if (n > 0) {
      buf[n] = '\0';
      if (strcmp(buf, "exit\n") == 0) {
        write(sock_fd, buf, n);
        close(sock_fd);
        break;
      }
      char *tok = strtok(buf, " \n");
      if (strcmp(tok, "put") == 0) {
        write(sock_fd, tok, sizeof(tok));

        char file_name[LINE_MAX];
        tok = strtok(NULL, " \n");
        strcpy(file_name, tok);
        std::string file(file_name);

        printf("wanna put %s\n", file_name);
        send_file(sock_fd, file);
      }
    }

    char file_name[LINE_MAX];
    n = read(sock_fd, file_name, LINE_MAX - 1);
    if (n > 0) {
      // get file name
      file_name[n] = '\0';

      // get file size
      char file_size[LINE_MAX];
      n = read(sock_fd, file_size, LINE_MAX - 1);
      file_size[n] = '\0';
      int size = atoi(file_size);

      // get file content
      char *content = new char[size];
      FILE *fp = fopen(file_name, "w+t");
      n = read(sock_fd, content, size);
      write(fileno(fp), content, size);
      fclose(fp);

    } else if (n == 0) {
      close(sock_fd);
      break;
    }
  }

  return 0;
}
