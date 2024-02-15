#include <unistd.h> // fork(), execve(const char* pathname,char* const _Nullable argv[], char* const _Nullable envp[]);
#include <stdlib.h> // exit()
#include <errno.h> 
#include <stdio.h>  // printf etc.


int
initiateProgram(char* path)
{
	int pid = fork();
	if(pid == 0) 
	{
		// work the app and then go away, please.
		int ret = execve(path,NULL,NULL);
		if (ret!=0) printf("A problem encountered with work in %s.\n",path);
		exit(0);
	}
	else if(pid == -1) {
		printf("Woah. Program can't be initiated. ErrorCode:%i\n",errno);
	}
}
