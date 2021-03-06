#include "main.h"
#include <limits.h>

/*
 * Justin Barish
 * Jack Kraszewski
 *
 * We pledge our honor that we have abided by the Stevens Honor System 
 */

/*Globals */
int id = 0; // unique id for each product
int productTotal; // total number of products to produce, given as CL input
int currTotal = 0; //total items produced so far
int consumeTotal = 0;
int quantum; //given as CL input
int seed; //given as CL input, for random number generation
int maxBufferSize;//given as CL input
int schedType; //given as CL input
pthread_mutex_t theMutex;
pthread_cond_t condc, condp;
pthread_t* producerThreads;
pthread_t* consumerThreads;
int* pn; //keeps track of producer thread numbers
int* cn; //keeps track of consumer thread numbers

/*Variables for timings */
long turnTime = 0;
long waitTime = 0;
long minWaitTime = LONG_MAX;
long maxWaitTime = 0;
long minTurnTime = LONG_MAX;
long maxTurnTime = 0;
long pThrough = 0;
long cThrough = 0;


/*Forward Declarations */
product makeProduct();
void *producer(void *ptr);
void *consumer(void *ptr);
product makeProduct();
int fn();

/*Create and return a new product*/
product makeProduct(){
	product p= (product)malloc(sizeof(struct productInfo));
	gettimeofday(&(p->timeStamp),NULL);
	p->id = id;
	id++;
	p->consumeTime=0; //used for time statistics
	p->life = rand()%1024;
	return p;
 }

/*Calculate the 10th fibbonacci number iteratively */
int fn(){
  int cnt = 2;
  int n1 = 0;
  int n2 = 1;
  int newN = 0;
  while(cnt < 10){
    newN = n1 + n2;
    n1 = n2;
    n2 = newN;
    cnt++;
  }
  return newN;
}

int main(int argc, char *argv[]){
  
	int numPThreads;
	int numCThreads;
	struct timeval progStart;
	gettimeofday(&progStart,NULL);
	
	/* Must take in 7 arguments (excluding args[0])
	 *  P1: Number of producer threads
	 *  P2: Number of consumer threads
	 *  P3: Total number of products to be generated by all producer threads
	 *  P4: Size of the queue to store products for both producer and consumer threads
	        (0 for unlimited queue size)
	 *  P5: 0 or 1 for type of scheduling algorithm: 0 for First-Come-First-Serve, and 1
	        for Round-Robin
	 *  P6: Value of quantum used for round-robin scheduling
	 *  P7: Seed for random number generator
	 */
	if(argc < 8 ){
	  perror("Must provide 7 arguments"), exit(1);
	}
	if(atoi(argv[5])&& atoi(argv[6])<=0){
	  perror("Must provide valid quantum"), exit(1);
	}
	if(atoi(argv[4])<0){
	  perror("Must provide valid queue size of >=0"), exit(1);
	}
	if(atoi(argv[1]) <= 0){
	  perror("Must provide valid number of producers >= 1"), exit(1);
	}
	if(atoi(argv[2]) <= 0){
	  perror("Must provide valid number of consumers >= 1"), exit(1);
	}
	
	/*Extract from args*/
	numPThreads = atoi(argv[1]);
	numCThreads = atoi(argv[2]);
	seed = atoi(argv[7]);
	quantum=atoi(argv[6]);
	productTotal= atoi(argv[3]);
	maxBufferSize = atoi(argv[4]);
	schedType = atoi(argv[5]);
	
	/*Set up consumer/producer thread lists based on input sizes*/
	producerThreads = (pthread_t*)malloc(sizeof(pthread_t)*numPThreads);
	consumerThreads = (pthread_t*)malloc(sizeof(pthread_t)*numCThreads);
	pn = (int*)malloc(sizeof(int)*numPThreads);
	cn = (int*)malloc(sizeof(int)*numCThreads);
	
	/*initialize mutexes and condition variables*/
	pthread_mutex_init(&theMutex, 0);
	pthread_cond_init(&condc, 0);
	pthread_cond_init(&condp, 0);
	
	/*Set seed for random number generator*/
	srand(seed);
	
	/*create threads*/
	int i;
	for(i = 0; i<numPThreads; i++){
	  /*set up producer threads */
		pn[i] = i;
		pthread_create(&producerThreads[i], NULL, producer, &pn[i]);
		
	}
	for(i = 0; i<numCThreads; i++){
	  /*set up consumer threads */
		cn[i] = i;
		pthread_create(&consumerThreads[i], NULL, consumer, &cn[i]);
	}
	
	/*wait for all threads to exit*/
	for(i = 0; i<numPThreads; i++){
	  pthread_join(producerThreads[i], NULL);
	}
	for(i = 0; i<numCThreads; i++){
	  pthread_join(consumerThreads[i], NULL);
	}
	
	/*Times for analysis */
	struct timeval end;
	gettimeofday(&end, NULL);
	long secs_used = (end.tv_sec - progStart.tv_sec);
	long prog_micros_used = ((secs_used*1000000) + end.tv_usec) - (progStart.tv_usec);

	float pThroughput = productTotal/((float)pThrough/60000000) ;
	float cThroughput = productTotal/((float)cThrough/60000000) ;
	printf("\n------------------Runtime Information-------------------\n");
	printf("Total Processing time = %ld Microseconds.\n", prog_micros_used);
	
	printf("\nMinimum Turnaround Time = %ld Microseconds.\n", minTurnTime);
	printf("Maximum Turnaround Time = %ld Microseconds.\n", maxTurnTime);
	printf("Average Turnaround Time = %f Microseconds.\n", ((double)turnTime / productTotal));

	printf("\nMinimum Wait Time = %ld Microseconds.\n", minWaitTime);
	printf("Maximum Wait Time = %ld Microseconds.\n", maxWaitTime);
	printf("Average Wait Time = %f Microseconds.\n", ((double)waitTime / productTotal));

	printf("\nProducer Throughput = %f per minute.\n", pThroughput);
	printf("Consumer Throughput = %f per minute.\n\n", cThroughput);
	
	/*destroy condition variables and mutexes*/
	pthread_cond_destroy(&condc);
	pthread_cond_destroy(&condp);
	pthread_mutex_destroy(&theMutex);
	free(pn);
	free(cn);
    return 0;
} 

