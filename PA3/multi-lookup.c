#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multi-lookup.h"
#include "util.c"
#include <pthread.h>
#include <sys/time.h>
#include "Queue.h"
#include <unistd.h>


#define MAXARGS 15
#define BUFFSIZE 1025
#define INPUTFS "%1024s"
#define NUM_RQTHREADS 5
#define NUM_RTHREADS 10
#define MAXFILES 10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN



void* RequestFnc(void* argument)
{
	//INIT LOCAL VARIABLES TO MAKE LIFE EASIER
	struct Request *requestor = argument;

	char website[BUFFSIZE];
	char* payload;

	FILE* inFile;
	FILE* Log;

	queue* q = requestor->Globop->queue;

	int pushIsGood = 0;
	int threadNum;
	int touchedFiles = 0;
	int actualhelp = 0;
	int HelpFile = 0;
	//int KLTFBNF = 0;

	int currFile = requestor->currentFile;
	int ENDFILE = requestor->Globop->lastFileindex;

	//printf("ENDFILE = %d\n",ENDFILE);
	pthread_mutex_t* queueBlock;
	pthread_mutex_t* serviceBlock;

	//SET THE VARIABLES PERTAINNING TO THE STRUCT PASSED IN

	threadNum = requestor->ID;
	touchedFiles = requestor->touched;
	queueBlock = requestor->Globop->queueBlock;
	serviceBlock = requestor->Globop->readBlock;

	Log = requestor->Log;

	/*
	pthread_mutex_lock(queueBlock);
	printf("I have the lock bitch! ID # %d\n\n",threadNum);

	printf("THREAD ID: %d\n", threadNum);
	printf("Current file number: %d\n",currFile);
	printf("ENDFILE #: %d\n", ENDFILE);
	printf("LOG POINTER: %p\n\n",Log);


	printf("I am going to unlock the lock bitch! ID # %d\n\n",threadNum);
	pthread_mutex_unlock(queueBlock);
	*/

	//BEGIN CRITICAL DANGER ZONE	
	while(currFile < ENDFILE)
	{	

		//printf("Thread %d is switching to file %d from %d. \n",threadNum,currFile-1,currFile);
		//printf("Thread %d is switching to a file pointer of %p from %p. \n",threadNum,requestor->Globop->listOfiles[currFile-1],requestor->Globop->listOfiles[currFile]);
		if(requestor->Globop->listOfiles[currFile] != NULL)
		{
			//touchedFiles++;
			actualhelp = 0;
			//KLTFBNF++;
			inFile = requestor->Globop->listOfiles[currFile];

			while(fscanf(inFile,INPUTFS,website) > 0) //READING FROM THE FILE...
			{	
				while(pushIsGood == 0)
				{
					pthread_mutex_lock(queueBlock);//Lock the queue!
					//printf("Request thread %d has locked the queue\n",threadNum);
					if(!queue_is_full(q))
					{
						payload = malloc(BUFFSIZE);
						strncpy(payload,website,BUFFSIZE);

						//printf("Adding %s to the Queue...\n",website);
						queue_push(q,payload);
						pushIsGood = 1;
						actualhelp = 1;
						//printf("Request thread %d has unlocked the queue\n\n",threadNum);
						pthread_mutex_unlock(queueBlock);//unlock the queue!
					}
					else
					{
						//printf("Queue's full bitch!\n\n");
						pthread_mutex_unlock(queueBlock);//unlock the queue!
						pushIsGood = 0;
					}
					//printf("Request thread %d has unlocked the queue\n",threadNum);
				}

				pushIsGood = 0;
			}
		}
		if(actualhelp == 1)
		{
			HelpFile++;
		}
		if(touchedFiles < ENDFILE - 1)
		{
			//printf("Loppin! \n");
			touchedFiles++;
			if(currFile == ENDFILE - 1)
			{
				currFile = 0;
			}
			else
			{
				currFile ++;
			}
		}
		else
		{
			currFile++;
		}
		
		//if(requestor->Globop->fileRefrence[])
	}

	pthread_mutex_lock(serviceBlock);
	if(Log != NULL)
	{
		if(HelpFile <= ENDFILE)
		{
			fprintf(Log,"Requester %d has serviced %d files. \n",threadNum,HelpFile);
		}
		else
		{
			fprintf(Log,"Requester %d has serviced %d files. MORE THAN FILES? \n",threadNum, HelpFile);
		}
	}
	pthread_mutex_unlock(serviceBlock);
	return NULL;
}

