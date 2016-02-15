#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h> 
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <pthread.h>
#include "../DynamycBuffer/DynamycBuffer.h"

#define true 1
#define false 0

#define MY_PORT 49158
#define MAXBUF 1000
#define MAXDATA 10000
#define SYNRESIVED 1
#define FINRESIVED 2

volatile int Work = 1;
void  INThandler(int sig)
{
     char  c;
     signal(sig, SIG_IGN);
    
     printf("\nQuit? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y')
     {
		 Work = 0;
	 }
     else
     signal(SIGINT, INThandler);
}

int Compare(char* first,char *second)
{
	if(strlen(first) != strlen(second))
	{
		return 0;
	}
	for(int i = 0;i<strlen(first);++i)
	{
		if(first[i] != second[i])
		{
			return 0;
		}
	}
	return 1;
}

void* Req_Accept(void* clientfd)
{
	struct DynamycBuffer Dev;
	DynamycBuffer_Init(&Dev,MAXBUF);
	int Status = 0;
	//int idx = 0;
	SHA256_CTX hash;
    SHA256_Init(&hash);

	unsigned char h_buffer[32];
	
	while(1)
	{
		int result = 0;
		char buffer[MAXBUF];
		memset(buffer,'\0',MAXBUF);
		if((result = recv(*(int*)clientfd, buffer, MAXBUF, 0)) == -1)
		{
			
			printf("Error Reciving Data!!!\n");
			DynamycBuffer_Free(&Dev);
			close(*(int*)clientfd);
			free(clientfd);
			pthread_exit(0);
			return 0;
		}
		DynamycBuffer_Push_Back(&Dev,buffer,result);
		if(Dev.size >= 3 && Compare("SYN",Dev.Buff))
		{
			DynamycBuffer_Pop_Front(&Dev,3);
			Status = SYNRESIVED;
			printf("Starting to Receive Data!!!\n");
			if(3 != send(*(int*)clientfd,"SYN",3,0))
			{
				printf("Error Sending SYN!!!\n");
				DynamycBuffer_Free(&Dev);
				close(*(int*)clientfd);
				free(clientfd);
				pthread_exit(0);
				return 0;
			}
			continue;
		}
		if(Dev.size >=3 && Compare("FIN",buffer))
		{
			DynamycBuffer_Pop_Front(&Dev,3);
			Status = FINRESIVED;
			printf("Data Receiving Finished!!!\n");
		}
		if(!DynamycBuffer_Empty(&Dev) && Status == SYNRESIVED)
		{   
			if(2 != send(*(int*)clientfd,"OK",2,0))
			{
				printf("Error Sending OK!!!\n");
				DynamycBuffer_Free(&Dev);
				close(*(int*)clientfd);
				free(clientfd);
				pthread_exit(0);
				return 0;
			}
			if(Dev.size == MAXBUF)
			{
				SHA256_Update(&hash,(void*)Dev.Buff,MAXBUF);
				DynamycBuffer_Pop_Front(&Dev,MAXBUF);
			}
			else
			{
				SHA256_Update(&hash,(void*)Dev.Buff,Dev.size-1);
				DynamycBuffer_Pop_Front(&Dev,Dev.size);
			}
		}
		if(Status == FINRESIVED)
		{  
			printf("%s",Dev.Buff);
			SHA256_Final(h_buffer,&hash);
			if(32 != send(*(int*)clientfd,h_buffer, sizeof(h_buffer), 0))
			{
				printf("Error Sending Hash!!!\n");
				DynamycBuffer_Free(&Dev);
				close(*(int*)clientfd);
				free(clientfd);
				pthread_exit(0);
				return 0;
			}
			printf("%s","Sending Hash Message of Received Data:\n");
			printf("%s","------------------------------------------------------------------\n ");
			for(int i=0;i<32;++i)
			{
				printf("%x",h_buffer[i]);
			}
			printf("%s","\n------------------------------------------------------------------\n");
			close(*(int*)clientfd);
			DynamycBuffer_Free(&Dev);
			free(clientfd);
			pthread_exit(0);
			return 0;
		}
		printf("Data Received: %d Bites\n",(int)strlen(buffer));
	}
	close(*(int*)clientfd);
	DynamycBuffer_Free(&Dev);
	free(clientfd);
	pthread_exit(0);
	return 0;
}


void* SockInit(void *sockfd)
{
	struct sockaddr_in *self = malloc(sizeof(struct sockaddr_in));
    int iSetOption = 1;
    if ( (*(int*)sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Socket");
    }
    setsockopt(*(int*)sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,
                    sizeof(iSetOption));
    bzero(self, sizeof(*self));
    self->sin_family = AF_INET;
    self->sin_port = htons(MY_PORT);
    self->sin_addr.s_addr = INADDR_ANY;
    return self;
}

int main()
{   
	signal(SIGINT,INThandler);
	pthread_t thread_id;
	int sockfd;
    struct sockaddr_in *self = SockInit(&sockfd);
    if ( bind(sockfd, (struct sockaddr*)self, sizeof(*self)) != 0 )
    {
        perror("socket--bind");
    }
    if ( listen(sockfd, 20) != 0 )
	{
		perror("socket--listen");
	}
	struct sockaddr_in client_addr;
	int addrlen=sizeof(client_addr);
    while(Work)
    {
		int clientfd = accept(sockfd,(struct sockaddr*)&client_addr,(socklen_t*)&addrlen);
		int *temp = malloc(sizeof(int));
		*temp=clientfd;
		if(pthread_create( &thread_id , NULL , Req_Accept, (void*)temp) < 0)
        {
            perror("Could not create thread");
            return 1;
        }
        if(pthread_detach(thread_id))
        {
			perror("Could not Detach thread");
            return 1;
		}
		
    }
    close(sockfd);
    free(self);
    return 0;
}
