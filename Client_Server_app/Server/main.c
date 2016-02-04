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

#define true 1
#define false 0

#define MY_PORT 1337
#define MAXBUF 1000
#define MAXDATA 10000

/*
    Create an SHA256_CTX object.
    Initialize it with sha256_init().
    Read some/all of the data to hash into an array, calculate the size of the data, and add it to the hash with sha256_update().
    Repeat the previous step for all the data you want to hash.
    Finalize and output the hash with sha256_final().
*/


#define uchar unsigned char // 8-bit byte
#define uint unsigned int // 32-bit word

// DBL_INT_ADD treats two unsigned ints a and b as one 64-bit integer and adds c to it
#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

typedef struct {
   uchar data[64];
   uint datalen;
   uint bitlen[2];
   uint state[8];
} SHA256_CTX;

uint k[64] = {
   0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
   0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
   0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
   0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
   0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
   0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
   0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
   0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};


void sha256_transform(SHA256_CTX *ctx, uchar data[])
{  
   uint a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
      
   for (i=0,j=0; i < 16; ++i, j += 4)
      m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
   for ( ; i < 64; ++i)
      m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

   a = ctx->state[0];
   b = ctx->state[1];
   c = ctx->state[2];
   d = ctx->state[3];
   e = ctx->state[4];
   f = ctx->state[5];
   g = ctx->state[6];
   h = ctx->state[7];
   
   for (i = 0; i < 64; ++i) {
      t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
      t2 = EP0(a) + MAJ(a,b,c);
      h = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
   }
   
   ctx->state[0] += a;
   ctx->state[1] += b;
   ctx->state[2] += c;
   ctx->state[3] += d;
   ctx->state[4] += e;
   ctx->state[5] += f;
   ctx->state[6] += g;
   ctx->state[7] += h;
}  

void sha256_init(SHA256_CTX *ctx)
{  
   ctx->datalen = 0; 
   ctx->bitlen[0] = 0; 
   ctx->bitlen[1] = 0; 
   ctx->state[0] = 0x6a09e667;
   ctx->state[1] = 0xbb67ae85;
   ctx->state[2] = 0x3c6ef372;
   ctx->state[3] = 0xa54ff53a;
   ctx->state[4] = 0x510e527f;
   ctx->state[5] = 0x9b05688c;
   ctx->state[6] = 0x1f83d9ab;
   ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, uchar data[], uint len)
{  
   uint i;
   
   for (i=0; i < len; ++i) { 
      ctx->data[ctx->datalen] = data[i]; 
      ctx->datalen++; 
      if (ctx->datalen == 64) { 
         sha256_transform(ctx,ctx->data);
         DBL_INT_ADD(ctx->bitlen[0],ctx->bitlen[1],512); 
         ctx->datalen = 0; 
      }  
   }  
}  

void sha256_final(SHA256_CTX *ctx, uchar hash[])
{  
   uint i; 
   
   i = ctx->datalen; 
   
   // Pad whatever data is left in the buffer. 
   if (ctx->datalen < 56) { 
      ctx->data[i++] = 0x80; 
      while (i < 56) 
         ctx->data[i++] = 0x00; 
   }  
   else { 
      ctx->data[i++] = 0x80; 
      while (i < 64) 
         ctx->data[i++] = 0x00; 
      sha256_transform(ctx,ctx->data);
      memset(ctx->data,0,56); 
   }  
   
   // Append to the padding the total message's length in bits and transform. 
   DBL_INT_ADD(ctx->bitlen[0],ctx->bitlen[1],ctx->datalen * 8);
   ctx->data[63] = ctx->bitlen[0]; 
   ctx->data[62] = ctx->bitlen[0] >> 8; 
   ctx->data[61] = ctx->bitlen[0] >> 16; 
   ctx->data[60] = ctx->bitlen[0] >> 24; 
   ctx->data[59] = ctx->bitlen[1]; 
   ctx->data[58] = ctx->bitlen[1] >> 8; 
   ctx->data[57] = ctx->bitlen[1] >> 16;  
   ctx->data[56] = ctx->bitlen[1] >> 24; 
   sha256_transform(ctx,ctx->data);
   
   // Since this implementation uses little endian byte ordering and SHA uses big endian,
   // reverse all the bytes when copying the final state to the output hash. 
   for (i=0; i < 4; ++i) { 
      hash[i]    = (ctx->state[0] >> (24-i*8)) & 0x000000ff; 
      hash[i+4]  = (ctx->state[1] >> (24-i*8)) & 0x000000ff; 
      hash[i+8]  = (ctx->state[2] >> (24-i*8)) & 0x000000ff;
      hash[i+12] = (ctx->state[3] >> (24-i*8)) & 0x000000ff;
      hash[i+16] = (ctx->state[4] >> (24-i*8)) & 0x000000ff;
      hash[i+20] = (ctx->state[5] >> (24-i*8)) & 0x000000ff;
      hash[i+24] = (ctx->state[6] >> (24-i*8)) & 0x000000ff;
      hash[i+28] = (ctx->state[7] >> (24-i*8)) & 0x000000ff;
   }  
}  
/*
void convert (void *first,void *second,unsigned int size)
{
    unsigned char *f_ptr = first;
    unsigned char *s_ptr = second;
    for(int i=0;i<size;++i)
    {
        s_ptr[i] = f_ptr[i];
    }
}*/

