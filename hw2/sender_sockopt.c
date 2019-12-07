// client
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 1000

struct frame_t {
  long int ID;
  long int length;
  char data[BUF_SIZE];
};

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Usage: %s [send filename] [target address] [connect port]\n",
           argv[0]);
    exit(0);
  }

  struct sockaddr_in srv_addr;
  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = inet_addr(argv[2]);
  srv_addr.sin_port = htons(atoi(argv[3]));

  int sock_fd;
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (access(argv[1], F_OK) == 0) {
    struct stat st;
    stat(argv[1], &st);
    int file_size = st.st_size;

    struct timeval timeout;
    timeout.tv_sec = 2;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof(struct timeval));

    int total_frame;
    if (file_size % BUF_SIZE == 0) {
      total_frame = file_size / BUF_SIZE;
    } else {
      total_frame = file_size / BUF_SIZE + 1;
    }

    int ack_num;
    struct sockaddr_in cli_addr;
    int sock_len = sizeof(cli_addr);

    while (ack_num != total_frame) {
      sendto(sock_fd, &total_frame, sizeof(total_frame), 0,
             (struct sockaddr *)&srv_addr, sizeof(srv_addr));
      recvfrom(sock_fd, &ack_num, sizeof(ack_num), 0,
               (struct sockaddr *)&cli_addr, (socklen_t *)&sock_len);
    }

    FILE *file = fopen(argv[1], "r");
    for (int idx = 0; idx < total_frame;) {
      struct frame_t frame;
      frame.ID = idx;
      frame.length = fread(frame.data, sizeof(char), BUF_SIZE, file);

      sendto(sock_fd, &frame, sizeof(frame), 0, (struct sockaddr *)&srv_addr,
             sizeof(srv_addr));
      recvfrom(sock_fd, &ack_num, sizeof(ack_num), 0,
               (struct sockaddr *)&cli_addr, (socklen_t *)&sock_len);

      while (ack_num != frame.ID) {
        sendto(sock_fd, &frame, sizeof(frame), 0, (struct sockaddr *)&srv_addr,
               sizeof(srv_addr));
        recvfrom(sock_fd, &ack_num, sizeof(ack_num), 0,
                 (struct sockaddr *)&cli_addr, (socklen_t *)&sock_len);
      }

      idx++;

      if (idx == total_frame) {
        printf("File transfer success!\n");
      }
    }
    fclose(file);

    timeout.tv_sec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof(struct timeval));
  }
  return 0;
}
