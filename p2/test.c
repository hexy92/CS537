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
const int MODE_SPACE = 1;
const int MODE_REDIR = 2;
const int MODE_PARA = 3;


  char **parse(char *cmd, int mode, int * endpos)
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
	
 	if(mode == MODE_SPACE){
  		token = strtok_r(cmd, " \n\t\r\a", &svptr);
	}
	else if(mode == MODE_REDIR){
		token = strtok_r(cmd, ">", &svptr);
	}
	else if(mode == MODE_PARA){
		token = strtok_r(cmd, "&", &svptr);
	}
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
	 	if(mode == MODE_SPACE){
	  		token = strtok_r(NULL, " \n\t\r\a", &svptr);
		}
		else if(mode == MODE_REDIR){
			token = strtok_r(NULL, "> ", &svptr);
		}
		else if(mode == MODE_PARA){
			token = strtok_r(NULL, "&", &svptr);
		}
  	}
  
  	tokens[pos] = NULL;//last arg should be NULL
	printf("%d", pos);
	*endpos = pos;
	//free(start);
  	return tokens;
  }



  int main(int argc, char * argv[]) {

	char** args;					
	char s[100] = "ls  pwd > out";
	int endpos = 0;
	args = parse(s, 1, &endpos);
	printf("%d", endpos);

	return 0;
  }
