#include "main.h"
struct node{
	product p;
	struct node* next;
	struct node* prev;
};
typedef struct node* Node;
Node head = NULL;
Node tail = NULL;

/*
struct productInfo{
  int id;
  struct timeval timeStamp;
  int life;
	
 };
typedef struct productInfo* product;
*/

/*
 *  [tail]                   [head]
 *     |                       |
 *  |product  |   |product|   |product  |
 *  |prev=null|<--|=prev  |<--|=prev    |
 *  |next = --|-->|next=--|-->|next=null|
 *
 * Add to tail, remove from head with dequeueFirst
 */
 
/*num of elements currently in queue*/
int numElements = 0;


/*remove item at head
 *returns NULL if nothing in list
 */
product dequeueFirst(){
	if(head!=NULL){
		product pTemp = head->p;
		Node temp = head;
		if(head->prev!=NULL){
			temp->prev->next=NULL;
		}
		head=temp->prev;

		free(temp);
		numElements--;
		if(numElements==0){
			tail ==NULL;
		}
		return pTemp;
	}
	return NULL;
}
/*
void printHelp(product nxt){
  if(nxt->next != head){
    printf("%i\n", (nxt->p)->id);
    printHelp(nxt->next);
  }
}

void printQueue(){
  if(tail!=head){
    printf("%i\n", (tail->p)->id);
    printHelp(tail->next);
  }
  printf("%i\n", (head->p)->id);
}
*/
/*add to tail*/
void enqueue(product prod){
	if(head!=NULL){
		Node new = (Node)malloc(sizeof(struct node));
		new->p = prod;
		tail->prev = new;
		new->next = tail;
		tail = new;
		new->prev = NULL;
	}else{
		/*first thing in list*/
		Node new = (Node)malloc(sizeof(struct node));
		new->p = prod;
		new->prev = NULL;
		new->next=NULL;
		head = new;
		tail = new;
	}
	numElements++;
}

int getNumElements(){
	return numElements;
}

/*get node at index, with index 0 being the node at the head */
product peekAt(int index){
	if(index >numElements){
		return NULL;
	}
	Node curr = head;
	int i;
	for(i = 0; i< index;i++){
		curr = curr->prev;
	}
	return curr->p;
}

/*remove node at a specific index*/
product removeAt(int index){
	if(index >numElements){
		return NULL;
	}else if(index = 0){
		return dequeueFirst();
	}
	Node curr = head->prev;
	int i;
	for(i = 1; i< index;i++){
		curr = curr->prev;
	}
	product temp = curr->p;
	if(curr->prev!=NULL){
		(curr->prev)->next = curr->next;
	}else{
		tail = curr->next;
	}
	(curr->next)->prev =curr->prev;
	
	free(curr);
	return temp;
}
