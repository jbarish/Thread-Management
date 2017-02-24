#include "main.h"
struct productInfo{
  int id;
  struct timeval timeStamp;
  int life;
	
 };
 
 int id = 0;
 product makeProduct(int seed){
	 product p= (product)malloc(sizeof(struct productInfo));
	 gettimeofday(&(p->timeStamp),NULL);
	 p->id = id;
	 id++;
	 srandom(seed);
	 p->life = random()%1024;
	 return p;
 }