/*
**************************************************************************
Author: Avikalp Srivastava
Roll: 14CS10008
Assignment 2, Part-2 OS Lab Spring 2017
Problem Statement: Make a mini shell.
**************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <fcntl.h>

#define BUFFER_SIZE 5000
#define LARGE_BUFFER_SIZE 50000
#define MAX_ARGS 100


// Return number of args
char** parse(char* str){
	// Space is the delimiter
	char delim[3] = " \n";
	// Allocating maximum 100 args space	
	char **args = (char **)malloc(MAX_ARGS * sizeof(char *));
	int arg_count = 0;
	// Using strtok for parsing through spaces as delimiters
	args[arg_count] = strtok(str, delim);
	while(args[arg_count]){
		args[++arg_count] = strtok(NULL, delim);
	}
	arg_count = 0;
	return args;
}

// Read user's command
char* read_cmd(){
	char *inp;
	ssize_t size = 0;
	inp = NULL; 
	getline(&inp, &size, stdin);
	return inp;
}

// Init shell
void start_shell(){
	while(1){
		char buf[BUFFER_SIZE];
		// Print prompt
		getcwd(buf, sizeof(buf));
		printf("%s> ",buf);
		// Read command
		char* cmd = read_cmd();
		char **args;
		// Parse the command
		args = parse(cmd);
		// Handling empty space
		if(args[0]==NULL)
			continue;
		// Getting num of args
		int arg_count = -1;
		while(args[++arg_count]);
		// printf("Num args: %d\n", arg_count);
		// printf("%s\n", args[0]);
		if(!strcmp(args[0], "cd")){
			if(arg_count==1){
				int cd_fail = chdir(getenv("HOME"));
				if(cd_fail){
					perror("~");
				}
			}
			else{
				if(args[1][0] == '-'){
					printf("- flags not supported yet.\n");
				}
				else{
					int cd_fail = chdir(args[1]);
					if(cd_fail){
						perror(args[1]);
						continue;
					}
				}
			}
		}

		else if(!strcmp(args[0], "pwd")){
			printf("%s\n", buf);
		}

		else if(!strcmp(args[0], "mkdir")){
			if(arg_count == 1){
				printf("mkdir: missing operand\n");
			}
			int count;
			for(count = 1; count < arg_count; count++){
				if(args[count][0] == '-'){
					printf("- flags are not supported\n");
					break;
				}
				int mkdir_fail = mkdir(args[count], 0777); 
				/*POSIX refers to the st_mode bits corresponding to the mask S_IFMT
		       (see below) as the file type, the 12 bits corresponding to the mask
		       07777 as the file mode bits and the least significant 9 bits (0777)
		       as the file permission bits.*/
				if(mkdir_fail){
					perror(args[count]);
					continue;
				}
			}
		}

		else if(!strcmp(args[0], "rmdir")){
			if(arg_count == 1){
				printf("rmdir: missing operand\n");
			}
			int count;
			for(count = 1; count < arg_count; count++){
				if(args[count][0] == '-'){
					printf("- flags are not supported\n");
					break;
				}
				int rmdir_fail = rmdir(args[count]); 
				if(rmdir_fail){
					perror(args[count]);
					break;
				}
			}
		}

		else if(!strcmp(args[0], "ls")){
			DIR *dir;
			struct dirent *ent;
			if((dir = opendir(buf))!=NULL){
				while((ent = readdir(dir))){
					if(arg_count > 1){
						if(!strcmp(args[1], "-l")){
							struct stat status;
							int file_status = stat(ent->d_name, &status);
							
							if(file_status>=0)
							{
								struct passwd *username = getpwuid(status.st_uid);
								struct group *groupname = getgrgid(status.st_gid);
								// printf("Total %d\n", ent->d_reclen);
								printf( ((S_ISDIR(status.st_mode)) ? "d" : "-"));
								printf( ((status.st_mode & S_IRUSR) ? "r" : "-"));
								printf( ((status.st_mode & S_IWUSR) ? "w" : "-"));
								printf( ((status.st_mode & S_IXUSR) ? "x" : "-"));
								printf( ((status.st_mode & S_IRGRP) ? "r" : "-"));
								printf( ((status.st_mode & S_IWGRP) ? "w" : "-"));
								printf( ((status.st_mode & S_IXGRP) ? "x" : "-"));
								printf( ((status.st_mode & S_IROTH) ? "r" : "-"));
								printf( ((status.st_mode & S_IWOTH) ? "w" : "-"));
								printf( ((status.st_mode & S_IXOTH) ? "x" : "-"));
								
								// char* time;
								// time=ctime(&status.st_atime);
								// int len = strlen(time);
								// time[len-1] = '\0';

								printf(" %d ",(int)status.st_nlink);//time,ent->d_name);
								printf(" %s ", username->pw_name);
								printf(" %s ", groupname->gr_name);
								printf(" %6d ", (int)status.st_size);
								char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
								struct tm* time_stamp;
				                time_stamp = localtime(&status.st_atim.tv_sec);
				                // printf("%s %d ", (int) dir_stat.st_nlink);
				                printf(" %s %d  %d:%02d ",month[time_stamp->tm_mon], time_stamp->tm_mday, time_stamp->tm_hour,
                       			time_stamp->tm_min);
                       			printf("  %s\n", ent->d_name);
							}
							else{
								perror(args[1]);
								break;
							}

						}
						else{
							printf("Command not supported yet\n");
							break;
						}
					}
					else{
						if(ent->d_name[0] != '.')
							printf("%s\n", ent->d_name);
					}
				}
				closedir(dir);
			} else{
				perror("");
			}
		}

		else if(!strcmp(args[0], "cp")){
			if(arg_count != 3){
				printf("Invalid Command, please use > cp filename1 filename2\n");
			}
			else{
				struct stat status1, status2;
				int stat_success1 = stat(args[1], &status1);
				int stat_success2 = stat(args[2], &status2);
				if(stat_success1==-1 || stat_success2==-1){
					perror("");
					continue;
				}

				int fd1 = open(args[1], O_RDONLY);
				if(fd1 == -1){
					perror(args[1]);
					close(fd1);
					continue;
				}
				int fd2 = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
				if(fd2 == -1){
					perror(args[2]);
					close(fd1); close(fd2);
					continue;
				}

				else{
					struct timespec t1 = status1.st_mtim;
					struct timespec t2 = status2.st_mtim;
					// printf("%d %d\n", t1.tv_sec, t2.tv_sec);
					if(t1.tv_sec > t2.tv_sec){
						char read_buffer[LARGE_BUFFER_SIZE];
						int no_of_bytes_read = read(fd1, read_buffer, LARGE_BUFFER_SIZE);
						if(no_of_bytes_read == -1){
							perror(args[1]);
							close(fd1);
							continue;
						}
						// TODO: See sample and correct flags
						int no_of_bytes_written = write(fd2, read_buffer, no_of_bytes_read);
						if(no_of_bytes_written < no_of_bytes_read){
							perror(args[2]);
							continue;
						}
					}
					else{
						printf("Copy Failed: %s was modified more recently than %s\n", args[2], args[1]);
					}
				}
				close(fd1);
				close(fd2);
			}
		}
		else if(!strcmp(args[0], "exit")){
			exit(0);
		}
		else{
			int i;
			int pipe_count = 0, pipe_bool = 0, pipe_pos[2];
			for(i = 0; i < arg_count; i++){
				if(!strcmp(args[i], "|")){
					pipe_pos[pipe_count] = i;
					pipe_bool = 1;
					pipe_count++;
				}
			}
			if(pipe_bool){
				pid_t c1pid;
				if ((c1pid = fork())== -1) {
		       		perror("fork");
		       		continue;
		   		}
		   		if(c1pid == 0){		// child process of main process
					int fd[2];
					pid_t cpid;
					/* buffer for pipe */
					// char buf[BUFFER_SIZE];
					/* Checking for error in pipe construction */
					if (pipe(fd) == -1) {
				       perror("");
				       exit(-1);
				    }
				    if ((cpid = fork())== -1) {
			       		perror("fork");
			       		exit(-1);
			   		}
			   		if(cpid == 0){	// Small Child Process 
			   			close(fd[0]);		// Close read end
			   			close(1);			// close STDOUT
			   			dup(fd[1]);		// First exec will write to STDOUT
			   			close(fd[1]);
			   			char** new_args;	// args for exec call
			   			int new_arg_count = pipe_pos[0];	// args before pipe
						new_args = malloc((new_arg_count)*sizeof(char*));
						int i;
						for(i = 0; i < new_arg_count; i++){
							new_args[i] = malloc((strlen(args[i])+1)*sizeof(char));
							strcpy(new_args[i], args[i]);

						}
						// exec a kar diya, fd1 mein stored hai output
						int failure = execv(args[0], new_args);
						if(failure){
							perror(args[0]);
							exit(0);
						}
						// execl(args[0], )
			   		}
			   		else{			// Parent Process - Child of Main Process 
			   			wait(NULL);			// Wait for child
			   			close(fd[1]);		// Close write end
			   			close(0);			// STDIN
			   			dup(fd[0]);
			   			close(fd[0]);
						int fd2[2];
						if(pipe_count == 2){
							if (pipe(fd2) == -1) {
				       			perror("");
				       			exit(0);
				    		}
				    		pid_t cpid2;
				    		if ((cpid2 = fork())== -1) {
					       		perror("fork");
					       		exit(0);
					   		}
					   		if(cpid2==0){			// Small Child 2
					   			close(fd2[0]);		// Close read end of 2nd pipe
					   			close(1);			// Close STDOUT			
					   			dup(fd2[1]);		// STDOUT now in fd2
					   			close(fd2[1]);
					   			char** new_args;	// args for exec call
					   			int last = pipe_pos[1];
					   			int new_arg_count = last - pipe_pos[0] - 1;	// args before pipe
								new_args = malloc((new_arg_count)*sizeof(char*));
								int i;
								for(i = pipe_pos[0] + 1; i < last; i++){
									new_args[i] = malloc((strlen(args[i])+1)*sizeof(char));
									strcpy(new_args[i], args[i]);
								}
								// exec b kar diya, fd2 mein stored hai output
								int failure = execv(args[pipe_pos[0] + 1], new_args);
								if(failure){
									perror(args[pipe_pos[0] + 1]);
									exit(0);
								}

					   		}
					   		else{					// Parent - Child of main process
					   			wait(NULL);		// wait for second child
					   			close(fd2[1]);		// Close write end of 2nd pipe
					   			close(0);			// Close STDIN			
					   			dup(fd2[0]);		// STDIN now in fd2
					   			close(fd2[0]);
					   			char** new_args;	// args for exec call
					   			int new_arg_count = arg_count - pipe_pos[1] - 1;	// args before pipe
								new_args = malloc((new_arg_count)*sizeof(char*));
								int i;
								for(i = pipe_pos[1] + 1; i < arg_count; i++){
									new_args[i] = malloc((strlen(args[i])+1)*sizeof(char));
									strcpy(new_args[i], args[i]);
								}
								// exec c kar diya, fd1 mein stored hai output
								int failure = execv(args[pipe_pos[1]+1], new_args);
								if(failure){
									perror(args[0]);
									exit(0);
								}

					   		}
				    		
						}
						/* NOT PIPE COUNT = 2 AREA */
						else{
							char** new_args;	// args for exec call
			   				int last = arg_count;
			   				int new_arg_count = last - pipe_pos[0] - 1;	// args before pipe
							new_args = malloc((new_arg_count)*sizeof(char*));
							int i;
							for(i = pipe_pos[0] + 1; i < last; i++){
								new_args[i] = malloc((strlen(args[i])+1)*sizeof(char));
								strcpy(new_args[i], args[i]);
							}
							// exec b kar dia
							int failure = execv(args[pipe_pos[0] + 1], new_args);
							if(failure){
								printf("here\n");
								perror(args[pipe_pos[0] + 1]);
								exit(0);
							}
						}
			   		}
				}
				else{
					wait(NULL);
				}
			}

			else{
				int new_arg_count = (!strcmp(args[arg_count-1],"&"))? arg_count - 1 : arg_count;
				// if(!strcmp(args[arg_count-1],"&"))
				pid_t cpid = fork();
				if(cpid == 0){ // child of main proc
					char** new_args;
					new_args = malloc((new_arg_count)*sizeof(char*));
					int i;
					for(i = 0; i < new_arg_count; i++){
						// printf("%s\n", args[i]);
						if(!strcmp(args[i], "<")){
							// printf("here2\n");
							new_args[i] = NULL;
							int fd1 = open(args[++i], O_RDONLY);
							if(fd1 == -1){
								perror(args[i]);
								close(fd1);
								exit(0); 
							}
							close(0);
							dup(fd1);
							close(fd1);
						}
						else if(!strcmp(args[i], ">")){
							// printf("here3\n");
							new_args[i] = NULL;
							int fd2 = open(args[++i], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
							if(fd2 == -1){
								perror(args[i]);
								close(fd2);
								exit(0); 
							}
							close(1);
							dup(fd2);
							close(fd2);
						}
						// else if(!strcmp(args[i], "|")){

						// }
						else{
							// printf("here4\n");
							new_args[i] = malloc((strlen(args[i])+1)*sizeof(char));
							strcpy(new_args[i], args[i]);
						}
						// printf("\n%d %s\n",i ,new_args[i]);
					}
					// printf("%s\n", new_args[0]);
					int failure = execvp(args[0], new_args);
					if(failure){
						// printf("yaha fail\n");
						perror(args[0]);
						exit(0);
					}
				}
				else{										// parent
					if(!strcmp(args[arg_count-1],"&")){
						// printf("waiting\n");
					}
					else
						wait(NULL);
				}
			}
			
			
		}

	}
}


int main(){
	start_shell();
	return 0;
}





