#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct DynamicBuffer
{
	char *Buff;
	unsigned long int Max_Size;
	unsigned long int size;
};
void DynamicBuffer_Init(struct DynamicBuffer* obj,unsigned int len)
{
	obj->Max_Size = len;
	obj->Buff = malloc(len);
	memset(obj->Buff,'\0',len);
	obj->size = 0;
}

void DynamicBuffer_Free(struct DynamicBuffer* obj)
{
	free(obj->Buff);
	obj->size = 0;
	obj->Buff = 0;
}
void DynamicBuffer_Push_Back(struct DynamicBuffer* obj,char *msg,unsigned long int len)
{
	int i;
	for(i=0;i<len;++i)
	{
		obj->Buff[obj->size++] = msg[i];
	}
}

void DynamicBuffer_Pop_Front(struct DynamicBuffer* obj,unsigned long int len)
{
	//printf("Delete Str: %s len: %d\n",obj->Buff,len);
	char *temp = malloc(obj->Max_Size);
	memset(temp,'\0',obj->Max_Size);
	int i,j;
	for(i=0,j=len;j<obj->Max_Size;++i)
	{
		temp[i] = obj->Buff[j++]; 
	}
	obj->size -= len;
	free(obj->Buff);
	obj->Buff = temp;
}
int DynamicBuffer_Empty(struct DynamicBuffer* obj)
{
	if(obj->size == 0)
	return 1;
	return 0;	
}