/*Producer thread, that produces an item, and places it into a queue. Sleeps 100ms after producing*/
void *producer(void *ptr){
	printf("Hello! I am producer %i.\n", *((int*)ptr));
	struct timeval pStart;
	struct timeval pEnd;
	gettimeofday(&pStart, NULL);
	while(1){
		
		/*obtain lock to protect the queue*/
		pthread_mutex_lock(&theMutex);
		
		/*if buffer is full (and not infinite), wait*/
		while(maxBufferSize && getNumElements()>=maxBufferSize)
			pthread_cond_wait(&condp, &theMutex);
		
		/*thread exit criteria, if already produced the number of requested products*/
		if(currTotal==productTotal){
			printf("Producer %i is exiting.\n", *((int*)ptr));
			
			/*time information for producer throughput speed*/
			gettimeofday(&pEnd, NULL);
			long secs_used = (pEnd.tv_sec -pStart.tv_sec);
			long micros_used = ((secs_used*1000000) + pEnd.tv_usec) - (pStart.tv_usec);
			pThrough+= micros_used;
			
			/*signal consumers to wake up if exiting 
			 *prevents sleeping consumers from being stuck and staying that
			 *way if all producers exit before waking them up
			 */
			pthread_cond_broadcast(&condc);
			pthread_mutex_unlock(&theMutex);
			
			pthread_exit(0);
		}
		
		/*make and enqueue a new product*/
		product tempProduct = makeProduct();
		enqueue(tempProduct);
		printf("Producer %i has produced product %i, which has lifespan of %i.\n", *((int*)(ptr)), tempProduct->id, tempProduct->life);
		currTotal++;
		
		/*signal consumers that there is an item to consume, and unlock the mutex*/
		pthread_cond_broadcast(&condc);
		pthread_mutex_unlock(&theMutex);
		usleep(100000);
	}
}

/*Consumer thread, that consumes an item, removes it from a queue. Sleeps 100ms after consuming
 * Will either do round-robin scheduling, or FCFS scheduling.
 */
