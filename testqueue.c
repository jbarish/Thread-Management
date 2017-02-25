#include "main.h"

struct productInfo{
  int id;
  struct timeval timeStamp;
  int life;
	
 };
typedef struct productInfo* product;

int id = 0;
int seed;
product makeProduct(){
	 product p= (product)malloc(sizeof(struct productInfo));
	 gettimeofday(&(p->timeStamp),NULL);
	 p->id = id;
	 id++;
	 srandom(seed);
	 p->life = random()%1024;
	 return p;
 }

int main(){
  product a = makeProduct();
  product b = makeProduct();
  product c = makeProduct();
  product d = makeProduct();
  product e = makeProduct();
  
  printf("a = %i\nb = %i\nc = %i\nd = %i\ne = %i\n", a->id, b->id, c->id, d->id, e->id);
  printf("test enqueue");
  enqueue(a);
  enqueue(b);
  enqueue(c);
  enqueue(d);
  enqueue(e);
  printQueue();

  dequeueFirst();
  dequeueFirst();
  dequeueFirst();
  dequeueFirst();
  dequeueFirst();
  
  enqueue(a);
  dequeueFirst();

  
}
