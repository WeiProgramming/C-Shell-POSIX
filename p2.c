/*
*Name: Wei Leung
*Prof Name: Carroll
*Class: CS570
*2/28/2017
*/

#include <time.h>
#include <errno.h>
#include "getword.h"
#include "p2.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <dirent.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>

int promptNumber = 1;
int flags;
int logoutFlag = 0;
int leftFlag = 0;
int rightFlag = 0;
int noInput = 0;
int backgrounded = 0;
pid_t  pid, kpid;
int fileexists = 0;

void sigcatcher(int signum){
	exit(0);
}


//handles the builtin and logout signal produced by parse
main(){
	 char *newargv[MAXITEM];
	 char rArray[STORAGE];
	 char lArray[STORAGE];
	 char s[STORAGE];
	 char bang[STORAGE];
	 int iochar;
	 
	 signal(SIGTERM,sigcatcher);
	 
	 printf("%%%d%% ",promptNumber);

	 for(;;){
	 	iochar = getchar();
	 	if(iochar == '\n'){
	 		printf("%%%d%% ",promptNumber);
	 		continue;
	 	}
	 	else{
	 		ungetc(iochar,stdin);
	 		
	 		leftFlag = 0;
	 		rightFlag = 0;
	 		backgrounded = 0;
	 		logoutFlag= 0;

	 		
	 		parse(s,newargv,lArray,rArray,bang);

	 		//handling cd
			if(newargv[0] != NULL){
	 			if(strcmp(newargv[0],"cd") == 0){
	 				if(newargv[1] == NULL){
	 					char *homeVar = getenv("HOME");
	 					int a = chdir(homeVar);
	 					if(a == 0){
							printf("%%%d%% ",promptNumber);
                    		printf("success");
                    		printf("\n");
							printf("%%%d%% ",promptNumber);
	 						continue;
						}
						else{
							perror("Fail");
							continue;
						}
					}
	 				if(newargv[1] != NULL && newargv[2] == NULL){
	 					int b = chdir(newargv[1]);
						if(b == 0){
	 						printf("%%%d%% ",promptNumber);
                			printf("success2");
                			printf("\n");
							printf("%%%d%% ",promptNumber);
	 						continue;
	 					} 
	 					else{
	 						perror("Fail");
	 						continue;
	 					}
	 				}
	 				else{
	 						printf("%%%d%% ",promptNumber);
							perror("cd only works for 0 or 1 arguments");
							continue;
	 				}
	 			}
	 			if (backgrounded == 1){
   					pid = redirection(lArray,rArray,newargv);
   					printf("%s [%d]\n", newargv[0], pid);
   					fflush(NULL);
   					continue;
   				}
	 			if(logoutFlag == 1){
					printf("p2 terminated.\n");
					exit(0);
				}
				redirection(lArray, rArray,newargv);
	 		}
		}
	}
	killpg(getpid(),SIGTERM);
	printf("p2 terminated.\n");
	exit(0);
}

/*
*Parse will populate the the storage and using the newargv as pointed terminated by a null
*
*/
parse(char *s,char **newargv, char *lArray, char *rArray,char *bang){
	int p = 0;
	int RVal=0;


	newargv[0] = s;
	//echo hello
	for(;;){
		RVal = getword(s);
		
		if(((strcmp(newargv[p],"logout")) == 0)){ //checks if line is empty when logout is created
			if((strcmp(newargv[0],"logout")) == 0){
				logoutFlag = 1;
				return;
			}
			s = s+6+1;			
			p++;
			newargv[p] = s;
			continue;
		}
		
		if(RVal == -1){
			newargv[p] = NULL;
			printf("%%%d%% ",promptNumber);
			printf("p2 terminated.\n");
			exit(1);
		}
		
		if(RVal == 0){
			newargv[p] = NULL;
			return;
		}
		if(RVal == 1){
	   		if(*s =='&'){
	   			s = s+1+1;
	   			RVal = getword(s);
	   			p++;
				newargv[p] = s;
	   			if(RVal == 0){ //if it's a newline meaning & is at end
      				newargv[p]= NULL;
      				backgrounded = 1;
      				return;
      			}
      		}
			if(*s == '>'){
				rightFlag = 1;
				RVal = getword(rArray);
				continue;
			}
			if(*s == '<'){
				leftFlag = 1;
				RVal = getword(lArray);
			}
		}
		s = s+RVal+1;
		p++;
		newargv[p] = s;
		continue;		
	}
}

//filedescriptor stuff
int redirection(char *lArray, char *rArray,char **newargv){
	int execReturn;
	int status;
	
	flags = O_CREAT | O_RDWR;
	
	fflush(stdout); //flush out all of stdout so we are ensured and empty output
	fflush(stderr);
	
	kpid = fork();
	if(-1 == kpid){
		perror("cannot fork");
		exit(1);
	}
	else if(0 == kpid){
		kpid = getpid();
		printf("%%%d%% ",promptNumber);
		// input redirection <
		if(leftFlag == 1){
			int input_FD = open(lArray,O_RDONLY); //reading only
			dup2(input_FD,STDIN_FILENO); //places into fildes[0]
			leftFlag == 0;
			if(close(input_FD) < 0){
				perror("failed to close input");
			}
		}
		//echo hello > hello.txt
		if(rightFlag == 1){
				// > output.txt
				rightFlag = 0;
				int output_FD = open(rArray,flags,0400|0200); //if not an already existing file we create it and write into it
				dup2(output_FD,STDOUT_FILENO); // filedes[1] output
				if(close(output_FD) < 0){
					perror("failed to close output");
				}
		}
		execReturn = execvp(newargv[0],newargv);
		if(execReturn == -1){
			perror("Exec Failed");
			exit(9);
		}
	}
	else{
		if(backgrounded == 1){
			return kpid;
		}
		while ((kpid = wait(&status)) > 0);
		printf("%%%d%% ",promptNumber);
	}
}
