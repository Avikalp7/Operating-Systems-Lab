/*
**************************************************************************
Author: Avikalp Srivastava
Roll: 14CS10008
Assignment 3, Part-1 OS Lab Spring 2017
Problem Statement: Solving the m-producer n-consumer problem.
Link: cse.iitkgp.ac.in/~agupta/OSLab/Assgn3.pdf
**************************************************************************
*/

#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>	/*  This file is necessary for using shared
                        memory constructs
                        */
#include <sys/types.h>
#include <sys/sem.h>

#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */
#define BUFFER_SIZE 20

#define RANGE 50

void consumer();


int main(){

	/* Defining integer pointers for circular buffer, sum, number of ints read by consumer, in & out ptr position */
	int* circular_buff;
	int* sum;
	int *count_read;
	int *in, *out;
	
	/* User Inputs m: num producers, n: num consumers */
	int m, n;
	/* For Loop Variable */
	int i;
	/* Semaphore ids for 3 semaphores: full, empty and mutex */
	int semid1_full, semid2_empty, semid3_mutex;
	/* sumbufs for wait (pop) and signal (vop) */
	struct sembuf pop, vop ;
	/* Temporary process id */
	pid_t cpid;
	/* Shared Memory Address Variables */
	int shmid_buf, shmid_sum, shmid_in, shmid_out, shmid_countR;

	semid1_full = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	semid2_empty = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	semid3_mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	/* Handling Error */
	if(semid1_full == -1 || semid2_empty == -1 || semid3_mutex == -1){
		perror("semget: ");
		exit(-1);
	}

	/* Circular Buffer shared memory */
	shmid_buf    = shmget(IPC_PRIVATE, BUFFER_SIZE*sizeof(int), 0777|IPC_CREAT);
	/* Sum shared memory */
	shmid_sum    = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT);
	/* Count_Read shared memory */
	shmid_countR = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT);
	/* In pointer position shared memory */
	shmid_in = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT); 
	/* Out pointer position shared memory */
	shmid_out = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT);

	if(shmid_buf == -1 || shmid_sum == -1 || shmid_countR == -1 || shmid_in == -1 || shmid_out == -1){
		perror("shmget: ");
		exit(-1);
	}

	/* Initialising our 3 semaphores and error checking */
	if(semctl(semid1_full, 0, SETVAL, 0)==-1 || semctl(semid2_empty, 0, SETVAL, BUFFER_SIZE) == -1 || semctl(semid3_mutex, 0, SETVAL, 1) == -1){
		perror("semctl: ");
		exit(-1);
	}

	/* Aptly initialising structures pop and vop to perform wait and signal on semaphores respectively */
	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;


	/* Taking user input */
	printf("Enter m, n: ");
	scanf("%d %d",&m, &n);

	/* Main Child Process 1 : This child creates n-1 more consumers through forking and itself is a consumer as well. */
	// int cpid1, cpid2;
	if((cpid = fork()) == 0){	
		/* Attaching relevant memory segments */
		int cpid3 = getpid();
		circular_buff = (int *)shmat(shmid_buf, NULL, 0);
		sum = (int *)shmat(shmid_sum, NULL, 0);
		count_read = (int *)shmat(shmid_countR, NULL, 0);
		in = (int *)shmat(shmid_in, NULL, 0);
		/* Error Checking */
		if(*circular_buff == -1 || *sum == -1 || *count_read == -1 || *in == -1){
			perror("shmat: ");
			exit(-1);
		}
		/* Initialising shared variables */
		*in = 0;
		*sum = 0;
		*count_read = 0;

		/* Creating n-1 more consumers */
		for(i = 0; i < n-1; i++){
			cpid = fork();
			if(cpid==0){			// New child consumer
				/* Attaching relevant memory segments */
				circular_buff = (int *)shmat(shmid_buf, NULL, 0);
				in = (int *)shmat(shmid_in, NULL, 0);
				sum = (int *)shmat(shmid_sum, NULL, 0);
				count_read = (int *)shmat(shmid_countR, NULL, 0);
				/* Error Checking */
				if(*circular_buff == -1 || *sum == -1 || *count_read == -1 || *in == -1){
					perror("shmat: ");
					exit(-1);
				}
				break;
			}
			else{
				// do nothing
			}
		}

		while(1){
			/* Consumer Code */
			P(semid1_full);
			P(semid3_mutex);
			/* Till all the written ints haven;t been read */
			if(*count_read < RANGE * m){
				int read = circular_buff[*in];
				*sum = *sum + read;
				*in = (*in + 1)%BUFFER_SIZE;
				*count_read = *count_read + 1;
				// printf("Reading %d\n", *count_read);
				if(*count_read == RANGE*m)
					V(semid1_full);
				V(semid3_mutex);
				V(semid2_empty);
			}
			/* Else, all read, so signal, detach and exit */
			else{
				V(semid3_mutex);
				V(semid2_empty);
				V(semid1_full);
				if(shmdt(circular_buff) == -1 || shmdt(sum) == -1 || shmdt(count_read) == -1 || shmdt(in) == -1){
					perror("");
					exit(-1);
				}
				printf("CONSUMER DETACHING\n");
				if(getpid()==cpid3){
					for(i = 0; i < n-1; i++){
						wait(NULL);
					}
				}
				exit(0);
			}
		}
	}
	else{	// Parent Process
		sum = (int *) shmat(shmid_sum, NULL, 0);
		count_read = (int *) shmat(shmid_countR, NULL, 0);
		if((cpid2 = fork()) == 0){	// Main Child Process 2
			circular_buff = (int *)shmat(shmid_buf, NULL, 0);
			out = (int *)shmat(shmid_out, NULL, 0);
			/* Error Handling */
			if(*circular_buff == -1 || *out == -1){
				perror("");
				exit(-1);
			}
			/* Initialisation */
			*out = 0;
			/* Creating m producers */
			for(i = 0; i < m; i++){
				cpid = fork();
				if(cpid==0){			// New child producer
					circular_buff = (int *)shmat(shmid_buf, NULL, 0);
					out = (int *)shmat(shmid_out, NULL, 0);
					/* Error Handling */
					if(*circular_buff == -1 || *out == -1){
						perror("");
						exit(-1);
					}
					for(i=1; i<=RANGE; i++){
						P(semid2_empty);
						P(semid3_mutex);
						circular_buff[*out] = i;
						*out = (*out + 1)%BUFFER_SIZE;
						V(semid3_mutex);
						V(semid1_full);
					}
					if(shmdt(circular_buff) == -1 || shmdt(out)){
						perror("");
						exit(-1);
					}
					printf("PRODUCER DETACHING\n");
					exit(0);
				}
				else{
					wait(NULL);
				}
			}
			if(shmdt(circular_buff) == -1 || shmdt(out) == -1){
				perror("");
				exit(-1);
			}
			// printf("Child here\n");
			exit(0);
		
		}
		else{ // Parent Process
			/* Wait for children */
			wait(NULL);
			wait(NULL);
			// int status;
			// waitpid(cpid2, &status, 0);
			// waitpid(cpid1, &status, 0);
			/* Print the shared variable sum */
			printf("SUM: %d\n", *sum);
			/* Detach from shared memory */
			if(shmdt(sum) == -1 || shmdt(count_read) == -1){
				perror("");
				exit(-1);
			}
			/* Remove semaphores */
			if(semctl(semid1_full, 0, IPC_RMID, 0) == -1 || semctl(semid2_empty, 0, IPC_RMID, 0) == -1 || semctl(semid3_mutex, 0, IPC_RMID, 0) == -1){
				perror("");
				exit(-1);
			}
			/* Delete the created shared memories */
			if(shmctl(shmid_buf, IPC_RMID, 0) == -1 || shmctl(shmid_out, IPC_RMID, 0) == -1 || shmctl(shmid_sum, IPC_RMID, 0) == -1 || shmctl(shmid_in, IPC_RMID, 0) == -1 || shmctl(shmid_countR, IPC_RMID, 0) == -1){
				perror("");
				exit(-1);
			}
		}

	}

	return 0;
}



