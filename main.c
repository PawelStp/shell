#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
volatile int flag_back=0;

void handler(int signum)
{

}
int cd(char **args)
{
    if(args[1]==NULL)
    {
        fprintf(stderr, "Nie podano folderu docelowego\n");
    }
    else
    {
        if(chdir(args[1])!=0)
        {
            perror("Brak podanego folderu");
        }
    }
    return 1;
}


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
	token = tokens[position-1];
	if(token[strlen(token)-1]=='&')
	{
        flag_back=1;
        token[strlen(token)-1]=NULL;
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
		else if(flag_back == 0)
		{
			do
			{
				wpid=waitpid(pid,&status,WUNTRACED);
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
	}
}

void saveHistory(char *arg, char *cmd)
{
    int r=open(arg,O_RDONLY);
    int s,i, max=0;
    int j=0;
    char c[1];
    char history[20][100];
    if(r != -1)
    {
        i = 0;
        while(read(r, c, 1) && i < 19)
        {
            history[i][j]=c[0];
            if(c[0] == '\n')
            {
                history[i][j]=NULL;
                j=0;
                i++;
            }
            else
            {
                j++;
            }
        }
        max=i;
        close(r);
        s = open(arg, O_WRONLY | O_CREAT| O_TRUNC, 0666);
        write(s, cmd, strlen(cmd));
        write(s, "\n",1);
        j=0; i=0;
        while(1)
        {

            write(s, history[i], strlen(history[i]));
            write(s, "\n",1);
            i++;
            if(i==max)break;
        }
        close(s);
    }
    else
    {
        s = open(arg, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        write(s,cmd,strlen(cmd));
        write(s, "\n",1);
        close(s);
    }

}

int main (int argc, char* argv[])
{
    int a;
	while(1)
	{
		char *line=readArguments();
		char **args=SepLine(line);

		if(strcmp(args[0],"cd")==0)
		{
            saveHistory("h.txt", args[0]);
            cd(args);
		}
		else
		{
		    saveHistory("h.txt", args[0]);
            a = launch(args);
		}
        if(flag_back==1)
		{
            flag_back=0;
		}
    }


return 0;
}
