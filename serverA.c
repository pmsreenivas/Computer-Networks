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
#include <signal.h>

#include "header.h"

int main(){
	/*This code is copied from Beej's page 30 */
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv,LI;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[BUFFERLENGTH];
	char buf2[BUFFERLENGTH];
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	double BW,L,V,NP,sz,SP;
	FILE* fp;
	int last_link_id = 0;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP		    
	if ((rv = getaddrinfo(localhost, Server_A_Port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
	    	perror("listener: socket");
			continue; 
		}
	    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    	close(sockfd);
	        perror("listener: bind");
			continue; 
		}
		break; 
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
	    return 2;
	}
	freeaddrinfo(servinfo);
	printf("The Server A is up and running using UDP on port <%s>\n",Server_A_Port);

	while(1){
		sin_size = sizeof their_addr;
		buf[0] ='\0';
		if ((numbytes = recvfrom(sockfd, buf, BUFFERLENGTH , 0,(struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';
		//write the data into database.txt
		if(!strncmp(buf, "write_input", strlen("write_input"))){
			printf("The Server A received input for writing\n");
			fp = fopen("database.txt", "a");
			++last_link_id;
			char *start_ptr = buf;
			char *tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			start_ptr = ++tab_ptr;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			BW = atof(start_ptr);
			start_ptr = ++tab_ptr;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			L = atof(start_ptr);
			start_ptr = ++tab_ptr;
			tab_ptr = strchr(start_ptr, '\t');
			*tab_ptr = '\0';
			V = atof(start_ptr);
			start_ptr = ++tab_ptr;
			NP = atof(start_ptr);
			sprintf(buf2,"%d\t%.2lf\t%.2lf\t%.2lf\t%.2lf\n", last_link_id, BW,L,V,NP);
			fputs(buf2, fp);
			strcpy(buf, "write_result");


			if ((sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *)&their_addr, sin_size)) == -1) {
		        perror("talker: sendto");
				exit(1); 
			}
			fclose(fp);
			printf("The Server A wrote link <%d> to database\n", last_link_id);

			//search for the link to be computed
		} else if(!strncmp(buf, "compute_input", strlen("compute_input"))){
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
			printf("The Server A received input <%d> for computing\n", LI);
			if((LI > last_link_id )|| (LI <= 0)){
				strcpy(buf, "compute_result_failure");
				if ((sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *)&their_addr, sin_size)) == -1) {
		        	perror("talker: sendto");
					exit(1); 
				}
				printf("Link ID not found\n");
			} else {
				sprintf(buf,"compute_result_success\t%d\t%.2lf\t%.2lf\t",LI,sz,SP);
				if ((sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *)&their_addr, sin_size)) == -1) {
		        	perror("talker: sendto");
					exit(1); 
				}
				printf("The Server A finished sending the search result to AWS\n");
			}
		}
	}

}

