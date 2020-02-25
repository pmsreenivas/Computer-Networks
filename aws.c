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


#define BACKLOG 10   // how many pending connections queue will hold

/*This code is copied from Beej's page 25*/
void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

int main(){
/*This code is copied from Beej's page 27*/

    int sockfd_Client, sockfd_Monitor, new_fd, sockfd_ServerA, sockfd_ServerB, new_fd2;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv,numbytes,LI;
	char buf[BUFFERLENGTH];
	char str[BUFFERLENGTH];
	double sz,SP;
	int flag = 0;

    //bind with client
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(localhost, AWS_Client_Port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
	}
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_Client = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
			continue; 
		}
        if (setsockopt(sockfd_Client, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
			exit(1); 
		}
        if (bind(sockfd_Client, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd_Client);
            perror("server: bind");
			continue; 
		}
		break; 
	}
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
	}
	
    //bind with monitor
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP


	if ((rv = getaddrinfo(localhost, AWS_Monitor_Port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
	}
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd_Monitor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
			continue; 
		}
        if (setsockopt(sockfd_Monitor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
			exit(1); 
		}
        if (bind(sockfd_Monitor, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd_Monitor);
            perror("server: bind");
			continue; 
		}
		break; 
	}
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
	}

	if (listen(sockfd_Client, BACKLOG) == -1) {
        perror("listen");
        exit(1);
	}
	if (listen(sockfd_Monitor, BACKLOG) == -1) {
        perror("listen");
        exit(1);
	}
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
		exit(1); 
	}	
    printf("The AWS is up and running\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd_Client, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
			continue; 
		}// accept client

		

		numbytes = recv(new_fd, buf, BUFFERLENGTH - 1, 0);
		buf[numbytes] = '\0';
		if(buf[0] == 'w'){
			strcpy(str,"write");
		} else{
			strcpy(str,"compute");
		}
		printf("The AWS received operation <%s> from the client using TCP over port <%s>\n",str, AWS_Client_Port);

		sin_size = sizeof their_addr;
		if(!flag){
        	new_fd2 = accept(sockfd_Monitor, (struct sockaddr *)&their_addr, &sin_size);
        	if (new_fd2 == -1) {
            	perror("accept");
				continue; 
			}// accept monitor
		flag = 1;
		}
		send(new_fd2, buf, strlen(buf), 0);
		printf("The AWS sent operation <%s> and arguments to the monitor using TCP over port <%s>\n",str, AWS_Monitor_Port);

		/*This code is copied from Beej's page 30 */
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		if ((rv = getaddrinfo(localhost,Server_A_Port , &hints, &servinfo)) != 0) {
		 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		    return 1;
		}
		    // loop through all the results and make a socket
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd_ServerA = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		    	perror("talker: socket");
				continue; 
			}
			break; 
		}
		if (p == NULL) {
			fprintf(stderr, "talker: failed to create socket\n");
		    return 2;
		}
		//Talk with serverA
		if ((sendto(sockfd_ServerA, buf, strlen(buf), 0,p->ai_addr, p->ai_addrlen)) == -1) {
		        perror("talker: sendto");
				exit(1); 
		}
		freeaddrinfo(servinfo);
		printf("The AWS sent operation <%s> to Backend-Server A using UDP over port <%s>\n",str, AWS_UDP_Port );
        sin_size = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_ServerA, buf, BUFFERLENGTH -1 , 0,(struct sockaddr *)&their_addr, &sin_size)) == -1) {
		        perror("recvfrom");
		        exit(1);
		}
		buf[numbytes] = '\0';
		if(!strncmp(buf, "write_result", strlen("write_result"))){
			printf("The AWS received response from Backend-Server A for writing using UDP over port <%s>\n",AWS_UDP_Port);
			send(new_fd, buf, strlen(buf), 0);
			printf("The AWS sent result to client for operation <write> using TCP over port <%s>\n",AWS_Client_Port);
			send(new_fd2, buf, strlen(buf), 0);
			printf("The AWS sent write response to the monitor using TCP over port <%s>\n",AWS_Monitor_Port);

		} else if(!strncmp(buf, "compute_result", strlen("compute_result"))){
			if(!strncmp(buf, "compute_result_success", strlen("compute_result_success"))){
				printf("The AWS received link information from Backend-Server A using UDP over port <%s>\n",AWS_UDP_Port);
				/*This code is copied from Beej's page 30 */
				memset(&hints, 0, sizeof hints);
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_DGRAM;
				if ((rv = getaddrinfo(localhost,Server_B_Port , &hints, &servinfo)) != 0) {
		 			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		    		return 1;
				}
		    	// loop through all the results and make a socket
				for(p = servinfo; p != NULL; p = p->ai_next) {
					if ((sockfd_ServerB = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		    			perror("talker: socket");
						continue; 
					}
					break; 
				}
				if (p == NULL) {
					fprintf(stderr, "talker: failed to create socket\n");
		    		return 2;
				}
				//Talk with serverB
				if ((sendto(sockfd_ServerB, buf, strlen(buf), 0,p->ai_addr, p->ai_addrlen)) == -1) {
		        	perror("talker: sendto");
					exit(1); 
				}
				freeaddrinfo(servinfo);
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
				printf("The AWS sent link ID=<%d>, size=<%.2lf>, power=<%.2lf>, and link information to Backend-Server B using UDP over port <%s>\n",LI, sz, SP, Server_B_Port );
        		sin_size = sizeof their_addr;
				if ((numbytes = recvfrom(sockfd_ServerB, buf, BUFFERLENGTH -1 , 0,(struct sockaddr *)&their_addr, &sin_size)) == -1) {
		        	perror("recvfrom");
		        	exit(1);
				}
				buf[numbytes] = '\0';
				printf("The AWS received outputs from Backend-Server B using UDP over port <%s>\n",AWS_UDP_Port );
				send(new_fd, buf, strlen(buf), 0);
				printf("The AWS sent result to client for operation <compute> using TCP over port <%s>\n",AWS_Client_Port);
				send(new_fd2, buf, strlen(buf), 0);
				printf("The AWS sent compute response to the monitor using TCP over port <%s>\n",AWS_Monitor_Port);
				close(sockfd_ServerB);
			} else if(!strncmp(buf, "compute_result_failure", strlen("compute_result_failure"))){
				printf("Link ID not found\n");
				send(new_fd, buf, strlen(buf), 0);
				printf("The AWS sent result to client for operation <compute> using TCP over port <%s>\n",AWS_Client_Port);
				send(new_fd2, buf, strlen(buf), 0);
				printf("The AWS sent compute response to the monitor using TCP over port <%s>\n",AWS_Monitor_Port);
				close(sockfd_ServerB);
			}
		}
        
        close(new_fd);
        close(sockfd_ServerA);
    }
	return 0; 
}