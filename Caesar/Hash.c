#include <openssl/sha.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
const int shift = 4;

char Rotate_Left(char letter,int key)
{ 
	while(key > 26)
	{
		key-=26;
	}
	if(letter >= 97 && letter <= 122)
	{
		if(letter-key >= 97)
		{
			letter -= key;
		}
		else
		{
			if(letter-key == 96)
			{
				letter = 122;
			}
			else
			{
				letter=122+(letter-key-96);
			}
		}
	}
	else
	if(letter >= 65 && letter <= 90)
	{
		if(letter-key >= 65)
		{
			letter -= key;
		}
		else
		{
			if(letter-key == 65)
			{
				letter = 90;
			}
			else
			{
				letter=90+(letter-key-64);
			}
		}
	}
	
	return letter;
}

unsigned int Get_Size(FILE *fl)
{
	fseek(fl, 0L, SEEK_END);
	unsigned int size = ftell(fl);
	fseek(fl, 0L, SEEK_SET);
	return size;
}

char *source_init(FILE *fl,unsigned int *size)
{
	*size = Get_Size(fl)+1;
	char *source = malloc(*size);
	fgets(source,*size,fl);
	return source;
}

void Decrypt(char *source,char *buf)
{
	printf("Decrypted Message: ");
	int i=0;
	for(;i<strlen(source);++i)
	{
		printf("%c",(buf[i] = Rotate_Left(source[i],shift)));
	}
	buf[i] = '\0';
}

void Write_In_File(unsigned char* h_buffer,char *source,char *buf,FILE *fl)
{
	fputs("Message: ",fl);
	fputs(buf,fl);
	fputs("\nCaesar: ",fl);
	fputs(source,fl);
	fputs("\nHash: ",fl);
	printf("\nHash: ");
	for(int i=0;i<32;++i)
	{
		printf("%X",h_buffer[i]);
		fprintf(fl,"%x",h_buffer[i]);
	}
	printf("\n");
}

int main()
{
	unsigned char h_buffer[32];
	SHA256_CTX hash;
	SHA256_Init(&hash);
	unsigned int size;
	FILE *fl = fopen("Destination.txt","r");
	char *source=source_init(fl,&size);
	fclose(fl);
	fl = fopen("Text+Hash.txt","w");
	SHA256_Update(&hash,source,size);
	SHA256_Final(h_buffer,&hash);
	char buf[strlen(source)];
	Decrypt(source,buf);
	Write_In_File(h_buffer,source,buf,fl);
	
	fclose(fl);
	free(source);
}
