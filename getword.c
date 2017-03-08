/*
*Name: Wei Leung
*Prof Name: Carroll
*Class: CS570
*1/26/2017
*/
#include "getword.h"

int getword(char *w){
        int charCounter = 0; //counts number of letters
        int iochar; //variable for the char of stdin
        char* ogw; //char point variable
        ogw = w; //pointer to the original pointer *w to reference later

        while((iochar = getchar()) !=EOF){
		if(charCounter == 254){
			ungetc(iochar,stdin);
			*w = '\0';
			return charCounter;
		}
		//this entire block handles if iochar reaches a newline or a space
                if(iochar == 32 || iochar == 10){ //checks if newline or space
                        if(charCounter != 0){ //checks if counter is not zero
                                if(strcmp(ogw,"logout")==0){  //compares the string referencing the beginning of the passed in char array with the string "logout"
					*w = '\0';
                                        return -1; //returns -1 if it works
                                }
                                if(iochar == '\n'){ //puts back newline into stdin
                                        ungetc('\n',stdin);
                                        *w = '\0'; //finishes up the current string
                                        return charCounter;
                                }
                                else{ //prints standard string
                                        *w = '\0';
                                        return charCounter; //30
                                }
                        }
                        else if(iochar == 10 && charCounter == 0){ //handles a space followed by a newline, from input1
                                *w = '\0';
                                return 0;
                        }
                }
		else if(iochar == 92){
			iochar = getchar(); // ignore the backwards slash
			if(iochar == 10){
				ungetc(iochar,stdin);
				*w = '\0';
				return charCounter;
			}
			*w = iochar;
			charCounter++;
			*w++;
		}
                else if(iochar == 62 || iochar == 60 || iochar == 124 || iochar == 35 || iochar == 38){ //this will check metacharacters in a string
			if(charCounter != 0 && (iochar ==62 || iochar == 60 || iochar == 124 || iochar == 35 || iochar == 38)){ //meta char follwing reg string, this will handle the infinite outputs after ungetc from a string
				ungetc(iochar,stdin); //place back meta char
				*w = '\0';
				return charCounter;
			}//50
			else if (charCounter == 0 && iochar == 62 || iochar == 60 || iochar == 124 || iochar == 35 || iochar == 38){
				if(iochar == 60 || iochar == 124 || iochar == 35 || iochar == 38){
                                          *w = iochar; //80
                                          *w++;
                                          *w = '\0';
                                          charCounter++;
                                          return charCounter;
                                }
		      		else if(iochar == 62){
                                  *w = iochar;
                                  *w++;
                                  charCounter++;
                                  iochar = getchar(); // advance to the next in the stdin
                                 	if(iochar == 62){
                                        	*w = iochar;
						charCounter++;
                                        	*w++;
                                        	iochar = getchar();
                                        	if(iochar == 38){
                                                	*w = iochar;
                                                	charCounter++;
                                                	*w++;
                                                	*w = '\0';
							return charCounter; 
                          	             	}
                         	      	}
                                	else if(iochar == 38){
                                        	*w = iochar;
                                        	charCounter++; //70
                                        	*w++;
                                        	*w = '\0';
						return charCounter;
                                	}
				}
				*w = '\0';
				if(charCounter < 3){
					ungetc(iochar,stdin);
				}
				return charCounter;
		}
		}
		else{
			*w = iochar;
			*w++;
			charCounter++; //90
		}
	}
	if(strcmp(ogw,"logout")==0){  //handles eof logout
		*w = '\0';         
		return -1; //returns -1 if it works
        }

        *w = '\0'; // handles premature EOF
        if(charCounter == 0){
                *w = '\0';
                return -1;
        }
        return charCounter; //97
}

