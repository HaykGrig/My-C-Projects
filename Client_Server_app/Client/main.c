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

struct DynamycBuffer
{
	char *Buff;
	unsigned long int size;
};

void DynamycBuffer_Init(struct DynamycBuffer* obj,unsigned int len)
{
	obj->Buff = malloc(len);
	memset(obj->Buff,'\0',len);
	obj->size = 0;
}

void DynamycBuffer_Free(struct DynamycBuffer* obj)
{
	free(obj->Buff);
	obj->size = 0;
	obj->Buff = 0;
}
void DynamycBuffer_Push_Back(struct DynamycBuffer* obj,char *msg,unsigned long int len)
{
	for(int i=0;i<len;++i)
	{
		obj->Buff[obj->size++] = msg[i];
	}
}

void DynamycBuffer_Pop_Front(struct DynamycBuffer* obj,unsigned long int len)
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
int DynamycBuffer_Empty(struct DynamycBuffer* obj)
{
	if(obj->size == 0)
	return 1;
	return 0;	
}

int main()
{
	//Init
	struct DynamycBuffer Dev;
	DynamycBuffer_Init(&Dev,MAXDATA);
    int sockfd, numbytes;
    init(&sockfd);
    char ACK[3];
	char in_buf[MAXDATA];
	char buf[MAXDATASIZE];
	buf_init(in_buf);
	unsigned int index = 0;
    unsigned int size = strlen(in_buf);
    
    /*Hash Buffer*/
    struct DynamycBuffer h_buf;
	unsigned char Hash_Buffer[32];
	/* */
    int result = 0;
    //Init_End
    
    if(3 != send(sockfd,"SYN",3,0))
    {
		printf("Error Sending SYN!!!\n");
	}
	int expected_bytes = 3;
    while(Dev.size < 3)
    {
		if((result = recv(sockfd, ACK, expected_bytes-=result,0)) == -1)
		{
			DynamycBuffer_Free(&Dev);
			printf("Error Sending SYN!!!\n");
			return 1;
		}
		DynamycBuffer_Push_Back(&Dev,ACK,result);
	}
    if(!Compare(Dev.Buff,"SYN"))
    {
		printf("%s","Error Reciving ACK - SYN\n ");
		return 1;
	}
	else
	{
		DynamycBuffer_Pop_Front(&Dev,3);
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
		expected_bytes = 2;
		while(Dev.size < 2)
		{
			result=0;
			if((result = recv(sockfd, ACK,expected_bytes-=result,0)) == -1)
			{
				DynamycBuffer_Free(&Dev);
				return 1;
			}
			DynamycBuffer_Push_Back(&Dev,ACK,result);
		}
		if(!Compare(Dev.Buff,"OK"))
		{
			printf("%s","Error Reciving ACK - OK\n ");
			DynamycBuffer_Free(&Dev);
			return 1;
		}
		else
		{
			DynamycBuffer_Pop_Front(&Dev,2);
		}
	}
    if(send(sockfd,"FIN",3,0) != 3)
    {
		printf("%s","Error Sending FIN\n ");
		DynamycBuffer_Free(&Dev);
		return 1;
	}
	
	DynamycBuffer_Init(&h_buf,32);
	expected_bytes = 32;
	
    while(h_buf.size<32)
    {
		if((numbytes = recv(sockfd, Hash_Buffer, expected_bytes-=numbytes,0)) == -1)
		{
			DynamycBuffer_Free(&h_buf);
			perror("recv()");
			exit(1);
		}
		DynamycBuffer_Push_Back(&h_buf,(void*)Hash_Buffer,numbytes);
	}
	/*Print Hash Sum*/
    for(int i=0;i<h_buf.size;++i)
    {
        printf("%x",(unsigned char)h_buf.Buff[i]);
    }
    
    printf("\n");
    DynamycBuffer_Free(&h_buf);
    DynamycBuffer_Free(&Dev);
    close(sockfd);
    return 0;
}
