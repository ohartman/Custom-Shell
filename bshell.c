//  AUTHOR: Owen Hartman

#include "bshell.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

int parsePath(char *dirs[]);
char *lookupPath(char *fname, char **dir,int num);
int parseCmd(char *cmdLine, Command *cmd);


int parsePath(char *dirs[]) {
  	int i, numDirs;
  	char *pathEnv;
  	char *thePath;
  	pathEnv = (char*) malloc((strlen((char *) getenv("PATH"))+1) * sizeof(char));
  	char *nextcharptr; /* point to next char in thePath */
  	for (i = 0; i < MAX_PATHS; i++) dirs[i] = NULL;
  	pathEnv = (char *) getenv("PATH");
  	if (pathEnv == NULL) return 0; /* No path var. That's ok.*/

  	/* for safety copy from pathEnv into thePath */
  	thePath = (char*) malloc((strlen((char *) getenv("PATH"))+1) * sizeof(char));
  	strcpy(thePath,pathEnv);

#ifdef DEBUG
  	printf("Path: %s\n",thePath);
#endif
  	nextcharptr = thePath;
  	char *token;
  	token = strtok(thePath, DELIM);
  	numDirs = 0;
  	while(token != NULL){
		dirs[numDirs] = (char *) malloc(sizeof(token));
		dirs[numDirs++] = token;
 		token = strtok(NULL, DELIM); 
 	}

#ifdef DEBUG
  	for (i = 0; i < numDirs; i++) {
    		printf("%s\n",dirs[i]);
  	}
#endif
    
  	return numDirs;
}

char *lookupPath(char *fname, char **dir,int num) {
  	char *fullName; // resultant name
  	int maxlen; // max length copied or concatenated.
  	int i;
	
  	fullName = (char *) malloc(MAX_PATH_LEN);
  	/* Check whether filename is an absolute path.*/
  	if (fname[0] == '/') {
    		strncpy(fullName,fname,MAX_PATH_LEN-1);
    	if (access(fullName, F_OK) == 0) {
      		return fullName;
    }
  }

  else {
    	for (i = 0; i < num; i++) {
      		// create fullName
      		maxlen = MAX_PATH_LEN - 1;
      		strncpy(fullName,dir[i],maxlen);
      		maxlen -= strlen(dir[i]);
      		strncat(fullName,"/",maxlen);
      		maxlen -= 1;
      		strncat(fullName,fname,maxlen);
      		// OK, file found; return its full name.
      		if (access(fullName, F_OK) == 0) {
				return fullName;
     			 }
   	 	}
  	}
  	fprintf(stderr,"%s: command not found\n",fname);
  	free(fullName);
  	return NULL;
}

int parseCmd(char *cmdLine, Command *cmd) {
  	int argc = 0; // arg count
  	char* token;
  	int i = 0;

  	token = strtok(cmdLine, SEP);
  	while (token != NULL && argc < MAX_ARGS){    
    		cmd->argv[argc] = strdup(token);
    		token = strtok (NULL, SEP);
    		argc++;
  }

  	cmd->argv[argc] = NULL;  
  	cmd->argc = argc;

#ifdef DEBUG
  	printf("CMDS (%d): ", cmd->argc);
  	for (i = 0; i < argc; i++)
   		printf("CMDS: %s",cmd->argv[i]);
  		printf("\n");
#endif
  
  return argc;
}

/*
  Runs simple shell.
*/
int main(int argc, char *argv[]) {
	
	
	char *dirs[MAX_PATHS]; // list of dirs in environment
  	int numPaths;
  	char cmdline[LINE_LEN];
	Command *cmd = malloc(sizeof(Command));
	pid_t pid, ww;
	int *status;
	char* hum = "/home/";
	int wdir = 0;
	char* workingdir = (char*)malloc(sizeof(char)*1000);
  	numPaths = parsePath(dirs);
    
  	// while loop begins the terminal i/o loop ends when user inputs either 'exit' or 'ctrl+c'
  	while(1){
		int elsecheck = 0;
		int check = 0;
		char* get;
		getcwd(workingdir, 512);
		char* fullpath = malloc(sizeof(char)*100);
		printf("%s%s",workingdir ,PROMPT);
		get = fgets(cmdline,LINE_LEN,stdin);
	
		if(cmdline != NULL && cmdline[0] != '\n'){
			parseCmd(get, cmd);
			if(strcmp(cmd->argv[0],"exit")== 0){
				elsecheck = 1;
				break;
				
			}
			if(strcmp(cmd->argv[0], "cd")==0){
					if(cmd->argv[1] != NULL){
							if(chdir(cmd->argv[1])<0){
								printf("error");
							}
							getcwd(workingdir, 512);
						}
						else{
							chdir(hum);
					}
				elsecheck = 1;
				}
			if(elsecheck == 0){
				fullpath = lookupPath(get, dirs, numPaths);
				pid = fork();
				if(strcmp(cmd->argv[argc-1], "&")==0){
					check = 1;
				}
				
				if(pid == 0){
					printf("%s", fullpath);
					printf("%ld", getpid());
					printf("\n");
					if(fullpath != NULL){
						execv(fullpath, cmd->argv);
					}
				}
				else{
					if(check == 0){
						ww = waitpid(pid, &status, WNOHANG);
					}
					else{
						printf("%ld", getpid());
						printf("\n");
					}
				}
			}	
		}
		wdir++;
	}		
}
