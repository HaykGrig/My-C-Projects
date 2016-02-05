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

int main()
{
    int sockfd, numbytes;
    init(&sockfd);
    char ACK[3];
	char in_buf[MAXDATA];
	char buf[MAXDATASIZE];
	buf_init(in_buf);
	unsigned int index = 0;
    unsigned int size = strlen(in_buf);
    if(3 != send(sockfd,"SYN",3,0))
    {
		printf("Error Sending SYN!!!\n");
	}
    if(recv(sockfd, ACK, 3,0) <= 0)
    {
		printf("Error Sending SYN!!!\n");
	}
    if(!Compare(ACK,"SYN"))
    {
		printf("%s","Error Reciving ACK - SYN\n ");
		return 1;
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
		if(MAXDATASIZE != send(sockfd,buf,MAXDATASIZE,0))
		{
			printf("%s","Error Sending Data\n ");
		}
		if(2 != recv(sockfd, ACK, 2,0) && Compare(ACK,"OK"))
		{
			printf("%s","Error Reciving ACK - OK\n ");
			return 1;
		}
	}
    if(send(sockfd,"FIN",3,0) != 3)
    {
		printf("%s","Error Sending FIN\n ");
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
    close(sockfd);
    return 0;
}
