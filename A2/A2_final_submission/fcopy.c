/*
**************************************************************************
Author: Avikalp Srivastava
Roll: 14CS10008
Assignment 2, Part-1 OS Lab Spring 2017
Problem Statement: Copying contents of file1 to file2 using two pipes.
**************************************************************************
*/

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define BUFFER_SIZE 100

int main(int argc, char *argv[]){;
	/* Ensuring correct command line input */
	if (argc != 3) {
	   fprintf(stderr, "Usage: %s <filename 1> <filename2>\n", argv[0]);
	   exit(EXIT_FAILURE);
	}

	/* Opening File to read */
	int fd1 = open(argv[1], O_RDONLY);
	if(fd1 == -1){
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}
	/* Opening file to write to */
	int fd2 = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
	if(fd2 == -1){
		perror(argv[2]);
		exit(EXIT_FAILURE);
	}

	/* File Descriptors for pipe */
	int pipefd1[2];
	int pipefd2[2];
	pid_t cpid;
	/* buffer for pipe */
	char buf[BUFFER_SIZE];
	/* Checking for error in pipe construction */
	if (pipe(pipefd1) == -1) {
       perror("pipe1");
       exit(EXIT_FAILURE);
    }

    if (pipe(pipefd2) == -1) {
       perror("pipe2");
       exit(EXIT_FAILURE);
    }

    /* Child process created */
   cpid = fork();
   if (cpid == -1) {
       perror("fork");
       exit(EXIT_FAILURE);
   }

   if(cpid == 0) {				/* Child Process */
   	close(pipefd1[1]);			/* Pipe 1 for reading only */
   	close(pipefd2[0]);			/* Pipe 2 for writing status only */
   	int err = -1, done = 0;
   	while(1){
   		/* Read 100 bytes from pipe 1's read end */ 
   		int bytes_read = read(pipefd1[0], buf, BUFFER_SIZE);
   		/* Write bytes_read number of bytes into file2 */ 
   		int bytes_written = write(fd2, buf, bytes_read);
   		/* This is where things get flowy, erroneous behavior from child/pipe */
   		if(bytes_read != bytes_written){
   			/* Write -1 to write end of pipe2 */
   			write(pipefd2[1], &err, sizeof(err));
   			perror("child write");
   			exit(-1);
   		}
   		else{ /* Successful write case*/
   			write(pipefd2[1], &done, sizeof(done));
   			/* Exit Condition */
   			if(bytes_read < 100){
   				printf("Child says: File successfully copied\n");
   				exit(0);
   			}
   		}
   	}
   }

   else {						/* Parent Process */
   	close(pipefd1[0]);          /* Close unused read end of pipe 1*/
   	close(pipefd2[1]);			/* Close write end of pipe 2 */
   	// read(fd1, buf, 100);		
   	int bytes_read = BUFFER_SIZE;
   	int status = 0;
   	/* While number of bytes being read from file1 are not less than 100, keep going */
   	while(bytes_read >= BUFFER_SIZE){
   		bytes_read = read(fd1, buf, BUFFER_SIZE);
   		write(pipefd1[1], buf, bytes_read);
   		/* 4 byte int status is read from read end of pipe 2 */
   		read(pipefd2[0], &status, sizeof(status));
   		/* If child returns -1 or bytes_read from file1 < 100 */
   		if(status == -1 || bytes_read < 100){
   			if(bytes_read < 100)
   				exit(0);
   			else
   				exit(-1);
   		}
   	}

   }
	return 0;
}