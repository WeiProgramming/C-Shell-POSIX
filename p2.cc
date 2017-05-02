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
void pipeExec();
int parse();

char s[STORAGE*5];
char *newargv[MAXITEM];
char *left , *right;		//points to the in/out file in s
int rVal;			//will detect if there is a EOF character
int backGroundFlag = 0;
int pipeFlag = 0;
int lFlag = 0;
int rFlag = 0; 
int exitFlag = 0;//flag for !99 or other input not !1-9
int poundFlag = 0;
int appendFlag = 0;
int pipeLoc = 0; //This holds the second newargv position for pipe
int output_FD, input_FD ,devnull_FD;	//this is used for redirection
int inputFD; //for the script file 
int promptNumber = 1;

//using global in order to handle history
int p = 0;
int offset = 0;
int newargLoc = 0; // this will be the a pointer to the point in newargv, support history
int overWritePointer = 0;
int bangLoc = 0; //for !! recall
//history locations
int one = 0;
int two = 0;
int three = 0;
int four = 0;
int five = 0;
int six = 0;
int seven = 0;
int eight = 0;
int nine = 0;
int ten = 0;

int num = 0;

// file rules
int flags = O_CREAT | O_EXCL | O_RDWR ;

void sigcatcher(int signum){
        exit(0);
}

int main(int argc, char* argv[]){
	signal(SIGTERM,sigcatcher);
	pid_t pid, kpid;
	int numWords;
	poundFlag = 0;
	
	if(argv[1] != NULL && (strcmp(argv[1],"<") != 0)){
		poundFlag++;
		inputFD = open(argv[1],O_RDONLY);
		dup2(inputFD,STDIN_FILENO);
		close(inputFD);
	}
	for(;;){
		printf("%%%d%% ",promptNumber);
		
		numWords = parse();
		if(rVal == -2){
			fprintf(stderr,"Unmatched '.\n");
			continue;
		}
		if(rVal == -1){  //if eof is returned we stop the shell
			break;
		}
		if(numWords == 0){	//If no words are inputted, re-issue the prompt
			continue;
		}
		if(newargv[newargLoc] == NULL){		
			fprintf(stderr,"Invalid null command.\n");
			continue;
		}
		if(exitFlag != 0){
			fprintf(stderr,"History only works with the first 9 commands.\n");
			continue;
		}
		//increments each processed line
		promptNumber++;
		
		if(strcmp(newargv[newargLoc],"cd") == 0){
			if(newargv[newargLoc+1] == NULL){ //if just cd with no arguments
                char *homeVar = getenv("HOME");
                int a = chdir(homeVar);
                if(a == 0){
                	printf("success");
                	printf("\n");
                	continue;
                }
                else{
                	perror("Fail");
                	continue;
            	}
            }
            else if(newargv[newargLoc+1] != NULL && newargv[newargLoc+2] == NULL || poundFlag != 0){ //if cd has an argument
            	int b = chdir(newargv[newargLoc+1]);
            	if(b == 0){
                	printf("success2");
                	printf("\n");
                    continue;
                } 
                else{
                	perror("Fail non existant file");
                	continue;
                }
            }
            else{ //fails if cd has more than one argument
            	perror("cd only works for 0 or 1 arguments");
            }
            continue;
		}
		
		if(appendFlag != 0 && rFlag == 0){
			if(appendFlag > 2){ 
				perror("Too many output metacharacters.\n");
				continue;
			}
			if(right == NULL){
				fprintf(stderr,"No file for redirection.\n");
				continue;
			}
			if((output_FD = open(right,O_RDWR | O_APPEND)) < 0){ //only if fails we reissue the prompt
				fprintf(stderr,"%s: Already Exists.\n",right);
				appendFlag = 0;
				continue;
			}
		}

		if(rFlag != 0 && appendFlag == 0){
			if(rFlag > 2){ 
				perror("Too many output metacharacters.\n");
				continue;
			}
			if(right == NULL){
				fprintf(stderr,"No file for redirection.\n");
				continue;
			}
			if((output_FD = open(right,flags,0400|0200|0100)) < 0){ //only if fails we reissue the prompt
				fprintf(stderr,"%s: Already Exists.\n",right);
				rFlag = 0;
				continue;
			}
		}

		if(lFlag != 0){
			if(lFlag > 2){
				fprintf(stderr,"Too many input metacharacters.\n");
				continue;
			}
			if(left == NULL){
				fprintf(stderr,"No file for redirection.\n");
				continue;
			}
			if((input_FD = open(left,O_RDONLY)) < 0){
				fprintf(stderr,"Failed to open: %s\n",left);
				continue;
			}
		}
		if(pipeFlag != 0){
			if(pipeFlag > 1){
				perror("Too many pipe metacharacters");
				continue;
			}
			else{
				pipeExec();
				continue;
			}
		}
		
		fflush(stdout);
		fflush(stderr);

		if((kpid = fork()) == 0){
			if(backGroundFlag != 0 && lFlag == 0){//sending stdin to dev/null
				int flags = O_RDONLY;
				if((devnull_FD = open("/dev/null",flags)) < 0){
					fprintf(stderr,"Failed to open /dev/null.\n");
					exit(9);
				}
				dup2(devnull_FD,STDIN_FILENO);
				close(devnull_FD);
			}

			if(rFlag != 0){
				dup2(output_FD,STDOUT_FILENO);
				close(output_FD);
			}
			if(appendFlag != 0){
				dup2(output_FD,STDOUT_FILENO);
				close(output_FD);
			}

			if(lFlag != 0){
				dup2(input_FD,STDIN_FILENO);
				close(input_FD);
			}
			//after initial conditions child runs the command
			if((execvp(*(newargv+newargLoc), newargv+newargLoc)) < 0){
				fprintf(stderr,"%s: Command not found.\n",newargv[newargLoc]);
				exit(9);
			}
				
		}
		//when bg we are sending stdin to dev/null
		if(backGroundFlag !=0){	
			printf("%s [%d]\n", *newargv , kpid);
			backGroundFlag = 0;
			continue;
		}
		//the for loop will wait for completion of child before running
		else{	
			for(;;){
				pid = wait(NULL);
				if(pid == kpid)
					break;
			}
		}
	}
	killpg(getpid(),SIGTERM);
	printf("p2 terminated.\n");
	exit(0);
}


