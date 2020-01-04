#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char *sockaddr2str(const struct sockaddr *sa, int flags) {
	char host[128];
	char serv[16];

	socklen_t salen = sizeof(struct sockaddr_in);
	int n = getnameinfo(sa, salen, host, sizeof(host), serv, sizeof(serv), flags);

	if (n != 0)
		return NULL;

	char *ret = malloc(200 * sizeof(char));
	sprintf(ret, "%s", host);

	return ret;
}

int print_host(const char *hostname) {
	struct addrinfo *result = NULL;
	struct addrinfo hint;

	memset(&hint, 0, sizeof(hint));
	hint.ai_socktype = SOCK_STREAM;
	int n = getaddrinfo(hostname, NULL, &hint, &result);

	if (n == 0) {
		struct addrinfo *cur = result;
		unsigned cnt = 0;

		struct sockaddr_in sock_addr;
		int req_flag = inet_pton(AF_INET, hostname, &sock_addr.sin_addr);

		if(cur){
			char *dotted, *revhost;
			dotted = sockaddr2str(cur->ai_addr, NI_NUMERICHOST | NI_NUMERICSERV);
			revhost = sockaddr2str(cur->ai_addr, NI_NAMEREQD | NI_NUMERICSERV);

			if (req_flag) {
				if (revhost) {
					puts(revhost);
					free(revhost);
				}else{
					printf("Can't find %s\n", hostname);
				}
			} else {
				if (dotted) {
					puts(dotted);
					free(dotted);
				}else{
					printf("Can't find %s\n", hostname);
				}

			}

		}
	} else {
		printf("Can't find %s\n", hostname);
	}


	freeaddrinfo(result);
	return n;

}

int main(int argc, char **argv) {
	for(;;) {
		printf("> ");
		char req[200];
		scanf("%s", req);
		if(strcmp(req, "exit") == 0)
			break;
		print_host(req);
	}
	return 0;
}
