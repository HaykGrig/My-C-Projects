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
