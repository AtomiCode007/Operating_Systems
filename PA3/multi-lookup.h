#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Queue.h"
#include "util.h"
#include <pthread.h>
#include <semaphore.h>

struct Globo
{	
	queue* queue;

	int totalInfiles;
	int inCompleteF;
	int newFilepoint;
	int helpPoint;
	int lastFileindex;
	int ID;
	int* allRequestfinish;	
	
	FILE* listOfiles [10];

	//pthread_mutex_t* mutex;
	pthread_mutex_t* writeBlock;
	pthread_mutex_t* readBlock;
	pthread_mutex_t* queueBlock;
};


struct Request
{		
	int touched;
	int ID;
	int currentFile;
	FILE* inputF;
	FILE* Log;
	struct Globo* Globop;
};

struct Resolve
{
	FILE* output;
	struct Globo* Globop;
	int ID;
};