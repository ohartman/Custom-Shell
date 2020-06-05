/*
  Implements a minimal shell.  The shell simply finds executables by
  searching the directories in the PATH environment variable.
  Specified executable are run in a child process.

  AUTHOR: PUT YOUR NAME HERE
*/

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

/*
  Read PATH environment var and create an array of dirs specified by PATH.

  PRE: dirs allocated to hold MAX_ARGS pointers.
  POST: dirs contains null-terminated list of directories.
  RETURN: number of directories.

  NOTE: Caller must free dirs[0] when no longer needed.

*/
int parsePath(char *dirs[]) {
  	int i, numDirs;
  	char *pathEnv;
  	char *thePath;
  	pathEnv = (char*) malloc((strlen((char *) getenv("PATH"))+1) * sizeof(char));
  	char *nextcharptr; /* point to next char in thePath */
  	for (i = 0; i < MAX_PATHS; i++) dirs[i] = NULL;
  	pathEnv = (char *) getenv("PATH");
  	//printf("%s\n", pathEnv);
  	if (pathEnv == NULL) return 0; /* No path var. That's ok.*/

  	/* for safety copy from pathEnv into thePath */
  	thePath = (char*) malloc((strlen((char *) getenv("PATH"))+1) * sizeof(char));
  	strcpy(thePath,pathEnv);

#ifdef DEBUG
  	printf("Path: %s\n",thePath);
#endif

  /* Now parse thePath */
  	nextcharptr = thePath;

  /* 
     Find all substrings delimited by DELIM.  Make a dir element
     point to each substring.
     TODO: approx a dozen lines.
  */


  	char *token;
  	token = strtok(thePath, DELIM);
  	numDirs = 0;
  	while(token != NULL){
		//printf("%s\n", token);
		dirs[numDirs] = (char *) malloc(sizeof(token));
		dirs[numDirs++] = token;
 		token = strtok(NULL, DELIM); 
 	}

  /* Print all dirs */
#ifdef DEBUG
  	for (i = 0; i < numDirs; i++) {
    		printf("%s\n",dirs[i]);
  	}
#endif
    
  	return numDirs;
}


/*
  Search directories in dir to see if fname appears there.  This
  procedure is correct!

  PRE dir is valid array of directories
  PARAMS
   fname: file name
   dir: array of directories
   num: number of directories.  Must be >= 0.

  RETURNS full path to file name if found.  Otherwise, return NULL.

  NOTE: Caller must free returned pointer.
*/

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

  /* Look in directories of PATH.  Use access() to find file there. */
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

/*
  Parse command line and fill the cmd structure.

  PRE 
   cmdLine contains valid string to parse.
   cmd points to valid struct.
  PST 
   cmd filled, null terminated.
  RETURNS arg count

  Note: caller must free cmd->argv[0..argc]

*/
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
  	// TODO
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
					
					printf("success");
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
						printf("success2");
					}
					else{
						printf("success1");
						printf("%ld", getpid());
						printf("\n");
					}
				}
				//free(fullpath);
				//free(cmd);
				//free(cmd->argv[]);
                //free(cmd->argc);
                //free(workingdir);
			}	
		}
		wdir++;
	}		
}
