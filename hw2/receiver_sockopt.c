// server
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define BUF_SIZE 1000

struct frame_t {
  long int ID;
  long int length;
  char data[BUF_SIZE];
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s [save filename] [bind port]\n", argv[0]);
    exit(0);
  }

  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(listen_addr));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_addr.sin_port = htons(atoi(argv[2]));

  int listen_fd = socket(AF_INET, SOCK_DGRAM, 0);

  bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr));

  struct sockaddr_in cli_addr;
  int sock_len = sizeof(cli_addr);
  long int total_frame = 0, bytes_recv = 0;

  // get total frame
  struct timeval timeout;
  timeout.tv_sec = 2;
  setsockopt(listen_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
             sizeof(struct timeval));

  recvfrom(listen_fd, &total_frame, sizeof(total_frame), 0,
           (struct sockaddr *)&cli_addr, (socklen_t *)&sock_len);

  timeout.tv_sec = 0;
  setsockopt(listen_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
             sizeof(struct timeval));

  sendto(listen_fd, &total_frame, sizeof(total_frame), 0,
         (struct sockaddr *)&cli_addr, sizeof(cli_addr));

  for (int idx = 0; idx < total_frame;) {
    struct frame_t frame;
    recvfrom(listen_fd, &frame, sizeof(frame), 0, (struct sockaddr *)&cli_addr,
             (socklen_t *)&sock_len);
    sendto(listen_fd, &frame.ID, sizeof(frame.ID), 0,
           (struct sockaddr *)&cli_addr, sizeof(cli_addr));

    if (frame.ID == idx) {
      FILE *file = fopen(argv[1], "a");
      fwrite(frame.data, sizeof(char), frame.length, file);
      fclose(file);
      bytes_recv += frame.length;
      printf("Bytes received %ld\n", bytes_recv);
      idx++;
    }

    if (idx == total_frame) {
      printf("File transfer success!\n");
    }
  }

  return 0;
}
