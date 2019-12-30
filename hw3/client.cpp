#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1050
#define CONTENT_SIZE 1000
#include <fstream>
#include <string>

struct segment_t {
  char action[10];
  char file_name[20];
  char file_size[20];
  char content[CONTENT_SIZE];
};

void send_file(int fd, std::string file_name) {
  std::ifstream infile;
  infile.open(file_name, std::ios_base::binary);

  const auto begin = infile.tellg();
  infile.seekg(0, std::ios_base::end);
  const auto end = infile.tellg();
  infile.seekg(0, std::ios_base::beg);
  int file_size = end - begin;

  struct segment_t segment;
  strcpy(segment.action, "put");
  sprintf(segment.file_name, "%s", file_name.c_str());
  sprintf(segment.file_size, "%d", file_size);
  int n = write(fd, &segment, sizeof(segment));
  printf("send segment %d bytes\n", n);

  int loops, last_size;
  if (file_size % CONTENT_SIZE == 0) {
    loops = file_size / CONTENT_SIZE;
    last_size = CONTENT_SIZE;
  } else {
    loops = (file_size / CONTENT_SIZE) + 1;
    last_size = file_size % CONTENT_SIZE;
  }

  for (int i = 1; i <= loops; i++) {
    int size;
    if (i == loops)
      size = last_size;
    else
      size = CONTENT_SIZE;
    char content[size];
    infile.read(content, size);
    n = write(fd, content, size);
    printf("send content %d bytes <-- %d\n", n, i);
  }

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
        // get file name
        char file_name[LINE_MAX];
        tok = strtok(NULL, " \n");
        strcpy(file_name, tok);
        std::string file(file_name);

        send_file(sock_fd, file);
      }
    }

    struct segment_t segment;
    n = read(sock_fd, &segment, sizeof(segment));
    if (n > 0) {
      if (strcmp(segment.action, "put") == 0) {
        FILE *fp = fopen(segment.file_name, "w+t");

        char *content = new char[atoi(segment.file_size)];
        n = read(sock_fd, content, atoi(segment.file_size));
        write(fileno(fp), content, atoi(segment.file_size));
        delete content;

        fclose(fp);
      }
    } else if (n == 0) {
      close(sock_fd);
      break;
    }
  }

  return 0;
}
