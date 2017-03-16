/*
**************************************************************************
Author: Avikalp Srivastava
Roll: 14CS10008
Assignment 1, Part-2 OS Lab Spring 2017
Problem Statement: Sort on integers in file specified by command line args
**************************************************************************
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char* argv[])
{
	int id, status = 0;
	id = fork();
	if (id == 0)
	{
		/* Child Process, make it run the xterm, and sort using sort1. */
		char* xterm_command;
		xterm_command = (char *)malloc(strlen("./sort1 ") + strlen(argv[1]) + 1);
		strcpy(xterm_command, "./sort1 ");
		strcat(xterm_command, argv[1]);
		execl("/usr/bin/xterm", "/usr/bin/xterm", "-hold", "-e", "bash", "-c", xterm_command, (void*)NULL);
        /* The code should never be here if no error */ 
		perror("execvp failed:");
           exit(-1);
	}
	else {
		     /* parent process */
                /* Make the parent process wait for the child to finish */
                wait(&status);
                /* child has exited; so some  printing and exit */
                printf("PARENT SAYS: Child exited with status %d\n", status);
	     }
}
			
