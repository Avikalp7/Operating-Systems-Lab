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


#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)

int main(){
	int shmsize, shmid, shmsem, shmflag;
	int choice;
	int *bool_changed;
	student * students_info_table;
	int* size;
	struct sembuf pop, vop;
	int semid_mutex = semget(2222, 1, 0777 | IPC_CREAT);

	// Do not start Y before process X has started
    key_t start_key = 2000;
    int sstart = shmget(start_key, sizeof(int), IPC_CREAT | 0777);
    if (sstart == -1)
    {
        perror("shmget start key");
        exit(EXIT_FAILURE);
    }

    int* go_flag = shmat(sstart, 0, 0);

    while (*go_flag != 21)
    	printf("yo\n");;

    pop.sem_num = 0;
    vop.sem_num = 0;
    pop.sem_flg = 0;
    vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;




	// NO IPC_CREAT flag here, coz they have to be created by X only
	shmsem = shmget(4440, sizeof(int), 0666);
	shmsize = shmget(3330, sizeof(int), 0666);
	shmid = shmget(2220, 100*sizeof(student), 0666); 
	shmflag = shmget(9990, sizeof(int), 0666);

	// check attachment errors
	check_errors(shmid, shmflag, shmsize, shmsem, &size, &students_info_table, &bool_changed);

	printf("No Error Encountered in Attaching to memory\n");	



	// keep looping
	printf("At any time enter 0 to stop your process.\n\n\n");
	while(1)
	{
		printf("\n\nHello. Please enter roll number: ");
		scanf("%d", &choice);
		if(choice == 0)
			exit(EXIT_SUCCESS);

		int itr = 0;
		int flag = 0;
		// printf("SIZE: %d\n", *size);
		while(itr < *size){
			if(students_info_table[itr].roll_num == choice)
			{
				flag = 1;
				printf("First Name \t %s\n",students_info_table[itr].first_name);
				printf("Last Name \t %s\n",students_info_table[itr].last_name);
				printf("Roll Num \t %d\n",students_info_table[itr].roll_num);
				printf("cgpa \t %lf\n",students_info_table[itr].cgpa);
				break;
			}
			else{
				// printf("YO: %d\n", students_info_table[itr].roll_num);
			}
			itr++;
		}
		if(flag == 1){
			char c;
			printf("Is Modification Intended by you? [y/n]: ");
			scanf(" %c",&c);
			if (c == '0'){
				exit(0);
			}
			if(c == 'n'){
				printf("No problems then\n");
				continue;
			}
			printf("Only gpa Modification allowed, enter float GPA: ");
			float f;
			scanf("%f",&f);
			// Only make edit when X is not writing updated material to file
            P(semid_mutex);
			students_info_table[itr].cgpa = f;
			*bool_changed = 1;
			V(semid_mutex);
		}
		else{
			printf("No record found, try again\n");			
		}

	}

	return 0;
}	