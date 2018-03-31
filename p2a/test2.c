#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


char error_message[30] = "An error has occurred\n";



  char ***parse(char *cmd)
  {	
  	int size = 128;
	int size1 = 100;
  	char ***tokens = malloc(size1 * sizeof(char**));
	for(int i = 0; i < size1; i++){
		tokens[i] = malloc(size * sizeof(char*));
	}
  	char *token;
  	char *svptr;
  
  	if(!tokens)
  	{
  		fprintf(stderr,"allocation error\n");
  		exit(1);
  	}
  
  	token = strtok_r(cmd, " \n\t\r\a", &svptr);
  	int pos = 0;//position tracker
	int k = 0;
  	while(token != NULL)
  	{
		
		if(strcmp(token, "&") != 0){		
			tokens[k][pos] = token;//put token into tokens
			pos++;
		}
		else {
			tokens[k][pos+1] = NULL;
			k++;
			pos = 0;
		}
  		token = strtok_r(NULL," \n\t\r\a", &svptr);
  	}
  
  	tokens[pos] = NULL;//last arg should be NULL
	//free(start);
  	return tokens;
  }
  int main(int argc, char * argv[]) {

		char * line = NULL;
		size_t len = 0;
		//size_t read;
		char** args;
		printf("wish> ");
		getline( & line, & len, stdin);
		args = parse(line);
		int i = 0;
		char sub[10][10][128];
		int j = 0;
		int k = 0;
		while(1){
			if(args[i] == NULL) break;
			if(strcmp(args[i], "&") != 0){
				strcpy(sub[k][j], args[i]);
				j++;
			}
			else{
				sub[k][j+1] = NULL;
				j = 0;				
				k++;				
			}
			i++;
		}
		printf("%s\n", sub[0][0]);
		printf("%s\n", sub[1][0]);
	return 0;
  }