int parse(){
	int wordCounter = 0; //counts the number of words resets every call to keep track of one command at a time
	left = NULL;
	right = NULL;
	appendFlag = 0;
	exitFlag = 0;
	lFlag = 0;
	rFlag = 0;
	backGroundFlag = 0;
	pipeFlag = 0;
	pipeLoc=0;	
	//house keeping
	if(num >= 10){ //if we go over 10 newargLoc will constantly be rewriting at newargv point at ten and buffer will be rewriting in the same position
    	p = ten;
    	offset = overWritePointer;
    }
	if(num != 0 && num < 10){
		p++;
		newargLoc = p; //whenever function is called then newargLoc is marked at one above the null
	}
	else if(num == 0){ //if it's the first one
		newargLoc = p;
	}
	//start of the parsing program
	for(;;){
		rVal = getword(s + offset);
		if(rVal == 0){
			break;
		}
		if(rVal == -1){ //will check if logout is present
			if(((strcmp((s+offset),"logout")) == 0) && wordCounter != 0){ //will continue if logout isn't the only word on the command line
				newargv[p] =  s + offset;
				p++;
				s[offset+ 6] = '\0'; //we can remove this, i think
				offset += 6+1;
				wordCounter++;
				continue;
			}
			break;
		}
		
		if(*(s+offset) == '!'){
			if(rVal >= 3){
				exitFlag++;
			}
			if(*(s+offset+1) == '!'){ //if bang bang
				newargLoc = bangLoc;
			}
			if(*(s+offset+1) == '1'){
				newargLoc = one;
			}
			if(*(s+offset+1) == '2'){
				newargLoc = two;
			}
			if(*(s+offset+1) == '3'){
				newargLoc = three;
			}
			if(*(s+offset+1) == '4'){
				newargLoc = four;
			}
			if(*(s+offset+1) == '5'){
				newargLoc = five;
			}
			if(*(s+offset+1) == '6'){
				newargLoc = six;
			}
			if(*(s+offset+1) == '7'){
				newargLoc = seven;
			}
			if(*(s+offset+1) == '8'){
				newargLoc = eight;
			}
			if(*(s+offset+1) == '9'){
				newargLoc = nine;
			}
		}
		
// 		this will check # if and only if it's by itself
		if(poundFlag != 0){
			if(*(s+offset) == '#'){
				poundFlag++;
				s[offset+ 1] = '\0';
				offset += 1+1;
				wordCounter++;
				continue;
			}
			s[offset+ rVal] = '\0'; //for the logout
			offset += rVal+1;
			wordCounter++;
			continue;
		}
		//checking for pipe symbol
		if(*(s+offset) == '|'){
			s[offset+ rVal] = '\0'; //we can remove this, i think
			offset += rVal+1;
			wordCounter++;
			newargv[p] = NULL;
			p++;
			pipeLoc= p;
			pipeFlag++;
			continue;
		}
		//checks for background	
		else if(*(s+offset) == '&'){	//background check
			//checks if input/output file is non existant we exit
			 if((left == NULL && right == NULL) && (lFlag == 1 || rFlag == 1)){ 
				break;
			}
			backGroundFlag++;
			break;
		}

		else if((*(s+offset) == '<' && rVal == 1) || lFlag == 1){	//sets the inflag <
			lFlag++;
			if(lFlag == 2)
				left = s + offset;
		}

		else if((*(s+offset) == '>') || rFlag == 1 || appendFlag == 1){ //sets the outflag >
			if((*(s+offset+1) == '>')){ //checking for >>
				appendFlag++;
				if(appendFlag == 2){
					right = s + offset; //newargv will not point to files
				}
			}
			else{
				rFlag++;
				if(rFlag == 2){
					right = s+offset;
				}
			}
		}
		else{
			newargv[p] =  s + offset;
			p++;
		}

		s[offset+ rVal] = '\0'; //for the logout
		offset += rVal+1;
		wordCounter++;
	}
	if(rVal == 0 && wordCounter == 0){ //if nothing was entered we just get out
			return wordCounter;
	}
	num++;
    if(num == 1){
        one = newargLoc;
		bangLoc = newargLoc;
    }
    if(num == 2){
        two = newargLoc;
		bangLoc = newargLoc;
    }
    if(num == 3){
        three = newargLoc;
		bangLoc = newargLoc;
    } 
    if(num == 4){
        four = newargLoc;
		bangLoc = newargLoc;
    } 		
    if(num == 5){
        five = newargLoc;
		bangLoc = newargLoc;
    } 		  
    if(num == 6){
        six = newargLoc;
		bangLoc = newargLoc;
    } 	
    if(num == 7){
        seven = newargLoc;
		bangLoc = newargLoc;
    }
    if(num == 8){
        eight = newargLoc;
		bangLoc = newargLoc;
    } 		
    if(num == 9){
        nine = newargLoc;
		bangLoc = newargLoc;
    }	
    if(num == 10){
    	ten = newargLoc;
		bangLoc = newargLoc;
    	overWritePointer = offset;
    }
	newargv[p] = NULL;
	return wordCounter;
	
}
void pipeExec(){
   	pid_t pid,kpid,kkpid;
   	
	if((kpid = fork()) == 0){
		int fildes[2];
		pipe(fildes);
		fflush(stdout);
		fflush(stderr);
		
		if((kkpid = fork()) == 0){
		//first Run
      		dup2(fildes[1],STDOUT_FILENO);
      		close(fildes[0]);
      		close(fildes[1]);
			if(lFlag != 0){
				dup2(input_FD,STDIN_FILENO);
				close(input_FD);
			}
      		if((execvp(*(newargv+newargLoc), newargv+newargLoc)) < 0){	//executing command
				fprintf(stderr,"%s: Command not found.\n",newargv[newargLoc]);
				exit(9);
			}
   		}
   		//second run
      	dup2(fildes[0],STDIN_FILENO);
      	close(fildes[0]);
      	close(fildes[1]);
		if(rFlag != 0){
			dup2(output_FD,STDOUT_FILENO);
			close(output_FD);
		}
			
      	if((execvp(newargv[pipeLoc], newargv+pipeLoc)) < 0){	//executing command
			fprintf(stderr,"%s: Command not found.\n",newargv[newargLoc]);
			exit(9);
		}
   	}  
	if(backGroundFlag != 0 && lFlag == 0){		//sending STDIN to devNull
		int devnull;
		int flags = O_RDONLY;
		if((devnull = open("/dev/null",flags)) < 0){
			fprintf(stderr,"Failed to open devnull file\n");
			exit(9);
		}
		dup2(devnull,STDIN_FILENO);
		close(devnull);
	}
	if(backGroundFlag !=0){	//when & you will place in background and set STDIN to /dev/null
		printf("%s [%d]\n", *(newargv+newargLoc),kpid);
		backGroundFlag = 0;
	}
	else{	
		for(;;){
			pid = wait(NULL);
			if(pid == kpid){
				break;
			}
		}
	}
}
