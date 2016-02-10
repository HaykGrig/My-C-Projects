#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct DynamycBuffer
{
	char *Buff;
	unsigned long int Max_Size;
	unsigned long int size;
};
void DynamycBuffer_Init(struct DynamycBuffer* obj,unsigned int len)
{
	obj->Max_Size = len;
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
	char *temp = malloc(obj->Max_Size);
	memset(temp,'\0',obj->Max_Size);
	for(int i=0,j=len;j<obj->Max_Size;++i)
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
