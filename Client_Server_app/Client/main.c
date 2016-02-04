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
void convert (void *first,void *second,unsigned int size)
{
    unsigned char *f_ptr = first;
    unsigned char *s_ptr = second;
    for(int i=0;i<size;++i)
    {   
        s_ptr[i] = f_ptr[i];
    }
}


int main()
{
    int sockfd, numbytes;
    
    struct hostent *he;
    struct sockaddr_in their_addr;
    if((he=gethostbyname(HOST)) == NULL)
    {
        perror("gethostbyname()");
        exit(1);
    }
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket()");
        exit(1);
    }
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);
    if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        exit(1);
    }

		char in_buf[MAXDATA];
		char buf[MAXDATASIZE];
        memset(in_buf,'\0',MAXDATA);
        FILE *fl = fopen("request.txt","r");
        fgets (in_buf,MAXDATA,fl);
        //printf("%s",in_buf);
        fclose(fl);
       
        unsigned int size = strlen(in_buf);
        send(sockfd,"SYN",3,0);
        unsigned int index = 0;
        sleep(1);
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
            send(sockfd,buf,MAXDATASIZE,0);
        }
        sleep(1);
        send(sockfd,"FIN",3,0);
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
