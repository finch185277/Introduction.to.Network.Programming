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

  struct sockaddr_in dst_addr;
  memset(&dst_addr, 0, sizeof(dst_addr));
  dst_addr.sin_family = AF_INET;
  dst_addr.sin_addr.s_addr = inet_addr(argv[2]);
  dst_addr.sin_port = htons(atoi(argv[3]));

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (access(argv[1], F_OK) == 0) {
    // get file size
    struct stat st;
    stat(argv[1], &st);
    long int file_size = st.st_size;

    // set time out
    struct timeval timeout;
    timeout.tv_sec = 2;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof(struct timeval));

    // calculate total frame
    long int total_frame = 0, ack_num = 0;
    if (file_size % BUF_SIZE == 0) {
      total_frame = file_size / BUF_SIZE;
    } else {
      total_frame = file_size / BUF_SIZE + 1;
    }

    struct sockaddr_in src_addr;
    int sock_len = sizeof(src_addr);

    // send total frame
    while (ack_num != total_frame) {
      struct frame_t frame;
      frame.ID = 0;
      sprintf(frame.data, "%ld", total_frame);
      sendto(sock_fd, &frame, sizeof(frame), 0, (struct sockaddr *)&dst_addr,
             sizeof(dst_addr));
      recvfrom(sock_fd, &ack_num, sizeof(ack_num), 0,
               (struct sockaddr *)&src_addr, (socklen_t *)&sock_len);
    }

    // send file
    FILE *file = fopen(argv[1], "r");
    for (long int idx = 1; idx <= total_frame;) {
      struct frame_t frame;
      frame.ID = idx;
      frame.length = fread(frame.data, sizeof(char), BUF_SIZE, file);

      sendto(sock_fd, &frame, sizeof(frame), 0, (struct sockaddr *)&dst_addr,
             sizeof(dst_addr));
      recvfrom(sock_fd, &ack_num, sizeof(ack_num), 0,
               (struct sockaddr *)&src_addr, (socklen_t *)&sock_len);

      while (ack_num != frame.ID) {
        sendto(sock_fd, &frame, sizeof(frame), 0, (struct sockaddr *)&dst_addr,
               sizeof(dst_addr));
        recvfrom(sock_fd, &ack_num, sizeof(ack_num), 0,
                 (struct sockaddr *)&src_addr, (socklen_t *)&sock_len);
      }

      idx++;

      if (idx == total_frame + 1) {
        printf("File transfer success!\n");
      }
    }
    fclose(file);
  }
  return 0;
}
