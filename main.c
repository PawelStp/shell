#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stddef.h>
#include <locale.h>
#include <ctype.h>
volatile int flag_back=0;
volatile int flag_forward=0;
volatile int script_flag =0;
volatile char forwarding[20];
typedef struct PIPE{
    char *line;
    char **args;
};
volatile int counterPipes=0;
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

struct PIPE* SepPipes(char *line)
{
    counterPipes=0;
    int bufforSize=64;
    int position=0;
    struct PIPE* tokens= malloc(bufforSize * sizeof(struct PIPE));
    struct PIPE token;
    token.line = strtok(line,"|\n");
    while(token.line!=NULL)
	{

            tokens[position] = token;
            position++;
            counterPipes++;
            if(position >= bufforSize)
            {

                bufforSize += 64;
                tokens = realloc(tokens, bufforSize * sizeof(struct PIPE));
            }

		token.line = strtok(NULL,"|\n");

	}
	return tokens;
}

char** SepLine(char *line)
{
	int bufforSize = 64, position = 0;
	char** tokens = malloc(bufforSize * sizeof(char*));
	char *token;

	token = strtok(line, " \n");
	while(token!=NULL)
	{
        if(flag_forward==1)
        {
            strcpy(forwarding,token);
        }
        else if(strcmp(token,">>")==0)
        {
            flag_forward=1;
        }
        else
        {
            tokens[position] = token;
            position++;
            if(position >= bufforSize)
            {
                bufforSize += 64;
                tokens = realloc(tokens, bufforSize * sizeof(char*));
            }
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
void handler(int signa)
{

    char c[1];
    int r=open("h.txt",O_RDONLY);
    if(r != -1)
    {
        while(read(r, c, 1))
        {
            fprintf(stderr ,"%c", c[0]);
        }
    }
    fprintf(stderr,"shell->> ");
}
void scripts(char *file)
{
    int c;
    int i;
    FILE *fp;
    fp=fopen(file,"r");
    int bufforSize=64;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int j=0;
    while ((read = getline(&line, &len, fp)) != -1) {
       if(j>0 && read>0)
       {
            line[read-2]='\n';
            struct PIPE *pipe=SepPipes(line);
            for(i=0;i<counterPipes;i++)
            {
                pipe[i].args=SepLine(pipe[i].line);
            }
            int a = launch(pipe);
        }
        j++;
    }

    if (line)
        free(line);

    fclose(fp);
}
int launch(struct PIPE* pipes)
{
    pid_t wpid;
    int status;
	int tmpIn=dup(0);
	int tmpOut=dup(1);
	int fdin;
    fdin=dup(tmpIn);
	pid_t ret;
	int fdOut;
	int i;
	for(i=0;i<counterPipes;i++)
	{
        dup2(fdin,0);
        close(fdin);
        if(i==counterPipes-1)
        {
            if(flag_forward==1)
            {
                fdOut=open(forwarding,O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }
            else
            {
                fdOut=dup(tmpOut);
            }
        }
        else
        {
            int fdPipe[2];
            pipe(fdPipe);
            fdOut=fdPipe[1];
            fdin=fdPipe[0];
        }
        dup2(fdOut,1);
        close(fdOut);
        ret=fork();
        if(ret==0)
        {

            if((execvp(pipes[i].args[0],pipes[i].args))==-1)
            {
                fprintf(stderr,"Brak podanego programu\n");
            }

        }else
        {
            if(ret<0)
            {
                perror("Wystapil blad!!!");
            }
        }
    }
	dup2(tmpIn,0);
	dup2(tmpOut,1);
	close(tmpIn);
	close(tmpOut);
    if(flag_back == 0 && ret > 0)
		{
			do
			{
				wpid=waitpid(ret,&status,WUNTRACED);
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}

}
int main (int argc, char* argv[])
{
    int a;
    int i;
    fprintf(stderr,"shell->> ");
    int bufSize=64;
    if(argc==2)script_flag=1;
	while(1)
	{

        signal(SIGQUIT, handler);
        int j=0;
        int k=0;

        if(script_flag==1)
        {
            counterPipes=1;
             scripts(argv[1]);
             script_flag=0;
        }
        char *line=readArguments();
        struct PIPE *pipe=SepPipes(line);
        for(i=0;i<counterPipes;i++)
        {
            pipe[i].args=SepLine(pipe[i].line);
        }
        if(strcmp(pipe[0].args[0],"exit")==0)
        {
            exit(0);
        }
		if(strcmp(pipe[0].args[0],"cd")==0)
		{
            saveHistory("h.txt", pipe[0].args[0]);
            cd(pipe[0].args);
            fprintf(stderr,"shell->> ");
		}
		else
		{
		    saveHistory("h.txt", pipe[0].args[0]);
            int a = launch(pipe);
            fprintf(stderr,"shell->> ");
		}

        if(flag_back==1)
		{
            flag_back=0;
		}
		if(flag_forward==1)
		{
            flag_forward=0;
		}
    }
    return 0;
}
