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



  char **parse(char *cmd)
  {	
  	int size = 128;
  	char **tokens = malloc(size * sizeof(char*));
  	char *token;
  	char *svptr;
  
  	if(!tokens)
  	{
  		fprintf(stderr,"allocation error\n");
  		exit(1);
  	}
  
  	token = strtok_r(cmd, " \n\t\r\a", &svptr);
  	int pos = 0;//position tracker
	
  	while(token != NULL)
  	{
		tokens[pos] = token;//put token into tokens

		pos++;
  		//initial set memory not enough
  		if(pos >= size)
  		{
  			size += size;//double the size
  			tokens = realloc(tokens, size* sizeof(char*));
  			if(!tokens)
  			{
  				//reallocate failed
  				fprintf(stderr, "reallocation error\n");
  				exit(1);
  			}
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
		int slen = strlen(line);
		char subarg[10][100];
		int cutpos = 0;
		int j = 0;
		for(int i = 0; i< slen; i++){
			if(line[i] == '&'){
				//printf("%d", i);
				strncpy(subarg[j], line, i);
				cutpos = i;				
				j++;
			}
		}
		printf("%s",subarg[0]);
	return 0;
  }