void  INThandler(int sig)
{
     char  c;
     signal(sig, SIG_IGN);
     printf("OUCH, did you hit Ctrl-C?\n"
            "Do you really want to quit? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y')
          exit(0);
     else
     signal(SIGINT, INThandler);
}

int Req_Accept(int clientfd,int sockfd)
{
	signal(SIGINT, INThandler);
	int req = 0;
	//int idx = 0;
	char m_buf[MAXDATA];
	memset(m_buf,'\0',MAXDATA);
	SHA256_CTX hash;
	sha256_init(&hash);
	uchar h_buffer[32];
	while(1)
	{   
		char buffer[MAXBUF];
		memset(buffer,'\0',MAXBUF);
		//unsigned int sz = 
		recv(clientfd, buffer, MAXBUF, 0);
		printf("\n%s\n",buffer);
		if(buffer[0] == 'S'&&buffer[1]=='Y'&&buffer[2] == 'N')
		{
			req = 1;
			printf("\n\nSYN\n\n");
			continue;
		}
		if(buffer[0] == 'F'&&buffer[1]=='I'&&buffer[2] == 'N')
		{
			req = 2;
			printf("\n\nFIN\n\n");
			//scanf("%s",buffer);
		}
		if(req == 1)
		{   
			/*int k = 0;
			while(k < MAXBUF && buffer[k] != '\0')
			{
			m_buf[idx++] = buffer[k++];
			}
			*/
			printf("\nStrlen: %d\n",strlen(buffer));
			if(strlen(buffer)==MAXBUF)
			sha256_update(&hash,buffer,MAXBUF);
			else if(strlen(buffer) > 0) //////////////////////////////////////////////ERRRROOORRRRR//////////////////////////////
			sha256_update(&hash,buffer,strlen(buffer)-1);
		}
		if(req == 2)
		{  
			printf("%s",m_buf);
			sha256_final(&hash,h_buffer);	
			send(clientfd,h_buffer, sizeof(h_buffer), 0);
			printf("%s","\nMessage Hash: ");
			for(int i=0;i<32;++i)
			{
				printf("%x",h_buffer[i]);
			}
			printf("\n");
			//req = 0;
			close(clientfd);
			close(sockfd);
			//break;
			//return 0;
		}
		if(req == 2)
		{
			req = 0;
			close(clientfd);
			close(sockfd);
			return 0;
			//break;
		}
		printf("\n--------------------------------------------------------------\n");
	}
}

int main()
{   
    int sockfd;
    struct sockaddr_in self;
    int iSetOption = 1;
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("Socket");
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,
                    sizeof(iSetOption));
    bzero(&self, sizeof(self));
    self.sin_family = AF_INET;
    self.sin_port = htons(MY_PORT);
    self.sin_addr.s_addr = INADDR_ANY;

    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
    {
        perror("socket--bind");
    }
    while(1)
    {
		if ( listen(sockfd, 20) != 0 )
		{
			perror("socket--listen");
		}
		struct sockaddr_in client_addr;
		int addrlen=sizeof(client_addr);
		int clientfd = accept(sockfd,(struct sockaddr*)&client_addr, &addrlen);
		Req_Accept(clientfd,sockfd);
		return 0;
    }
    //
    return 0;
}
