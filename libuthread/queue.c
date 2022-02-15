#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"


struct node
{	
	void *datas;
	struct node *next;
	
};


struct queue {
	struct node *start, *end ;
	int sizeOfNode;
};

queue_t queue_create(void)
{
	queue_t QueueP1 = malloc(sizeof(struct queue));

	QueueP1->start = QueueP1->end = NULL; //init
	QueueP1->sizeOfNode = 0; //size == 0 at beginning
	
	return QueueP1;
}

int queue_destroy(queue_t queue)
{
	if (queue->sizeOfNode >0) return -1; //check if empty
	if (queue == NULL) return -1; //check if == NULL

	free(queue); //Deallocate the memory
	return 0;
}


//check if empty, or we connect it to the header

bool ifQEmpty (queue_t Qu){
	return queue_length(Qu) == 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if (queue == NULL ) return -1;
	if (data == NULL) return -1;

	struct node *NewNode = malloc(sizeof(struct node));

	if (NewNode == NULL) return -1; // in case of memory allocation error

	NewNode->datas = data; 
	NewNode->next = NULL;

	if( ifQEmpty(queue)){
		queue->start = NewNode;
		queue->end = NewNode;

		queue->sizeOfNode += 1;
		return 0;
	}

	else{
		queue->end->next = NewNode; // add New Queue at back of the old one
		queue->end = NewNode;
		queue->sizeOfNode += 1;
		NewNode->next = NULL;

		return 0;
	}
	
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL) return -1;
	if (data == NULL) return -1;
	if (ifQEmpty(queue)) return -1;

	//remove and replace the oldest so we set start to the data
	*data = queue->start->datas; 
	queue->start = queue->start->next;

	if (ifQEmpty(queue)) queue->end = NULL;
	
	queue->sizeOfNode -= 1;//reduce the size
	return 0;
}

int queue_delete(queue_t queue, void *data) 
{
	if (queue == NULL) return -1;
	if (data == NULL) return -1;
	struct node *findRep; //for iterate through
	
	findRep = queue->start;
	

	while(findRep != NULL){
		if(findRep->datas == data){ // if the first element is delect one
			queue_dequeue(queue, &data);
			break;
		}
		

		if(findRep->next == NULL && findRep->next->datas != data){ //didn't find 
			return -1;
		}

		//actually delect the element found
		if(findRep->next->datas == data){
			struct node *temp;
			temp = findRep -> next;
			//since we need to pass(delect) next element
			findRep->next =	findRep->next->next; 

			if(temp == NULL){
				queue->end = findRep;
			}
			queue->sizeOfNode -= 1;
			break;
		}

		findRep = findRep->next;

	}

	free(findRep);
	return 0;

}


//go throught and give call back
int queue_iterate(queue_t queue, queue_func_t func)
{
		if (queue == NULL) return -1;
		if (func == NULL) return -1;

		struct node *QCp;
		QCp = queue->start;
		struct node *React;
		React = NULL; //Since queue do not have *next, we need to create a new linked list for copy

		while(QCp != NULL){
			
			func(QCp->datas);
			React = QCp->next;
			//free(QCp);
			if (QCp != NULL){ //clear the QCp and redirect it
				QCp = NULL; 
			}
			QCp = React;


		}
		return 0;

}

int queue_length(queue_t queue)
{
	if(queue == NULL)return -1;

	return queue->sizeOfNode;
}
