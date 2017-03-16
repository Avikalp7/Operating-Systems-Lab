/*
**************************************************************************
Author: Avikalp Srivastava
Roll: 14CS10008
Assignment 1, Part-2 OS Lab Spring 2017
Problem Statement: Sort on integers in file specified by command line args
**************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_FILENAME_LEN 100
#define MAX_ARR_SIZE 1000

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}


int main(int argc, char* argv[])
{
	char fname[MAX_FILENAME_LEN];
	strcpy(fname, argv[1]);


	FILE *fp = fopen(fname, "r");
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
    printf("\n");


    qsort (arr, len_arr, sizeof(int), compare);


	printf("\nSorted Integers :\n");
    for(i = 0; i < len_arr; i++)
    {
    	printf("%d ",arr[i]);
    }     
    printf("\n");


	return 0;
}
