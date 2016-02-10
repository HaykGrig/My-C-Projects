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
#include "../DynamycBuffer/DynamycBuffer.h"

#define true 1
#define false 0

#define MY_PORT 49158
#define MAXBUF 1000
#define MAXDATA 10000
#define SYNRESIVED 1
#define FINRESIVED 2

volatile int Work = 1;
int sockfd;
struct DynamycBuffer Dev;
void *ptr;

void  INThandler(int sig)
{
     char  c;
     signal(sig, SIG_IGN);
    
     printf("\nQuit? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y')
     {
		 Work = 0;
		 close(sockfd);
		 free(ptr);
		 DynamycBuffer_Free(&Dev);
		 exit(0);
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

int Req_Accept(int clientfd,int sockfd)
{

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
		if((result = recv(clientfd, buffer, MAXBUF, 0)) == -1)
		{
			DynamycBuffer_Free(&Dev);
			printf("Error Reciving Data!!!\n");
			return 1;
		}
		DynamycBuffer_Push_Back(&Dev,buffer,result);
		if(Dev.size >= 3 && Compare("SYN",Dev.Buff))
		{
			DynamycBuffer_Pop_Front(&Dev,3);
			Status = SYNRESIVED;
			printf("Starting to Receive Data!!!\n");
			if(3 != send(clientfd,"SYN",3,0))
			{
				printf("Error Sending SYN!!!\n");
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
			if(2 != send(clientfd,"OK",2,0))
			{
				printf("Error Sending OK!!!\n");
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
			if(32 != send(clientfd,h_buffer, sizeof(h_buffer), 0))
			{
				printf("Error Sending Hash!!!\n");
			}
			printf("%s","Sending Hash Message of Received Data:\n");
			printf("%s","------------------------------------------------------------------\n ");
			for(int i=0;i<32;++i)
			{
				printf("%x",h_buffer[i]);
			}
			printf("%s","\n------------------------------------------------------------------\n");
			DynamycBuffer_Free(&Dev);
			return 0;
		}
		printf("Data Received: %d Bites\n",(int)strlen(buffer));
	}
	DynamycBuffer_Free(&Dev);
}

void* SockInit(int *sockfd)
{
	struct sockaddr_in *self = malloc(sizeof(struct sockaddr_in));
    int iSetOption = 1;
    if ( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Socket");
    }
    setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,
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
    struct sockaddr_in *self = ptr = SockInit(&sockfd);
    if ( bind(sockfd, (struct sockaddr*)self, sizeof(*self)) != 0 )
    {
        perror("socket--bind");
    }
    if ( listen(sockfd, 20) != 0 )
	{
		perror("socket--listen");
	}
    while(Work) // listening the 1337 port
    {
		struct sockaddr_in client_addr;
		int addrlen=sizeof(client_addr);
		
		int clientfd = accept(sockfd,(struct sockaddr*)&client_addr,(socklen_t*)&addrlen);
		Req_Accept(clientfd,sockfd);
		close(clientfd);
    }
    close(sockfd);
    free(self);
    return 0;
}
