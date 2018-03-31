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
  	return tokens;
  }
  
  int main(int argc, char * argv[]) {

        	if(argc != 1)
    	{
    		//wrong
    		write(STDERR_FILENO, error_message, strlen(error_message));
    		exit(1);
    	}
        
      char * line = NULL;
      size_t len = 0;
      //size_t read;
      char** args;
      while(1){
        printf("wish> ");
        getline( & line, & len, stdin);
        //if(read != -1){
        //  printf("%s", line);
        //}
        args = parse(line);
        for(int i = 0; i< 100; i++){
        if(strcmp(args[i], "ls") == 0){
          int rc = fork();
	  if(rc == 0){
		//child process
		char *myargv[10];
		myargv[0] = strdup("ls");
		myargv[1] = NULL;
		execv(myargv[0], myargv);
	}
	  else{
		rc = (int) wait(NULL);
		printf("aaaaaa");
	}
        }
        else if(args[i] == NULL){
          break;
        }  
        }
      }
    	free(args);
//	free(myargvs[]);
	return 0;
  }
