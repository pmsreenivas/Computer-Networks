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

int main(){
	int sockfd, numbytes;
	struct addrinfo hints, *servinfo, *p;
	int rv, LI;
	char buf[BUFFERLENGTH];
	double L,V,BW,NP,sz,SP,Tt,Tp,D;

	/*This code is copied from Beej's page 28 */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo(localhost, AWS_Monitor_Port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    exit(1);
	}

	    // loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
	    	perror("monitor: socket");
			continue; 
		}
	    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    	close(sockfd);
	        perror("monitor: connect");
			continue; 
		}
		break; 
	}
	if (p == NULL) {
		fprintf(stderr, "monitor: failed to connect\n");
	    exit(1);
	}
	freeaddrinfo(servinfo); // all done with this structure

	printf("The monitor is up and running.\n");



	// when client sends a write or compute request to the AWS, monitor receives an appropriate response
	while(1){
		numbytes = recv(sockfd, buf, BUFFERLENGTH - 1, 0);
		buf[numbytes] = '\0';
		if(!(strncmp(buf,"write_input",strlen("write_input")))){
			char *start_ptr = buf;
			char *tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			BW = atof(start_ptr);
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			L = atof(start_ptr);
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			V = atof(start_ptr);
			start_ptr = tab_ptr + 1;
			NP = atof(start_ptr);
			printf("The monitor received BW = <%.2lf>, L = <%.2lf>, V = <%.2lf> and P = <%.2lf> from the AWS\n",BW,L,V,NP);
		} else if(!(strncmp(buf,"compute_input",strlen("compute_input")))){
			char *start_ptr = buf;
			char *tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			LI = atoi(start_ptr);
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			sz = atof(start_ptr);
			start_ptr = tab_ptr + 1;
			SP = atof(start_ptr);
			printf("The monitor received link ID = <%d>, size = <%.2lf>, and power = <%.2lf> from the AWS\n",LI,sz,SP);
		} else if(!(strncmp(buf,"write_result",strlen("write_result")))){
			printf("The write operation has been completed successfully\n");
		} else if(!(strncmp(buf,"compute_result_success",strlen("compute_result_success")))){
			char *start_ptr = buf;
			char *tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			Tt = atof(start_ptr);
			start_ptr = tab_ptr + 1;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			Tp = atof(start_ptr);
			start_ptr = tab_ptr + 1;
			D = atof(start_ptr);
			printf("The result for link <%d>:\nTt = <%.2lf>ms,\nTp = <%.2lf>ms,\nDelay = <%.2lf>ms\n",LI,Tt,Tp,D);
		} else if(!(strncmp(buf,"compute_result_failure",strlen("compute_result_failure")))){
			printf("Link ID not found\n");
		} 
	}
	return 0;	
}

