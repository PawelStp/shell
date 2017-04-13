#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
char* readArguments()
{
	char* line=NULL;
	ssize_t bufforSize=0;
	getline(&line, &bufforSize, stdin);
	return line;
}
char** SepLine(char *line)
{
	int bufforSize = 64, position = 0;
	char** tokens = malloc(bufforSize * sizeof(char*));
	char *token;
	
	token = strtok(line, " \n");
	while(token!=NULL)
	{
		tokens[position] = token;
		position++;
		if(position >= bufforSize)
		{
			bufforSize += 64;
			tokens = realloc(tokens, bufforSize * sizeof(char*));
		}
		token = strtok(NULL," \n");		

	}	
	tokens[position] = NULL;
	return tokens;

}
int launch(char** args)
{
	pid_t pid, wpid;
	int status;
	pid=fork();
	if(pid==0)
	{
		if(execvp(args[0],args)==-1)
		{
			perror("Brak podanego programu!!!");
		}
	exit(EXIT_FAILURE);
	}
	else
	{	
		if(pid<0)
		{
			perror("Brak podanego programu!!!");			
		}
		else
		{
			do
			{
				wpid=waitpid(pid,&status,WUNTRACED);
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}
} 


int main (int argc, char* argv[])
{
	//fprintf(stderr,"%s",argv[0]);
	while(1)
	{
		char *line=readArguments();
		char **args=SepLine(line);
		int a = launch(args);
	}


return 0;
}
