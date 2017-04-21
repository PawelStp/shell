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
int launch(char** args)
{
	pid_t pid, wpid;
	int status;
	pid=fork();
	if(pid==0)
	{
        if(flag_forward==1)
        {
            int out = open(forwarding, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(out, 1);
            close(out);
        }
		if(execvp(args[0],args)==-1)
		{
			perror("Brak podanego programu!!!");
		}
		//printf("asdasdasd");
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
void catch_stop ();
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
    FILE *fp;
    fp=fopen(file,"r");
    int bufforSize=64;
    /*char** lines = malloc(bufforSize * sizeof(char*));
    int i=0;
    int j=0;
    int k=0;*/
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    /*while(1)
    {
        c = fgetc(fp);
        if( feof(fp) )
        {
            break;
        }
        lines[i]=(char)c;
        //fprintf(stderr,"%c",lines[i]);
        if(lines[i]=='\n')
        {
            if(j>0 && i>0)
            {
                char**args=SepLine(lines);
                fprintf(stderr,"%s", args[0]);
                int a=launch(args);
            break;
            }
            j++;
            i=0;
            for(k=0; k<bufforSize; k++) lines[i]=0;
        }
        i++;


    }
    fclose(fp);*/
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int j=0;
    while ((read = getline(&line, &len, fp)) != -1) {
       if(j>0 && read>0)
       {
            line[read-2]='\n';
            line[read-1]=0;
            char **args=SepLine(line);
            int a = launch(args);
       }
        j++;
    }

    if (line)
        free(line);

    fclose(fp);
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
             scripts(argv[1]);
             fprintf(stderr,"shell->> ");
             script_flag=0;
        }
        char *line=readArguments();
        char** args=SepLine(line);
		if(strcmp(args[0],"cd")==0)
		{
            saveHistory("h.txt", args[0]);
            cd(args);
            fprintf(stderr,"shell->> ");
		}
		else
		{
		    saveHistory("h.txt", args[0]);
            a = launch(args);
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
