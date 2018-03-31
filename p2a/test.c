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
	


	int batchmode = 0;
	char *filename;
	FILE *fp;
	if(argc == 1) {
		batchmode = 0;
	}
	else if(argc == 2) {
		filename = argv[1];
		fp = fopen(filename, "r");
		if (fp == NULL) {
			printf("cannot open file\n");
			exit(1);
		}
		batchmode = 1;
	}
	
	int stdin_fd = dup(STDIN_FILENO);
	int stdout_fd = dup(STDOUT_FILENO);
	char ** paths = malloc(1000 * sizeof(char*));
	int default_path = 1;
	int run = 1;
	
	while(run){


		int errorflag = 0;
		char * line = NULL;
		size_t len = 0;
		//size_t read;
		char** args;					
		if(batchmode == 1){

			if(getline( & line, & len, fp) == -1){
				printf("end of file\n");
				exit(0);
			}
		}
		else{
			printf("wish> ");
			getline( & line, & len, stdin);
		}
		args = parse(line);	
		//if no arg, skip
		if(args[0] == NULL) continue;


		//build-in commend
		if(strcmp(args[0], "path") == 0){
			default_path = 0;
			//char *path;
			int i = 1;
			while(1){
				paths[i-1] = args[i];
				if(args[i] == NULL) break;				
				//printf("%s\n", args[i]);	
				i++;			
			}
		}
		else if(strcmp(args[0], "exit") == 0){
			if(args[1] != NULL){
				 write(STDERR_FILENO, error_message, strlen(error_message)); 
			}
			else exit(0);
		}
		else if(strcmp(args[0], "cd") == 0){
			if(args[1] != NULL && args[2] == NULL){
				if(chdir(args[1]) != 0) {
					printf("cd error\n");
				}
			}
			else write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else{
			//basic function
			char *cmd = args[0];
			char * path = (char *) malloc(1 + 5 * sizeof(char));
			strcpy(path, "/bin/");	
			if(!default_path) {
				if(paths[0] == NULL){
					path = NULL;
					//free(paths);
					printf("NULL path\n");
				}
				else{
					int i = 0;
					while(1)
					{
						if(paths[i] == NULL) break;
						
						path = (char *) realloc (path, 1 + strlen(paths[i]));
						strcpy(path, paths[i]);
						//printf("%s\n", path);
						//printf("%c", path[strlen(path) -1]);
						if(path[strlen(path) -1] != '/'){
							
							strcat(path, "/");			
						}
						//printf("%s\n", path);
						char * start = (char *) malloc(1 + strlen(cmd)+ strlen(path) );
						strcpy(start, path);
						strcat(start, cmd);
						if(access(start, X_OK) == 0){
							
							free(start);
							break;				
						}
					i++;
					}

				}
			}

			if(path != NULL){
				char * start = (char *) malloc(1 + strlen(cmd)+ strlen(path) );			
				strcpy(start, path);
				strcat(start, cmd);
				args[0] = start;
			}
			else{
				errorflag = 1;			
			}

			//redirect
			int j = 0;
			while(1){
				if(args[j] == NULL) break;
				if(strcmp(args[j], ">") == 0){
					if(args[j+1] == NULL || args[j+2] != NULL) {
						errorflag = 1;
						break;
					}
					close(1);
					char *redirfile = strdup(args[j+1]);
					int fd_out = open(redirfile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
					dup2(fd_out, 1);
					args[j] = NULL;
					break;
				}
				j++;
				
			}		
			if(errorflag) {
				write(STDERR_FILENO, error_message, strlen(error_message));	
				continue;
			}



			//excute
			int pid = fork();
			if(pid == 0){
				//child process

				execv(args[0], args);
				
			}
			else{
				pid = (int) wait(NULL);
				printf("end\n");
				//dup2(STDOUT_FILENO, 1);
			}
		}
	dup2(stdin_fd, 0);			
	dup2(stdout_fd, 1); 
	}
	//free(paths);
	return 0;
  }