//BEGIN RESOVLER FUNCT

void* ResolveFnc(void* argument)
{
	//LOCAL VARIABLE DECLARATION
	struct Resolve *resolver = argument;

	FILE* inFile;
	queue* q;

	char IPstr[INET6_ADDRSTRLEN];
	char* payloadHst;
	char website[BUFFSIZE];

	int threadNum;
	int ReqWpulse;

	pthread_mutex_t* queueBlock;
	pthread_mutex_t* resolveWrite;
	

	//QUALITY OF LIFE ASSIGNMENTS
	q = resolver->Globop->queue;
	inFile = resolver->output;
	threadNum = resolver->ID;
	queueBlock = resolver->Globop->queueBlock;
	resolveWrite = resolver->Globop->writeBlock;
	ReqWpulse = *resolver->Globop->allRequestfinish;

	//printf("Status before root change. %d\n", *resolver->Globop->allRequestfinish);printf("Is there a requestor that has a pulse? %d\n", allReqFinish);

	//usleep(100000);
	//printf("Status of requestors after a long wait.%d\n", *resolver->Globop->allRequestfinish);

	while((!queue_is_empty(q)) || (ReqWpulse == 1))
	{
		//printf("Is there a requestor that has a pulse? %d\n", ReqWpulse);

		ReqWpulse = *resolver->Globop->allRequestfinish;

		//printf("Is there a requestor that has a pulse? %d\n", ReqWpulse);

		if(!queue_is_empty(q))
		{
			//MUTEX FOR QUEUE IS HERE
			pthread_mutex_lock(queueBlock);//Lock the queue!-----------

			//printf("Resolve thread %d has locked the queue\n",threadNum);
			payloadHst = queue_pop(q);
			strncpy(website,payloadHst,BUFFSIZE);
			if(payloadHst == NULL)
			{
				pthread_mutex_unlock(queueBlock);
			}
			else
			{
				//strncpy(website,payloadHst,BUFFSIZE);
				//printf("Resolve thread %d has unlocked the queue\n",threadNum);

				pthread_mutex_unlock(queueBlock);

				//pthread_mutex_lock(resolveWrite);
				//printf("Resolve thread %d has locked the file\n\n",threadNum);

				if(dnslookup(website,IPstr,INET6_ADDRSTRLEN)==UTIL_FAILURE)
				{		
					//printf("Resolve thread %d has the file #2\n",threadNum);
					strncpy(IPstr,"",INET6_ADDRSTRLEN);
					fprintf(stderr,"You dun goofed...\n");
				}

				pthread_mutex_lock(resolveWrite);

				//printf("Resolve thread %d has locked the file\n\n",threadNum);

				//printf("Resolve thread %d has printed %s:%s \n",threadNum,website,IPstr);
				fprintf(inFile, "%s:%s By resolver # %d\n",website,IPstr,threadNum);
				//printf("Resolve thread %d has wrote %s:%s \n\n",threadNum,website,IPstr);

				pthread_mutex_unlock(resolveWrite);//UNLOCK THE FILE ---------
				//printf("Resolve thread %d has unlocked the file\n\n",threadNum);

				free(payloadHst);
			}
		}
	}

	//free(payloadHst);
	//printf("Resolve thread %d has FINISHED.\n",threadNum);

	return NULL;
}


