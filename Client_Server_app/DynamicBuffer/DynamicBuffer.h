struct DynamicBuffer
{
	char *Buff;
	unsigned long int Max_Size;
	unsigned long int size;
};
extern void DynamicBuffer_Init(struct DynamicBuffer* obj,unsigned int len);
extern void DynamicBuffer_Free(struct DynamicBuffer* obj);
extern void DynamicBuffer_Push_Back(struct DynamicBuffer* obj,char *msg,unsigned long int len);
extern void DynamicBuffer_Pop_Front(struct DynamicBuffer* obj,unsigned long int len);
extern int DynamicBuffer_Empty(struct DynamicBuffer* obj);
