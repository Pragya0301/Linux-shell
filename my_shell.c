#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/wait.h>
#include <math.h> 
#include <limits.h>
//#include <conio.h>
//#include <dir.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
int series_pid = 0;
int flag = 0;
int flag_exit = 0;

int bg_pid[128];
int bg_count = 0;



void func_cc(int signum){
	if(series_pid!=0){
		kill(series_pid, SIGKILL);
	}
	kill(-10, SIGKILL);
	flag = 1;
}

void func(int signum)
{
    for(int j=0;j<128;j++){
    	if(bg_pid[j]!=INT_MAX)
    		kill(bg_pid[j], SIGKILL);
    }
    flag_exit = 1;
}

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void shell_fn(char **sep_cmds, int mode){

	char  line[MAX_INPUT_SIZE];   
	int i=0;
	int status;
	
		if(sep_cmds[0]==NULL)
			return;

		if(strcmp(sep_cmds[0], "exit") == 0){
			int k_res = kill(0, SIGTERM);
			return;
		}
		if(strcmp(sep_cmds[0], "cd") == 0){
			char *path = sep_cmds[1];
			if(chdir(path)==0){
				//printf("changing directory");
			}
			else{
				printf("Shell:Incorrect command\n");
				return;
			}
		}
		else{
			int rc = fork();
			/*if(mode==1){

			}*/
			
			if(mode==2 || mode==0){
				series_pid = rc;
			}
			if(mode==3){
				int grp = setpgid(rc, -10);
			}
			if(rc<0){
				fprintf(stderr, "fork failed\n");
				return;
			}
			else if(rc==0){
				//printf("hello i am child");
				execvp(sep_cmds[0], sep_cmds);
				//printf("Shell:Incorrect command\n");
				return;
			}
			else{
				if(mode==1){
					bg_pid[bg_count] = rc;
					bg_count++;
					setpgid(rc, 0);
				}
				if(mode!=1 && mode!=3){
					int wc = waitpid(rc, &status, 0);
					//printf("Child pid = %d\n", rc);
					return;
				}
				//printf("hello i am parent");
			}
		}
		return;
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;
	int i;
	int iter = 0;
	int status;
	for(int j=0;j<128;j++){
		bg_pid[j] = INT_MAX;
	}

	while(1) {			
		/* BEGIN: TAKING INPUT */
		signal(SIGTERM, func);
		signal(SIGINT, func_cc);
		if(flag_exit==1)
			break;
		flag_exit = 0;
		flag = 0;
		int wc_bg = waitpid( -1, &status, WNOHANG );
		if(wc_bg!=-1 && wc_bg!=0){
			//printf("%d", wc_bg);
			printf("Shell: Background process finished\n");
		}
		bzero(line, sizeof(line));
		sleep(0.001);
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();
		//int wc_bg = wait(NULL);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
    
    int mode = 0;
    for(i=0;tokens[i]!=NULL;i++){
    	if(strcmp(tokens[i], "&")==0){
    		mode = 1;
    		break;
    	}
    	if(strcmp(tokens[i], "&&")==0){
    		mode = 2;
    		break;
    	}
    	if(strcmp(tokens[i], "&&&")==0){
    		mode = 3;
    		break;
    	}
    }
       //do whatever you want with the commands, here we just print them

    if(mode==0){
    	shell_fn(tokens, mode);
    	if(flag==1)
    		continue;
    }
    else if(mode==1){
    	i=0;
    	int last_idx;
				while(1){
					if(tokens[i]==NULL){
						last_idx = i-1;
						break;
					}
					i++;
				}
				tokens[last_idx] = NULL;
				shell_fn(tokens, mode);
				continue;

    }
    else if(mode==2){

			int start = 0;
			i=0;

			//i=0;
			char **sep_tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
			while(1){
				if(tokens[i]!=NULL){

					if(strcmp(tokens[i], "&&")!=0){
						//printf(tokens[i]);
						sep_tokens[i-start] = tokens[i];
					}
					else{
						shell_fn(sep_tokens, mode);
						for(int j=0;sep_tokens[j]!=NULL;j++){
							sep_tokens[j] = NULL;
						//sep_tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
						}
						start = i+1;
					}
				}
				else{
					shell_fn(sep_tokens, mode);
					for(int j=0;sep_tokens[j]!=NULL;j++)
							sep_tokens[j] = NULL;
					break;
					
					}
					i++;
					if(flag==1)
    				break;
				}
			for(int j=0;sep_tokens[j]!=NULL;j++)
							free(sep_tokens[j]);
			free(sep_tokens);
		}

		else if(mode==3){

			int start = 0;
			i=0;

			char **sep_tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
			i=0;
			int num_cmd = 0;
			while(1){
				if(tokens[i]!=NULL){

					if(strcmp(tokens[i], "&&&")!=0){
						//printf(tokens[i]);
						sep_tokens[i-start] = tokens[i];
					}
					else{
						shell_fn(sep_tokens, mode);
						num_cmd++;
						for(int j=0;sep_tokens[j]!=NULL;j++){
							sep_tokens[j] = NULL;
						//sep_tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
						}
						start = i+1;
					}
				}
				else{
					shell_fn(sep_tokens, mode);
					num_cmd++;
					for(int j=0;sep_tokens[j]!=NULL;j++)
							sep_tokens[j] = NULL;
					break;
					}
					i++;
					if(flag==1)
    				break;
				}
			for(int j=0;sep_tokens[j]!=NULL;j++)
							free(sep_tokens[j]);
			free(sep_tokens);
			for(int j=0;j<num_cmd;j++){
				int wc = wait(NULL);
			}
		}
		iter++;
	}


		
       
		// Freeing the allocated memory	
		for(int j=0;tokens[j]!=NULL;j++){
			free(tokens[j]);
		}
		free(tokens);
		//free(sep_tokens);

	return 0;
}
