#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(){
	pid_t cpid;
	if((cpid = fork()) == -1){
		perror("");
		exit(0);
	}
	else if(cpid == 0){
		char** name = malloc(sizeof(char*));
		name[0] = malloc((strlen("shell.c")+1)*sizeof(char));
		strcpy(name[0], "shell.c");
		if(execl("shell", "shell", (void *)NULL)==-1)
			perror("Run.c: ");
	}
	else{
		wait(NULL);
	}
	return 0;
}
