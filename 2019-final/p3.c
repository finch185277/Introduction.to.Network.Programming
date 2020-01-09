#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void print_if_info(char *iface) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_addr.sa_family = AF_INET;
  strcpy(ifr.ifr_name, iface);

  // get metric
  if (ioctl(fd, SIOCGIFMETRIC, &ifr) == -1) {
    printf("No such interface\n");
    exit(ENOENT);
  }
  int metric = ifr.ifr_metric;

  // get MTU
  if (ioctl(fd, SIOCGIFMTU, &ifr) == -1) {
    printf("No such interface\n");
    exit(ENOENT);
  }
  int mtu = ifr.ifr_mtu;

  // get index
  ioctl(fd, SIOCGIFINDEX, &ifr);
  int index = ifr.ifr_ifindex;

  // get NIC info
  ioctl(fd, SIOCGIFFLAGS, &ifr);
  short if_flag = ifr.ifr_flags;
  char *nic_info = malloc(500 * sizeof(char));
  strcat(nic_info, "< ");
  if (if_flag & IFF_UP)
    strcat(nic_info, "UP ");
  if (if_flag & IFF_BROADCAST)
    strcat(nic_info, "BROADCAST ");
  if (if_flag & IFF_DEBUG)
    strcat(nic_info, "DEBUG ");
  if (if_flag & IFF_LOOPBACK)
    strcat(nic_info, "LOOPBACK ");
  if (if_flag & IFF_POINTOPOINT)
    strcat(nic_info, "POINTOPOINT ");
  if (if_flag & IFF_RUNNING)
    strcat(nic_info, "RUNNING ");
  if (if_flag & IFF_NOARP)
    strcat(nic_info, "NOARP ");
  if (if_flag & IFF_PROMISC)
    strcat(nic_info, "PROMISC ");
  if (if_flag & IFF_NOTRAILERS)
    strcat(nic_info, "NOTRAILERS ");
  if (if_flag & IFF_ALLMULTI)
    strcat(nic_info, "ALLMULTI ");
  if (if_flag & IFF_MASTER)
    strcat(nic_info, "MASTER ");
  if (if_flag & IFF_SLAVE)
    strcat(nic_info, "SLAVE ");
  if (if_flag & IFF_MULTICAST)
    strcat(nic_info, "MULTICAST ");
  if (if_flag & IFF_PORTSEL)
    strcat(nic_info, "PORTSEL ");
  if (if_flag & IFF_AUTOMEDIA)
    strcat(nic_info, "AUTOMEDIA ");
  if (if_flag & IFF_DYNAMIC)
    strcat(nic_info, "DYNAMIC ");
  strcat(nic_info, ">");

  // get hardware address
  unsigned char MAC[6];
  ioctl(fd, SIOCGIFHWADDR, &ifr);
  for (unsigned int i = 0; i < 6; i++)
    MAC[i] = ifr.ifr_hwaddr.sa_data[i];

  // get IP address
  ioctl(fd, SIOCGIFADDR, &ifr);
  char ip_addr[16];
  sprintf(ip_addr, "%s",
          inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

  // get network mask
  ioctl(fd, SIOCGIFNETMASK, &ifr);
  char net_mask[16];
  sprintf(net_mask, "%s",
          inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr));

  // get broadcast address
  ioctl(fd, SIOCGIFBRDADDR, &ifr);
  char brd_addr[16];
  sprintf(brd_addr, "%s",
          inet_ntoa(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr));

  printf("%d: %s: flags=%d%s metric %d mtu %d\n", index, iface, if_flag,
         nic_info, metric, mtu);
  printf("  ether %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", MAC[0], MAC[1], MAC[2],
         MAC[3], MAC[4], MAC[5]);
  printf("  inet %s mask: %s brd %s\n", ip_addr, net_mask, brd_addr);

  free(nic_info);
  close(fd);
}

void print_all_if_info() {
  for (int index = 1; index < 10; index++) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_addr.sa_family = AF_INET;
    ifr.ifr_ifindex = index;

    // get interface name
    char iface[IF_NAMESIZE];
    if (ioctl(fd, SIOCGIFNAME, &ifr) == -1) {
      close(fd);
      continue;
    }
    strcpy(iface, ifr.ifr_name);

    close(fd);

    print_if_info(iface);
  }
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    print_all_if_info();
  } else {
    for (int i = 1; i < argc; i++) {
      print_if_info(argv[i]);
    }
  }
  return 0;
}
