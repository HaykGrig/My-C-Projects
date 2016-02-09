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



int main()
{
	unsigned char h_buffer[32];
	SHA256_CTX hash;
	SHA256_Init(&hash);
	FILE *fl = fopen("Destination.txt","r");
	fseek(fl, 0L, SEEK_END);
	unsigned int size = ftell(fl)+1;
	fseek(fl, 0L, SEEK_SET);
	char *source = malloc(size);
	fgets(source,size,fl);
	printf("Message: ");
	fclose(fl);
	fl = fopen("Text+Hash.txt","w");
	SHA256_Update(&hash,source,size);
	SHA256_Final(h_buffer,&hash);
	fputs("Message: ",fl);
	int i=0;
	char buf[strlen(source)];
	for(;i<strlen(source);++i)
	{
		printf("%c",(buf[i] = Rotate_Left(source[i],shift)));
	}
	buf[i] = '\0';
	fputs(buf,fl);
	printf("\n");
	fputs("\nCaesar: ",fl);
	fputs(source,fl);
	fputs("\nHash: ",fl);
	for(int i=0;i<32;++i)
	{
		printf("%X",h_buffer[i]);
		fprintf(fl,"%x",h_buffer[i]);
	}
	printf("\n");
	fclose(fl);
	free(source);
}
