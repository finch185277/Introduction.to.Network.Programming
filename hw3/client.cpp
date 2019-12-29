#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LINE_MAX 1050
#include <fstream>
#include <string>

struct segment_t {
  char seq_no[10];
  char action[10];
  char file_name[20];
  char file_size[10];
  char seg_size[10];
  char content[1000];
};

void send_file(int fd, std::string file_name) {
  std::ifstream infile;
  infile.open(file_name, std::ios_base::binary);

  const auto begin = infile.tellg();
  infile.seekg(0, std::ios_base::end);
  const auto end = infile.tellg();
  infile.seekg(0, std::ios_base::beg);
  int file_size = end - begin;

  int loops = (file_size / 1000) + 1;
  for (int i = 1; i <= loops; i++) {
    struct segment_t segment;
    sprintf(segment.seq_no, "%d", i);
    sprintf(segment.file_name, "%s", file_name.c_str());
    sprintf(segment.file_size, "%d", file_size);

    if (i != loops)
      infile.read(segment.content, 1000);
    else
      infile.read(segment.content, file_size % 1000);

    write(fd, &segment, sizeof(segment));
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
        write(sock_fd, tok, sizeof(tok));

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
      FILE *fp = fopen(segment.file_name, "w+t");

      write(fileno(fp), segment.content, atoi(segment.seg_size));

      int loops = (atoi(segment.file_size) / 1000) + 1;
      for (int i = 2; i <= loops; i++) {
        write(fileno(fp), segment.content, atoi(segment.seg_size));
      }

      fclose(fp);

    } else if (n == 0) {
      close(sock_fd);
      break;
    }
  }

  return 0;
}
