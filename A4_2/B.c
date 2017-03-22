#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>	/*  This file is necessary for using shared
                        memory constructs
                        */
#include <sys/types.h>
#include <sys/sem.h>


#define QUEUE_CAPACITY 10


#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */


int too_many_threads_flag = 0;

pthread_mutex_t cnt_mutex;
pthread_cond_t cnt_cond;


int* signal_flag;
int signal_flag_B = 0;
void my_handler(int signum)
{
	printf("\nExiting...\n");
	*signal_flag = 1;
	signal_flag_B = 1;
}


int num_active_threads = 0;
int TICKET = 100;


void *book_ticket(void *param)
{

	int* x = (int *) param;

	pthread_mutex_lock(&cnt_mutex);
		num_active_threads += 1;
		if(too_many_threads_flag==0 && num_active_threads >= 10)
		{
			printf("\n\nTOO MANY THREADS: %d, TEMPORARY HALTING OF NEW THREADS TILL THREAD_COUNT <= 5\n\n", num_active_threads);
			too_many_threads_flag = 1;
		}
	pthread_mutex_unlock(&cnt_mutex);

	pthread_mutex_lock(&cnt_mutex);
		if (TICKET - *x >= 0)
		{
			TICKET = TICKET - *x < 100? TICKET - *x : 100;
			int k = TICKET;
			pthread_mutex_unlock(&cnt_mutex);
			int sleep_time = (rand() % 3);
			sleep(sleep_time);
			printf("Ticket: %d\n", k);
			
			pthread_mutex_lock(&cnt_mutex);
				if(too_many_threads_flag==1 && num_active_threads <= 5)
					pthread_cond_signal(&cnt_cond);
				num_active_threads -= 1;
			pthread_mutex_unlock(&cnt_mutex);
			*x = 10;
			return (void *)1;
		}
		else
		{
		pthread_mutex_unlock(&cnt_mutex);
			int sleep_time = (rand() % 2) + 1;
			sleep(sleep_time);
			printf("Sorry, the request to book the ticket could not be fulfilled!\n");
			pthread_mutex_lock(&cnt_mutex);
			if(too_many_threads_flag==1 && num_active_threads <= 5){
				printf("\nTHREAD_COUNT <= 5, COMMENCING CREATION OF NEW THREADS\n\n");
				pthread_cond_signal(&cnt_cond);
			}
			num_active_threads -= 1;
			pthread_mutex_unlock(&cnt_mutex);
			*x = 10;
			return (void *)0;
		}
}


int main()
{
	srand(time(NULL));
	signal(SIGINT, my_handler);
	struct sembuf pop, vop;
	/* Aptly initialising structures pop and vop to perform wait and signal on semaphores respectively */
	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;

	int* shared_queue;
	int* start_pointer;
	int* end_pointer;

	int shm_queue, shm_end_pointer, shm_signal_flag, shm_start_pointer;

	int keys[] = {4440,5550,6660};
	int semid1_full = semget(keys[0], 1, 0666|IPC_CREAT);
	int semid2_empty = semget(keys[1], 1, 0666|IPC_CREAT);
	int semid3_mutex = semget(keys[2], 1, 0666|IPC_CREAT);

	key_t shm_queue_key = 2220;
	key_t shm_end_pointer_key = 3330;
	key_t shm_start_pointer_key = 8880;
	key_t shm_signal_flag_key = 7770;

	shm_queue = shmget(shm_queue_key, QUEUE_CAPACITY*sizeof(int), 0666|IPC_CREAT);
	shm_end_pointer = shmget(shm_end_pointer_key, sizeof(int), 0666|IPC_CREAT);
	shm_start_pointer = shmget(shm_start_pointer_key, sizeof(int), 0666|IPC_CREAT);
	shm_signal_flag = shmget(shm_signal_flag_key, sizeof(int), 0666|IPC_CREAT);

	if(shm_queue < 0 || shm_end_pointer < 0 || shm_signal_flag < 0 || shm_start_pointer < 0)
	{
		perror("shmget");
		exit(1);
	}
	if((shared_queue = (int *) shmat(shm_queue, NULL, 0)) == (int *) -1)
	{
		perror("shmat");
		exit(1);
	}
	if ((start_pointer = (int *) shmat(shm_start_pointer, NULL, 0)) == (int *) -1) 
	{
        perror("shmat");
        exit(1);
    }
	if ((end_pointer = (int *) shmat(shm_end_pointer, NULL, 0)) == (int *) -1) 
	{
        perror("shmat");
        exit(1);
    }
    if ((signal_flag = (int *) shmat(shm_signal_flag, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    /* Thread */
	pthread_t tid;
	int x[11] = {10};
	
	// initialize the mutex and the condition variable

	pthread_mutex_init(&cnt_mutex, NULL);
	pthread_cond_init(&cnt_cond, NULL);
	/* Thread end */

	*signal_flag = 0;
    *end_pointer = 0;
    *start_pointer = 0;
    while(1)
    {
    	if(*signal_flag==1)
		{
			break;
		}
		P(semid1_full);
    	P(semid3_mutex);
    		
    		pthread_mutex_lock(&cnt_mutex);
    			while(too_many_threads_flag == 1 && num_active_threads > 5)
    			{
    				pthread_cond_wait(&cnt_cond, &cnt_mutex);
    				too_many_threads_flag = 0;
    			}
    			int i;
    			for(i=0;i<=10;i++){
    				if(x[i]==10)
    					break;
    			}
    			// printf("Reading from %d\n", *start_pointer);
    			x[i] = shared_queue[*start_pointer];
    			// printf("x[i] : %d\n", x[i]);
    		pthread_mutex_unlock(&cnt_mutex);
    		*start_pointer = (*start_pointer + 1)%QUEUE_CAPACITY;
    		pthread_create(&tid, NULL, book_ticket, (void *) &x[i]);
    	
    	V(semid3_mutex);
    	V(semid2_empty);
    }

    /* Detaching from shared memory */
	shmdt(shared_queue);
	shmdt(start_pointer);
	shmdt(end_pointer);
	/* Clean up the mutex and condition variable */
	pthread_mutex_destroy(&cnt_mutex);
	pthread_cond_destroy(&cnt_cond);

    return 0;
}