void *consumer(void *ptr){
	printf("Hello! I am consumer %i.\n", *((int*)ptr));
	struct timeval cStart;
	struct timeval cEnd;
	gettimeofday(&cStart, NULL);
	while(1){
	  
		/*obtain lock over queue*/
		pthread_mutex_lock(&theMutex);
	
		/*if there is no products to consume, sleep*/
		while(currTotal!=productTotal && getNumElements()<1)
			pthread_cond_wait(&condc, &theMutex);
  
		/* exit criteria, if all products are consumed*/
		if(currTotal==productTotal && getNumElements()==0){
			
			/*get time info for consumer throughput*/
			gettimeofday(&cEnd, NULL);
			long secs_used = (cEnd.tv_sec - cStart.tv_sec);
			long micros_used = ((secs_used*1000000) + cEnd.tv_usec) - (cStart.tv_usec);
			cThrough+= micros_used;
			
			pthread_cond_broadcast(&condp);    
			printf("consumer %i is exiting.\n", *((int*)ptr));
			pthread_mutex_unlock(&theMutex);

			pthread_exit(0);
		}
	
		if(schedType==0){ //FCFS
		
		  /*remove a product from the queue*/
			product temp = dequeueFirst();
			consumeTotal++;
			
			/*time info for product wait time */
			struct timeval end;
			gettimeofday(&end, NULL);
			long secs_used = (end.tv_sec - temp->timeStamp.tv_sec);
			long micros_used = ((secs_used*1000000) + end.tv_usec) - (temp->timeStamp.tv_usec);
			if(micros_used > maxWaitTime){
				maxWaitTime = micros_used;
			}else if(micros_used < minTurnTime){
				minWaitTime = micros_used;
			}
			waitTime+= micros_used;
			
			/*consume the product entirely*/
			int i;
			for(i =0; i<temp->life; i++){
				fn();
			}
		  
			/*time info for product turnaround time */
			gettimeofday(&end, NULL);
			secs_used = (end.tv_sec - temp->timeStamp.tv_sec);
			micros_used = ((secs_used*1000000) + end.tv_usec) - (temp->timeStamp.tv_usec);
			if(micros_used > maxTurnTime){
				maxTurnTime = micros_used;
			}else if(micros_used < minTurnTime){
				minTurnTime = micros_used;
			}
			turnTime += micros_used;
			
			printf("Consumer %i consumed product %i.\n", *((int*)ptr),temp->id);
			free(temp);
			
			/*wakeup producers, and unlock*/
			pthread_cond_broadcast(&condp);
			pthread_mutex_unlock(&theMutex);
			
		}else{ //Round-Robin
		
			product temp = dequeueFirst();
			int life = temp->life;
			
			struct timeval start;
			struct timeval end;
			gettimeofday(&start, NULL);
			
			/*consume the product up till either the max quantum, or the life of the product, whichever is less*/
			int maxIt = life>quantum ? quantum : life;
			int i;
			for(i = 0; i<maxIt; i++){
				fn();
			}
			
			int newLife = life-quantum;
			
			/*time info for wait time*/
			gettimeofday(&end, NULL);
			long secs_used=(end.tv_sec - start.tv_sec); 
			long micros_used= ((secs_used*1000000) + end.tv_usec) - (start.tv_usec);
			temp->consumeTime+= micros_used;
			
			if(newLife>0){
				
			/* if there is still stuff left after consuming for 
			 * the length of the quantum, write it to the queue
			 */
			//printf("Consumer %i partially consumed consumed product %i. Took %i. New timespan of %i.\n", *((int*)ptr),temp->id,maxIt, temp->life);
			temp->life = newLife;
			enqueue(temp);
			}else{ /*otherwise, it is done consuming*/
				
				/*Calculate statistics on turnaround and wait time*/
				struct timeval end;
				gettimeofday(&end, NULL);
				long secs_used = (end.tv_sec - temp->timeStamp.tv_sec);
				long micros_used = ((secs_used*1000000) + end.tv_usec) - (temp->timeStamp.tv_usec);
				if(micros_used > maxTurnTime){
					maxTurnTime = micros_used;
				}else if(micros_used < minTurnTime){
					minTurnTime = micros_used;
				}
				turnTime += micros_used;
				
				/*wait time is turnaround time-time spent consuming*/
				long pWaitTime = micros_used-temp->consumeTime;
				if(pWaitTime > maxWaitTime){
					maxWaitTime = pWaitTime;
				}else if(pWaitTime < minTurnTime){
					minWaitTime = pWaitTime;
				}
				waitTime+= pWaitTime;
				
				printf("Consumer %i consumed product %i.\n", *((int*)ptr),temp->id);
				free(temp);
			}
		  /*unlock and signal*/
		  pthread_cond_broadcast(&condp);
		  pthread_mutex_unlock(&theMutex);
		}
		usleep(100000);
	}
}
