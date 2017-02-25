#include "main.h"

struct productInfo{
  int id;
  struct timeval timeStamp;
  int life;
	
 };
typedef struct productInfo* product;

/*Globals */
int id = 0; // unique id for each product
int productTotal; // total number of products to produce, given as CL input
int currTotal = 0; //total items produced so far
int quantum; //given as CL input
int seed; //given as CL input, for random number generation
int maxBufferSize;//given as CL input
int schedType;
pthread_mutex_t theMutex;
pthread_cond_t condc, condp;
pthread_t* producerThreads;
pthread_t* consumerThreads;
int* pn;
int* cn;


/*Forward Declarations */
product makeProduct();
void *producer(void *ptr);

/*Create and return a new product*/
product makeProduct(){
	 product p= (product)malloc(sizeof(struct productInfo));
	 gettimeofday(&(p->timeStamp),NULL);
	 p->id = id;
	 id++;
	 srandom(seed);
	 p->life = random()%1024;
	 return p;
 }

int fn(){
  int cnt = 0;
  int n1 = 0;
  int n2 = 1;
  int newN = 0;
  while(cnt < 10){
    newN = n1 + n2;
    n1 = n2;
    n2 = newN;
  }
  return newN;
}
 
int main(int argc, char *argv[]){
	
	int numPThreads;
	int numCThreads;

	
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
	
	pthread_mutex_init(&theMutex, 0);
	pthread_cond_init(&condc, 0);
	pthread_cond_init(&condp, 0);
	
	for(int i = 0; i<numPThreads; i++){
		/*set up producer threads */
		pn[i] = i;
		pthread_create(&producerThreads[i], NULL, producer, &pn[i]);
	}
	for(int i = 0; i<numCThreads; i++){
		/*set up consumer threads */
		cn[i] = i;
		pthread_create(&consumerThreads[i], NULL, consumer, &cn[i]);
	}
	
	pthread_cond_destroy(&condc);
	pthread_cond_destory(&condp);
	pthread_mutex_destroy(&theMute);
	return 0;
} 

/*TODO: Add printout of "Producer X has produced product Y*/
void *producer(void *ptr){
	while(true){
		pthread_mutex_lock(&theMutex);
		while(maxBufferSize && getNumElements()>=maxBufferSize)
			pthread_cond_wait(&condp, &theMutex);
		if(currTotal==productTotal){
			pthread_mutex_unlock(&theMutex);
			pthread_exit(0);
		}
		product tempProduct = makeProduct();
		enqueue(tempProduct);
		printf("%snsns", "Producer ", *((int*)(ptr)), " has produced product ", tempProduct->id, ".");
	        usleep(100000);
		currTotal++;
		pthread_cond_signal(&condc);
		pthread_mutex_unlock(&theMutex);
	}
}

/*TODO: Implement*/
void *consumer(void *ptr){
	pthread_exit(0);
}
