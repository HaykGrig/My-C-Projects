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

#define PORT  1337
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
	for(int i = 0;i<strlen(first);++i)
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

struct R_Buffer
{
	char *Buff;
	unsigned long int size;
};

void R_Buf_Init(struct R_Buffer* obj)
{
	obj->Buff = malloc(MAXDATA);
	memset(obj->Buff,'\0',MAXDATA);
	obj->size = 0;
}

void R_Buf_Free(struct R_Buffer* obj)
{
	free(obj->Buff);
	obj->size = 0;
	obj->Buff = 0;
}
void R_Buf_Push_Back(struct R_Buffer* obj,char *msg,unsigned long int len)
{
	for(int i=0;i<len;++i)
	{
		obj->Buff[obj->size++] = msg[i];
	}
}

void R_Buf_Pop_Front(struct R_Buffer* obj,unsigned long int len)
{
	//printf("Delete Str: %s len: %d\n",obj->Buff,len);
	char *temp = malloc(MAXDATA);
	memset(temp,'\0',MAXDATA);
	for(int i=0,j=len;j<MAXDATA;++i)
	{
		temp[i] = obj->Buff[j++]; 
	}
	obj->size -= len;
	free(obj->Buff);
	obj->Buff = temp;
}
int R_Buf_Empty(struct R_Buffer* obj)
{
	if(obj->size == 0)
	return 1;
	return 0;	
}

int main()
{
	struct R_Buffer Dev;
	R_Buf_Init(&Dev);
    int sockfd, numbytes;
    init(&sockfd);
    char ACK[3];
	char in_buf[MAXDATA];
	char buf[MAXDATASIZE];
	buf_init(in_buf);
	unsigned int index = 0;
    unsigned int size = strlen(in_buf);
    int result = 0;
    if(3 != send(sockfd,"SYN",3,0))
    {
		printf("Error Sending SYN!!!\n");
	}
    while(Dev.size < 3)
    {
		if((result = recv(sockfd, ACK, 3,0)) == -1)
		{
			printf("Error Sending SYN!!!\n");
			return 1;
		}
		R_Buf_Push_Back(&Dev,ACK,result);
	}
    if(!Compare(Dev.Buff,"SYN"))
    {
		printf("%s","Error Reciving ACK - SYN\n ");
		return 1;
	}
	else
	{
		R_Buf_Pop_Front(&Dev,3);
	}
    while(index != size)
	{
		int p = 0;
		memset(buf,'\0',MAXDATASIZE);
		while(p != MAXDATASIZE && index != size)
		{
			buf[p] = in_buf[index];
			++p;
			++index;
		}
		if(strlen(buf) != send(sockfd,buf,strlen(buf),0))
		{
			printf("%s","Error Sending Data\n ");
		}
		while(Dev.size < 2)
		{
			if((result = recv(sockfd, ACK, 2,0)) == -1)
			{
				R_Buf_Free(&Dev);
				return 1;
			}
			R_Buf_Push_Back(&Dev,ACK,result);
		}
		if(!Compare(Dev.Buff,"OK"))
		{
			printf("%s","Error Reciving ACK - OK\n ");
			R_Buf_Free(&Dev);
			return 1;
		}
		else
		{
			R_Buf_Pop_Front(&Dev,2);
		}
	}
    if(send(sockfd,"FIN",3,0) != 3)
    {
		printf("%s","Error Sending FIN\n ");
		R_Buf_Free(&Dev);
		return 1;
	}
    unsigned char h_buf[32];
    if((numbytes = recv(sockfd,h_buf,32, 0)) == -1)
    {
        perror("recv()");
        exit(1);
    }
    for(int i=0;i<32;++i)
    {
        printf("%x",h_buf[i]);
    }
    printf("\n");
    R_Buf_Free(&Dev);
    close(sockfd);
    return 0;
}
