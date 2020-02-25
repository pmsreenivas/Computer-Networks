#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "header.h"


int main(int argc, char *argv[]){
	char buf[BUFFERLENGTH];
	int sockfd, numbytes;
	struct addrinfo hints, *servinfo, *p;
	int rv;


	if(!strcmp(argv[1], "write")){
		sprintf(buf,"write_input\t%lf\t%lf\t%lf\t%lf",atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]));
	} else if(!strcmp(argv[1], "compute")){
		sprintf(buf,"compute_input\t%d\t%lf\t%lf",atoi(argv[2]),atof(argv[3]),atof(argv[4]));
	}

	/*This code is copied from Beej's page 28 */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(localhost, AWS_Client_Port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    exit(1);
	}

	    // loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {

	    	perror("client: socket");
			continue; 
		}
	    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    	close(sockfd);
	        perror("client: connect");
			continue; 
		}
		break; 
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
	    exit(1);
	}
	freeaddrinfo(servinfo); // all done with this structure

	printf("The client is up and running.\n");


	// send a write or compute request to the AWS and receive the appropriate response
	if(!strcmp(argv[1], "write")){
		send(sockfd, buf, strlen(buf), 0);
		printf("The client sent write operation to AWS\n");
		numbytes = recv(sockfd, buf, BUFFERLENGTH - 1, 0);
		buf[numbytes] = '\0';
		printf("The write operation has been completed successfully\n");
		
	} else if(!strcmp(argv[1], "compute")){
		send(sockfd, buf, strlen(buf), 0);
		printf("The client sent link ID=<%d>, size=<%.2lf>, and power=<%.2lf> to AWS\n",atoi(argv[2]),atof(argv[3]),atof(argv[4]));
		numbytes = recv(sockfd, buf, BUFFERLENGTH - 1, 0);
		buf[numbytes] = '\0';
		if(!strncmp(buf,"compute_result_failure",strlen("compute_result_failure")))
			printf("Link ID not found\n");
		else{
			char *start_ptr = buf;
			char *tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			start_ptr = tab_ptr + 1;
			printf("The delay for link <%d> is <%.2lf> ms\n", atoi(argv[2]), atof(start_ptr));
		}
	}
	return 0;
}

