#include <stdio.h>
#include <stdlib.h>

const int shift = 4;

char Rotate_Right(char letter,int key)
{
	while(key > 26)
	{
		key-=26;
	}
	if(letter >= 97 && letter <= 122)
	{
		if(letter+key <= 122)
		{
			letter += key;
		}
		else
		{
			if(letter+key == 123)
			{
				letter = 97;
			}
			else
			{
				letter = letter+key-26 ;
			}
		}
	}
	else
	if(letter >= 65 && letter <= 90)
	{
		if(letter+key <= 90)
		{
			letter += key;
		}
		else
		{
			if(letter+key == 91)
			{
				letter = 65;
			}
			else
			{
				letter = letter+key-26 ;
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

int main()
{
	FILE *fl = fopen("Source.txt","r");
	
	unsigned int size = Get_Size(fl)+1;
	
	char *source = malloc(size);
	char *destination = malloc(size);
	fgets(source,size,fl);
	for(int i=0;i<size;++i)
	{
		destination[i] = Rotate_Right(source[i],shift);
	}
	fclose(fl);
	fl = fopen("Destination.txt","w");
	fputs(destination,fl);
	fclose(fl);
	free(source);
	free(destination);
}
