#ifndef MAIN_H
#define MAIN_H
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>

struct productInfo{
  int id;
  struct timeval timeStamp;
  int life;
	
 };
typedef struct productInfo* product;

product dequeueFirst();
void enqueue(product prod);
int getNumElements();
#endif