/*
**************************************************************************
Author: Avikalp Srivastava
Roll: 14CS10008
Assignment 3, Part-2 OS Lab Spring 2017
Problem Statement: Read and Write through 2 processes
Link: cse.iitkgp.ac.in/~agupta/OSLab/Assgn3.pdf
**************************************************************************
*/


/* System Libraries */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>

/* Other Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/* Define constants */
#define BUFFER_SIZE 300
#define MAX_STUD_NUM 100


/* Check error on opening file */
void check_file_open_status(FILE* fp){
	if(fp == NULL)
	    {
	    	perror("File ");
	    	exit(1);
	    }
}

/* Structure for storing each students' info */
typedef struct student{
	char first_name[20];
	char last_name[20];
	int roll_num;
	double cgpa;
} student;


void check_errors(int shmid, int shmflag, int shmsize, int shmsem, int** size, student** students_info_table, int** bool_changed){
	if(!(shmid >= 0 && shmflag >= 0 && shmsize > -1 && shmsem > -1))
	{
		perror("shmget");
		exit(1);
	}
	if((*size = (int *) shmat(shmsize, NULL, 0)) == (int *) -1)
	{
		perror("shmat");
		exit(1);
	}
	if ((*students_info_table = (student *) shmat(shmid, NULL, 0)) == (student *) -1) {
        perror("shmat");
        exit(1);
    }
    if((*bool_changed = (int *) shmat(shmflag, NULL, 0)) == (int *) -1)
    {
    	perror("shmat");
    	exit(1);
    }
}



#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing
				   the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing
				   the V(s) operation */





int main(int argc, char *argv[]){
	char *file_path = (char*)malloc(100);
	if(argc != 2){
		printf("Invalid Call ");
		printf("Format is <executable> <filename>\n");
		exit(-1);
	}
	strcpy(file_path, argv[1]);	
	student *students_info_table;
	FILE *fp;
	int shmid, shmflag,shmsize,shmsem,count=0;
	char buffer[BUFFER_SIZE], *line = NULL;
	size_t blah;

	int semid_mutex = semget(2222, 1, 0777 | IPC_CREAT);
	struct sembuf pop, vop;

	if(semid_mutex == -1){
			perror("semget: ");
			exit(-1);
		}
	int smutex = semctl(semid_mutex, 0, SETVAL, 1);
	pop.sem_num = 0;
    vop.sem_num = 0;
    pop.sem_flg = 0;
    vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;

	/* Shared Memory init */
	shmsem = shmget(4440, sizeof(int), IPC_CREAT | 0666); // shared memory for semaphore
	shmsize = shmget(3330, sizeof(int), IPC_CREAT | 0666);	// shared memory to keep size
	shmid = shmget(2220,  MAX_STUD_NUM * sizeof(student), IPC_CREAT | 0666); // Shared memory for student info tables
	shmflag = shmget(9990, sizeof(int), IPC_CREAT | 0666);  // a bool_changed to see if change made by y?
	

	/* Checking for errors!!! */
	int *bool_changed, *size;
	check_errors(shmid, shmflag, shmsize, shmsem, &size, &students_info_table, &bool_changed);

	printf("No Error Encountered in Initialiing memory\n");	


    fp = fopen(file_path, "r");
    check_file_open_status(fp);
    

    *size = 0;
    /* REading from file */
    while (!(getline(&line, &blah, fp) == -1)){
		char **words = (char **) malloc(sizeof(char *) * 4);
		char * word;
		word = strtok(line, " ");
		int idx = 0;
		while(!(word == NULL)){
			words[idx++] = word;
			word = strtok(NULL," ");
		}
		words[idx] = NULL;
    	printf("%s  %s  %d  %lf\n",words[0], words[1], atoi(words[2]), atof(words[3]));
    	strcpy(students_info_table[count].first_name, words[0]);
    	strcpy(students_info_table[count].last_name, words[1]);
    	students_info_table[count].cgpa = atof(words[3]);
    	students_info_table[count].roll_num = atoi(words[2]);
    	count++;
    	printf("%d\n",count);
    }

    // Close the file, no longer needed.
    fclose(fp);


    
    int sstart = shmget(2000, sizeof(int), IPC_CREAT | 0777);
    if (sstart == -1)
    {
        perror("shmget start key");
        exit(EXIT_FAILURE);
    }

    int* go_flag = shmat(sstart, 0, 0);
    *go_flag = 21;


    *size = count;
    *bool_changed = 1;
    
    int bool_init = 1;
    
    /* BECAUSE X GOES ON FOREVER AND FOREVER*/
    while(1){
    	if(*bool_changed == 1){
    		P(semid_mutex);
    		fp = fopen(file_path, "w");
    		int counter = 0;
    		
    		while(counter < count){
    			fprintf(fp, "%s %s %d %lf\n", students_info_table[counter].first_name, students_info_table[counter].last_name, students_info_table[counter].roll_num, students_info_table[counter].cgpa);
    			counter++;
    		}
    		fclose(fp);
    		
    		if(bool_init == 1)
    			bool_init = 0;
    		else
    			printf("File rewritten\n");
    		
    		*bool_changed = 0;
    		V(semid_mutex);
    	}

    	/* X wanna sleep for 5 sec*/
    	sleep(5);
    }
    return 0;
}