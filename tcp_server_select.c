#include <stdio.h>
#include <string.h>		/* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include "tcp_server_select.h"

#define MAXBUF 256
#define PORT    2323		/* Port to listen on */
#define BACKLOG     16		/* Passed to listen() */
#define MAX_CLIENTS 4


int main(void)
{
	unsigned int port=PORT;

	struct sockaddr_in local;

	unsigned int i;

	char buf[256];

	int sockfd=0, rfd=0;
	int client_socks[MAX_CLIENTS];

	fd_set reading_set;
	fd_set writing_set;
	int select_max_nfds=0;

	int ret;

	memset(&client_socks, -1, sizeof(client_socks[0])*MAX_CLIENTS);

	if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0) {
		perror("socket");
		return -1;
	}

	bzero(&local,sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(port);

	local.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*)&local, sizeof(local)) < 0) {
		perror("bind");
		return -errno;
	}

	listen(sockfd, 16);

	FD_ZERO(&reading_set);
	FD_SET(sockfd, &reading_set);
	select_max_nfds=sockfd;

	while(1) {
		/* Make sure we always wait for new connections at sockfd */
		FD_SET(sockfd, &reading_set);
		ret=select(select_max_nfds+1, &reading_set, NULL, NULL, NULL);

		/* Check if there's a new connection at sockfd */
		if(FD_ISSET(sockfd, &reading_set)) {
			ret=accept_new_connection(sockfd, client_socks);
			if(ret>=0) {
				FD_SET(client_socks[ret], &reading_set);
				if(client_socks[ret]>select_max_nfds) select_max_nfds++;
			}
		}
		for(i=0; i<MAX_CLIENTS; i++) {
			if(client_socks[i]>=0) {
				printf("Checking slot %i\n", i);
				if(FD_ISSET(client_socks[i], &reading_set)) {
					ret=read(client_socks[i], buf, MAXBUF);
					printf("\tRead from slot %i\n", i);
					if(!ret) {
						FD_CLR(client_socks[i], &reading_set);
						close(client_socks[i]);
						if(select_max_nfds==client_socks[i]) select_max_nfds--;
						client_socks[i]=-1;
					}
				}
			}
			if(client_socks[i]>=0) {
				FD_SET(client_socks[i], &reading_set);
			}
		}
	}
}

int accept_new_connection(int sockfd, int client_socks[])
{
	int first_empty_client=0;
	int rejected_client_fd;
	struct sockaddr_in remote;
	socklen_t remote_len;

	first_empty_client=find_empty_slot(client_socks, MAX_CLIENTS);
	if(first_empty_client<0) {
		fprintf(stderr, "No more slot clients available\n");
		rejected_client_fd=accept(sockfd, (struct sockaddr*)&remote, &remote_len);
		close(rejected_client_fd);
		return -1;
	} else {
		client_socks[first_empty_client]=accept(sockfd, (struct sockaddr*)&remote, &remote_len);

		if(client_socks[first_empty_client]<0) {
			perror("accept");
			return -1;
		} else {
			/* Set as nonblocking and return the slot used */
			fcntl(client_socks[first_empty_client], F_SETFL, O_NONBLOCK);
			return first_empty_client;
		}
	}
}

int find_empty_slot(int array[], unsigned int max)
{
	unsigned int i;
	for(i=0; i<max; i++) {
		if(array[i]<0) return i;
	}
	return -1;
}
