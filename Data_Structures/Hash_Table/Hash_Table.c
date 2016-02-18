#include <stdio.h>
#include "Linked_List.h"

const int HT_DEFAULT = 16;
void LL_Erase(struct Linked_List *obj)
{
	struct Node* temp;
	while(obj->head)
	{
		temp = obj->head->next;
		free(obj->head);
		obj->head = temp;
	}
}

void LL_Init(struct Linked_List *obj)
{
	obj->head = 0;
	obj->tail = 0;
	obj->size = 0;
}

void LL_Push_Front(struct Linked_List *obj,int val)
{
	++obj->size;
	if(obj->head == 0)
	{
		obj->head = obj->tail = malloc(sizeof(struct Node));
		obj->head->data = val;
		obj->head->next = 0;
		return;
	}
	else
	{
		struct Node* temp = obj->head;
		obj->head = malloc(sizeof(struct Node));
		obj->head->next = temp;
		obj->head->data = val;
		return;
	}
}

void LL_Push_Back(struct Linked_List *obj,int val)
{
	++obj->size;
	if(obj->head == 0)
	{
		obj->head = obj->tail = malloc(sizeof(struct Node));
		obj->head->data = val;
		obj->head->next = 0;
		return;
	}
	else
	{
		obj->tail->next = malloc(sizeof(struct Node));
		obj->tail->next->data = val;
		obj->tail->next->next = 0;
		obj->tail = obj->tail->next;
		return;
	}
}

void LL_Pop_Front(struct Linked_List *obj)
{
	--obj->size;
	struct Node *temp = obj->head->next;
	free(obj->head);
	obj->head = temp;
}

int *LL_Front(struct Linked_List* obj)
{
	return &(obj->head->data);
}
int *LL_Back(struct Linked_List* obj)
{
	return &(obj->tail->data);
}

struct Hash_Table
{
	unsigned int size;
	unsigned int capacity;
	struct Linked_List* table;
};

void HT_Init(struct Hash_Table *obj,int cap)
{
	obj->size = 0;
	obj->capacity = cap;
	obj->table = malloc(obj->capacity*sizeof(struct Linked_List));
	for(int i=0;i<obj->capacity;++i)
	{
		LL_Init(&(obj->table[i]));
	}
}
void HT_Erase(struct Hash_Table *obj)
{
	obj->size = 0;
	for(int i=0;i<obj->capacity;++i)
	{
		LL_Erase(&(obj->table[i]));
	}
	obj->capacity = 0;
	free(obj->table);
}

void HT_Insert(struct Hash_Table*,int);

void HT_Rehashing(struct Hash_Table *obj)
{
	struct Hash_Table *temp = malloc(sizeof(struct Hash_Table));
	HT_Init(temp,obj->capacity*2);
	for(int i=0;i<obj->capacity;++i)
	{
		struct Node* temp_l_head = obj->table[i].head;
		while(temp_l_head)
		{
			printf("Data: %i\n",temp_l_head->data);
			HT_Insert(temp,temp_l_head->data);
			temp_l_head = temp_l_head->next;
		}
	}
	
	HT_Erase(obj);
	free(obj);
	obj = temp;
}


void HT_Insert(struct Hash_Table *obj,int val)
{
	
	/*if((unsigned int)(obj->size*1.75) >= obj->capacity)
	{
		HT_Rehashing(obj);
	}*/
	++obj->size;
	unsigned int position = val%obj->capacity;
	LL_Push_Front(&(obj->table[position]),val);
}

void LL_Print(struct Linked_List *obj)
{
	struct Node *temp = obj->head;
	while(temp)
	{
		printf("%i ",temp->data);
		temp = temp->next;
	}
}

void HT_Print(struct Hash_Table *obj)
{
	for(int i=0;i<obj->capacity;++i)
	{
		printf("List[%i]: ",i);
		LL_Print(&(obj->table[i]));
		printf("\n");
	}
}

int main()
{
	struct Hash_Table *Hash = malloc(sizeof(struct Hash_Table));
	HT_Init(Hash,32);
	for(int i=0;i<50;++i)
	HT_Insert(Hash,i);
	HT_Print(Hash);
	HT_Erase(Hash);
	free(Hash);
	return 0;
}
