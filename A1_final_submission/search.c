/*
*********************************************************************
Author: Avikalp Srivastava
Roll: 14CS10008
Assignment 1, Part-1 OS Lab Spring 2017
Problem Statement: Linear Search over an array using parallelization
*********************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_FILENAME_LEN 100
#define MAX_ARR_SIZE 5000


// Parallelized Linear Search over arr[] b/w indices [i,j] for the integer k.
int find(int arr[], int i, int j, int k)
{
	// printf("pid = %d, i = %d, j = %d\n", getpid(), i, j);
	// Recording length of array to search in len
	int len = j - i + 1;
	
	// If length of array <= 10, do a linear search and return
	if(len <= 10)
	{
		// m is just a counter
		int m;
		for(m = i; m <= j; m++)
		{
			if(arr[m] == k)
				return 1;
		}
		return 0;
	}
	// If length of array > 10, run 2 child processes on each half and merge the result
	else
	{
		// Child Process 1 and Process 2 id's
		int p1 = 1, p2 = 1;
		// status variable to store statuses for process 1 and 2
		int status1, status2;
		p1 = fork();
		// Start of child process 1
		if(p1 != 0)
		{   // Parent Process

			// Create 2nd child process
			p2 = fork();
			// 2nd child process starts
		}
		if(p1 != 0 && p2 != 0)
		{   // Parent Process

			// Wait for both the child to terminate and store the statuses
			waitpid(p1, &status1, 0);
			waitpid(p2, &status2, 0);
			// If any of the statuses is not zero => k is found 
			return(status1 || status2);
		}
		else if(p1 == 0)
		{	// Child Process 1

			// Store in f1 whether k is found in 1st half of array
			int f1 = find(arr, i, i + len/2, k);
			f1 = f1 > 0 ? 1 : 0; 
			// exit with arg f1
			exit(f1);
		}
		else if(p2 == 0)
		{	// Child Process 2

			// Store in f2 whether k is found in 2nd half of array
			int f2 = find(arr, i + len/2 + 1, j, k);
			f2 = f2 > 0 ? 1 : 0;
			// exit with arg f2
			exit(f2);
		}
	}
}



int main()
{
	/** Getting FileName **/

	// Varaible f stores filename
	char f[MAX_FILENAME_LEN];
	// Print to user asking filename
	printf("Enter filename f without whitespaces : ");
	scanf("%s", f); 	

	/** FileName stored in f **/


	/** Reading integers from file f **/
	FILE *fp = fopen(f, "r");
	int arr[MAX_ARR_SIZE];
	int i = 0, num;
	while(fscanf(fp, "%d", &num) == 1) 
   	{
        arr[i] = num;
        i++;
    }
    fclose(fp);
    printf("The following was read from the file :\n");
    int len_arr = i;
    for(i = 0; i < len_arr; i++)
    {
    	printf("%d ",arr[i]);
    } 
    /** Integers from file stored in arr **/


    /** Find user-fed input in arr using parallelized find function **/
    int k;
    while(1)
    {
    	printf("\nEnter k : ");
    	scanf("%d",&k);
    	if(k > 0)
    		if(find(arr, 0, len_arr - 1, k) > 0)
    			printf("%d was Found!\n", k);
    		else
    			printf("%d was Not Found...\n", k);
    	else{
    		printf("Exiting.....\n");
    		exit(0);
    	}
    }
    /** Completed **/

	return 0;
}
