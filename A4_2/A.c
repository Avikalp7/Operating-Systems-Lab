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
#include <time.h>

#define QUEUE_CAPACITY 10

#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */



int* signal_flag;
void my_handler(int signum)
{
	// printf("Received a SIGINT\n");
	*signal_flag = 1;
}


int get_random()
{
	return (rand()%11 - 5);
}

int main()
{
	signal(SIGINT, my_handler);
	srand(time(NULL));
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
	int semid1_full = semget(keys[0], 1, 0666);
	int semid2_empty = semget(keys[1], 1, 0666);
	int semid3_mutex = semget(keys[2], 1, 0666);

	if(semctl(semid1_full, 0, SETVAL, 0)==-1 || semctl(semid2_empty, 0, SETVAL, QUEUE_CAPACITY) == -1 || semctl(semid3_mutex, 0, SETVAL, 1) == -1)
	{
		perror("No server to send request to! ");
		exit(-1);
	}

	key_t shm_queue_key = 2220;
	key_t shm_end_pointer_key = 3330;
	key_t shm_start_pointer_key = 8880;
	key_t shm_signal_flag_key = 7770;

	shm_queue = shmget(shm_queue_key, QUEUE_CAPACITY*sizeof(int), 0666);
	shm_start_pointer = shmget(shm_start_pointer_key, sizeof(int), 0666);
	shm_end_pointer = shmget(shm_end_pointer_key, sizeof(int), 0666);
	shm_signal_flag = shmget(shm_signal_flag_key, sizeof(int), 0666);
	
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
	if ((start_pointer = (int *) shmat(shm_start_pointer, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }
	if ((end_pointer = (int *) shmat(shm_end_pointer, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }
    if ((signal_flag = (int *) shmat(shm_signal_flag, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    *signal_flag = 0;
    *end_pointer = 0;
    *start_pointer = 0;
	while(1)
	{
		if(*signal_flag==1)
		{
			break;
		}
		/* Sleep for 0-2 seconds */
		int sleep_time = (rand() % 3);
		sleep(sleep_time);
		P(semid2_empty);
		P(semid3_mutex);
			shared_queue[*end_pointer] = get_random();
			printf("Sending Request, at position: %d, with value: %d\n", *end_pointer, shared_queue[*end_pointer]);
			*end_pointer = (*end_pointer + 1) % QUEUE_CAPACITY;
		V(semid3_mutex);
		V(semid1_full);
	}


	printf("Performing memory cleanup\n\n");
	/* Detaching from shared memory */
	shmdt(shared_queue);
	shmdt(end_pointer);
	shmdt(start_pointer);
	/* Remove semaphores */
	if(semctl(semid1_full, 0, IPC_RMID, 0) == -1 || semctl(semid2_empty, 0, IPC_RMID, 0) == -1 || semctl(semid3_mutex, 0, IPC_RMID, 0) == -1)
	{
		perror("semaphore removal: ");
		exit(-1);
	}
	/* Delete the created shared memories */
	if(shmctl(shm_queue , IPC_RMID, 0) == -1 || shmctl(shm_end_pointer, IPC_RMID, 0) == -1 || shmctl(shm_start_pointer, IPC_RMID, 0) == -1)
	{
		perror("shared memory removal: ");
		exit(-1);
	}

	printf("Memory cleanup successfully done, exiting...\n");
	return 0;
}