int main(int argc, char* argv[])
{
	struct timeval begin,end;
	gettimeofday(&begin,NULL);

	//ERROR CHECK FOR TOO MANY FILES
	if(argc < 6)
	{
		fprintf(stderr, "Not enough arguments passed in, only 6 are needed. You put in: %d\n", (argc));
		//printf("i'm in here!\n");
	}

	//OPEN THE TWO LOG FILES & DECLARE LOCAL VARIABLES
	FILE* Beet = fopen(argv[3],"w"); //serviced.txt

	if(Beet)
	{
		//printf("Requester thread service results will be written to: %s\n", argv[3]);
	}
	//printf("ARGV[3] = %s\n", argv[3]);
	//printf("ARGV[4] = %s\n", argv[4]);

	FILE* Potato = fopen(argv[4],"w");//results.txt

	if(Potato)
	{
		//printf("Resolver thread results will be written to: %s\n", argv[4]);
	}

	FILE* inFiles[MAXFILES];

	int totalFile = argc - 5;
	int lastFile = argc -5;
	int helpIndex = 0; 
	int newFileindex = 5;
	int remainingFiles = totalFile;
	int totalRequest = atoi(argv[1]);
	int totalResolve = atoi(argv[2]);
	int REQCheckPulse = 1;
	int ErrorChk;

	//printf("Total number of files %d\n", totalFile);
	//printf("Number of LAST file %d\n", lastFile);
	//printf("Total number of requestors asked for %d\n", totalRequest);
	printf("Total number of resolvers asked for %d\n", totalResolve);

	//printf("I'm initalizing the file array!\n");
	for(int k = 0;k < totalFile; k++)
	{
		inFiles[k] = fopen(argv[k+5],"r");//k = The beginning of the file arguements from the terminal.

		//printf("%p\n",inFiles[k]); //prints the memory location of each file index in the array.
	}


	//UNIVERSAL STRUCT INIT AND MUTEX INIT 
	struct Globo theClown;
	pthread_mutex_t webQueue;
	pthread_mutex_t resultsLock;
	pthread_mutex_t servicedLock;
	//pthread_mutex_t muteXx;
	pthread_mutex_init(&webQueue,NULL);
	pthread_mutex_init(&resultsLock,NULL);
	pthread_mutex_init(&servicedLock,NULL);

	//printf("QUEUE BLOCK INIT %d\n",pthread_mutex_init(&webQueue,NULL)); //Queue mutex init
	//printf("WRITING BLOCK INIT %d\n",pthread_mutex_init(&resultsLock,NULL)); //Resolver log mutex
	//printf("SERVICED BLOCK INIT %d\n",pthread_mutex_init(&servicedLock,NULL)); //Requester log mutex
	
	theClown.inCompleteF = remainingFiles; //Set to completed files to 0

	theClown.totalInfiles = totalFile; //Set to total input files given.

	theClown.newFilepoint = newFileindex; //Set the index of what will be the new files after request assign.

	theClown.helpPoint = helpIndex;//Sets an index to the first open file of the file array...

	theClown.lastFileindex = lastFile;//index to the end of the file array.
	
	theClown.allRequestfinish = &REQCheckPulse;

	//theClown.mutex = &muteXx;//sets the pointer of the struct mutex pointer to here...
	
	theClown.writeBlock = &resultsLock;//sets the pointer of the struct mutex pointer to here...
	
	theClown.readBlock = &servicedLock;//sets the pointer of the struct mutex pointer to here...
	
	theClown.queueBlock = &webQueue;//sets the pointer of the struct mutex pointer to here...
	
	//QUEUE INIT & ERROR CHECK
	//printf("Queue assignment\n");
	queue Queue;
 	ErrorChk = queue_init(&Queue, BUFFSIZE);
 	if(ErrorChk== -1)
 	{
 		printf("Queue is borked!%d\n",ErrorChk);
 	}
 	//printf("Slaps top of queue:\n This baby can hold so many god dam nodes... %d\n",ErrorChk); //will print the size of the queue (in nodes) you just created.

 	//ASSIGN THE QUEUE TO THE STRUCT.
	theClown.queue = &Queue;

	//OPEN ALL OF THE FILES THE USER GAVE
	//printf("I'm initalizing the file array PART TWO!\n");
	for(int k = 0;k < totalFile; k++)
	{

		inFiles[k] = fopen(argv[k+5],"r");//k = The beginning of the file arguements from the terminal.
		//printf("FILE NAME: %s\n",argv[k+5]);
		//printf("Opening the files! Memory location: %p\n",inFiles[k]); //prints the memory location of each file index in the array.
	}
	for(int o = 0; o < totalFile; o++)
	{
		theClown.listOfiles[o] = inFiles[o];
		//printf("%p\n",theClown.listOfiles[o]); //prints the memory location of each file index in the array.
	}

	//CREATE THE REQUEST THREAD POOL
	struct Request requestors[totalRequest];
	pthread_t requesterThreads[totalRequest];

	for(int i = 0; i < totalRequest; i++)
	{
		requestors[i].Globop = &theClown;
		if(i > totalFile)
		{
			requestors[i].currentFile = 0;
		}
		else
		{
			requestors[i].currentFile = i;
		}
		requestors[i].ID = i;
		requestors[i].touched = 0;
		requestors[i].Log = Beet;

		ErrorChk = pthread_create(&(requesterThreads[i]),NULL,RequestFnc,&(requestors[i]));

		if(ErrorChk != 0)
		{
			fprintf(stderr, "Something went wrong while creating the Request Thread,returned: %d\n",ErrorChk);
		}
		else
		{	
			//printf("Yee haw! %d\n",i);
		}

	}

	//END REQUEST THREAD POOL

	//BEGIN CREATING THE RESOLVER POOL
	//usleep(398123);
	struct Resolve resolvers[NUM_RTHREADS];
	pthread_t resolverThreads[NUM_RTHREADS];

	for(int i = 0; i <totalResolve; i++)
		{
			resolvers[i].Globop = &theClown;
			resolvers[i].output = Potato;
			resolvers[i].ID = i;

			ErrorChk = pthread_create(&(resolverThreads[i]),NULL,ResolveFnc,&(resolvers[i]));

			if(ErrorChk)
			{
				fprintf(stderr, "Something went wrong while creating the Resolve Threads,returned: %d\n",ErrorChk);
			}
			else
			{	

				//printf("RESOLVER Yee haw! %d\n",i);
			}
		}
	//END RESOLVER POOL CREATION

	//WAIT FOR ALL OF THE REQUEST THREADS TO FINISH
	int t;
	for(t=0; t< totalRequest; t++)
	{
		//printf("Waiting on the requestors...\n");
		//printf("Request Thread %d has FINISHED %d.\n" ,t,pthread_join(requesterThreads[t],NULL));
		pthread_join(requesterThreads[t],NULL);
	}

	//printf("Is there a requestor that has a pulse? BEFORE %d\n", REQCheckPulse);
	//printf("Is there a requestor that has a pulse? BEFORE w/ GLOBAL INT %d\n", *theClown.allRequestfinish);

	REQCheckPulse = 0;//All your base belong to us.

	//printf("Is there a requestor that has a pulse? AFTER %d\n", REQCheckPulse);
	//printf("Is there a requestor that has a pulse? AFTER w/ GLOBAL INT %d\n", *theClown.allRequestfinish);
	//WAIT FOR ALL OF THE RESOLVER THREADS TO FINISH
	int u;
	for(u=0; u < totalResolve;u++)
	{
		//printf("Waiting on the resolvers...\n");
		//printf("Resolve Thread %d has FINISHED %d.\n" ,u,pthread_join(resolverThreads[u],NULL));
		pthread_join(resolverThreads[u],NULL);

	}

	//for(int g; )
	//usleep(2891010);
	pthread_mutex_destroy(&webQueue);
	pthread_mutex_destroy(&resultsLock);
	pthread_mutex_destroy(&servicedLock);
	//pthread_mutex_destroy(&muteXx);

	/*for(int t = 0; t< totalFile; t++)
	{
		fclose(inFiles[t]);
	}*/
	fclose(Beet);
	fclose(Potato);
	queue_cleanup(&Queue);

	for(int g =0; g < totalFile;g++)
	{
		if(inFiles[g] != NULL)
		{
			fclose(inFiles[g]);
		}
		else
			continue;
	}
	//free(inFiles);
	gettimeofday(&end,NULL);
	long seconds = (end.tv_sec -begin.tv_sec);
	long micro = ((seconds * 1000000) + end.tv_usec) - (begin.tv_usec);
	printf("This process took %ld seconds and %ld microseconds to finish\n",seconds,micro);
	return 0;
}
