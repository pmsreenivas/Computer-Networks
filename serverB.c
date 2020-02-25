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
#include <math.h>

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
	FILE* fp;
	double sz, SP, BW, L, V, NP, Tt, Tp, D;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP		    
	if ((rv = getaddrinfo(localhost, Server_B_Port, &hints, &servinfo)) != 0) {
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
	printf("The Server B is up and running using UDP on port <%s>\n",Server_B_Port);
	//search for the appropriate link, extract all required values and compute the delays
	while(1){
		sin_size = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, BUFFERLENGTH -1 , 0,(struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';
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

		printf("The Server B received link information: link <%d>, file size <%.2lf>, and signal power <%.2lf>\n", LI,sz,SP);

		fp = fopen("database.txt", "r");
		int i;
		for(i = 1; i <= LI; i++){
			fgets(buf2, BUFFERLENGTH, fp);
		}
	 	start_ptr = buf2;
		tab_ptr = strchr(start_ptr, '\t');
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
		fclose(fp);


		double S = pow(10,((SP - 30.0)/10.0));
		double N = pow(10,((NP - 30.0)/10.0));
		double K = 1 + S/N;
		double C = BW*(log10(K)/log10(2.0));
		Tt = (sz/C)/1000.0;
		Tp = 1000.0*L/V;
		D = Tt + Tp;
		Tt *= 100.0 ;
		Tp *= 100.0;
		D *= 100.0;
		Tt = round(Tt);
		Tp = round(Tp);
		D = round(D);
		Tt /= 100.0 ;
		Tp /= 100.0;
		D /= 100.0;
		printf("The Server B finished the calculation for link <%d>\n",LI);
		sprintf(buf,"compute_result_success\t%.2lf\t%.2lf\t%.2lf",Tt,Tp,D);
		if ((sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *)&their_addr, sin_size)) == -1) {
			perror("talker: sendto");
			exit(1); 
		}
		printf("The Server B finished sending the output to AWS\n");

	}

}

				

