#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include "../DynamicBuffer/DynamicBuffer.h"
#define PORT  49158
#define HOST "localhost"

#define MAXDATASIZE 1000 
#define MAXDATA 10000


struct sockaddr_in their_addr;



int Compare(char* first,char *second)
{
	if(strlen(first) != strlen(second))
	{
		return 0;
	}
	int i;
	for(i = 0;i<strlen(first);++i)
	{
		if(first[i] != second[i])
		{
			return 0;
		}
	}
	return 1;
}

void init(int *sockfd)
{
	struct hostent *he;
    
    if((he=gethostbyname(HOST)) == NULL)
    {
        perror("gethostbyname()");
        exit(1);
    }
    if((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket()");
        exit(1);
    }
    
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);
    
    if(connect(*sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        exit(1);
    }
}

void buf_init(char *in_buf)
{        
	memset(in_buf,'\0',MAXDATA);
    FILE *fl = fopen("request.txt","r");
    fgets (in_buf,MAXDATA,fl);
    fclose(fl);
}

int Recv_ACK(int expected_bytes,char *ACK,int sockfd,struct DynamicBuffer *Dev)
{
	int result = 0;
	int exp_bytes = expected_bytes;
    while(Dev->size < expected_bytes)
    {
		if((result = recv(sockfd, ACK, exp_bytes-=result,0)) == -1)
		{
			printf("Error Sending SYN!!!\n");
			return 1;
		}
		DynamicBuffer_Push_Back(Dev,ACK,result);
	}
	return 0;
}

int main()
{
	//Init
	struct DynamicBuffer Dev;
	DynamicBuffer_Init(&Dev,MAXDATASIZE);
    int sockfd;
    init(&sockfd);
    char ACK[3];
	char in_buf[MAXDATA];
	char buf[MAXDATASIZE];
	buf_init(in_buf);
	unsigned int index = 0;
    unsigned int size = strlen(in_buf);
    
    /*Hash Buffer*/
    struct DynamicBuffer h_buf;
	unsigned char Hash_Buffer[32];
	/* */
    
    if(3 != send(sockfd,"SYN",3,0))
    {
		printf("Error Sending SYN!!!\n");
	}
	
	if(Recv_ACK(3,ACK,sockfd,&Dev))
	{
		DynamicBuffer_Free(&Dev);
		return 1;
	}
	
    if(!Compare(Dev.Buff,"SYN"))
    {
		printf("%s","Error Reciving ACK - SYN\n ");
		return 1;
	}
	else
	{
		DynamicBuffer_Pop_Front(&Dev,3);
	}
    while(index != size)
	{
		int cur_index = 0;
		memset(buf,'\0',MAXDATASIZE);
		while(cur_index != MAXDATASIZE && index != size)
		{
			buf[cur_index] = in_buf[index];
			++cur_index;
			++index;
		}
		if(strlen(buf) != send(sockfd,buf,strlen(buf),0))
		{
			printf("%s","Error Sending Data\n ");
		}
		if(Recv_ACK(2,ACK,sockfd,&Dev))
		{
			DynamicBuffer_Free(&Dev);
			return 1;
		}
		if(!Compare(Dev.Buff,"OK"))
		{
			printf("%s","Error Reciving ACK - OK\n ");
			DynamicBuffer_Free(&Dev);
			return 1;
		}
		else
		{
			DynamicBuffer_Pop_Front(&Dev,2);
		}
	}
    if(send(sockfd,"FIN",3,0) != 3)
    {
		printf("%s","Error Sending FIN\n ");
		DynamicBuffer_Free(&Dev);
		return 1;
	}
	DynamicBuffer_Init(&h_buf,32);
	if(Recv_ACK(32,(void*)Hash_Buffer,sockfd,&h_buf))
	{
		DynamicBuffer_Free(&h_buf);
		return 1;
	}
	/*Print Hash Sum*/
	int i;
    for(i=0;i<h_buf.size;++i)
    {
        printf("%x",(unsigned char)h_buf.Buff[i]);
    }
    printf("\n");
    DynamicBuffer_Free(&h_buf);
    DynamicBuffer_Free(&Dev);
    
    printf("Port number %d\n", ntohs(their_addr.sin_port));
    
    close(sockfd);
    return 0;
}
