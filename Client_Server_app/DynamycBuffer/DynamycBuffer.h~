struct DynamycBuffer
{
	char *Buff;
	unsigned long int Max_Size;
	unsigned long int size;
};
extern void DynamycBuffer_Init(struct DynamycBuffer* obj,unsigned int len);
extern void DynamycBuffer_Free(struct DynamycBuffer* obj);
extern void DynamycBuffer_Push_Back(struct DynamycBuffer* obj,char *msg,unsigned long int len);
extern void DynamycBuffer_Pop_Front(struct DynamycBuffer* obj,unsigned long int len);
extern int DynamycBuffer_Empty(struct DynamycBuffer* obj);
