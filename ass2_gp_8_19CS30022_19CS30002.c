/*
    OS LAB
    Assignment : 02 use of syscalls
    Group : 08
*/

/*
multiwatch => cmd1 && cmd2 && cmd3
every command has pipes

*/

//librabies
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fcntl.h"
#include "unistd.h"
#include <signal.h>
#include <ncurses.h>
#include <sys/wait.h>
#include <curses.h>
#include <dirent.h>

#define MAXLINE 128

//function declare
int takecommand(char *);
char **splitcommands(char *, int *);
char **splitcommand(char *, int *);
void printerror(const char *error);
int runcommand(char **, int);
int execute(char **, int, int, int);
void handle_sigint(int);
void collecthistory();
void printhistory();
int autocomplete(char *);
void writehistory();
int multiwatch(char *);
char *substring(char *, int, int);
int executemultipipes(char **, int);
int ctrlr(char *);
char ** searchfunction(char *, int *);
char * getrunning(char *);
void revstr(char *);
int notinhistory(char ** ,char *, int);

// struct for storing history commands
struct commandstore
{
    int n;
    char **commands;
    int pos;
} his;

// main function
int main()
{

    char *command;
    int commandscount = 0;
    char **commands;
    int quit = 0;
    char **args;
    int tokenscount;

    signal(SIGINT, handle_sigint);

    collecthistory();
    // autocomplete(command);

    // exit if "exit" command is entered
    while (quit != 1)
    {
        // input command line memory
        command = (char *)malloc(sizeof(char) * MAXLINE);
        printf("shell >> ");

        // take command in shell
        takecommand(command);
        nocbreak();
        if (strlen(command) > 0)
        {
            // split line into pipes
            commands = splitcommands(command, &commandscount);

            // got commands
            // only one pipe
            if (commandscount == 1)
            {

                // exit
                if (strcmp(commands[0], "exit") == 0)
                {
                    writehistory();
                    quit = 1;
                    break;
                }
                else if (strlen(commands[0]) > 9 && strcmp(substring(commands[0], 0, 10), "multiWatch") == 0)
                {
                    quit = multiwatch(commands[0]);
                }
                else
                {
                    // split command into words

                    args = splitcommand(commands[0], &tokenscount);

                    //
                    if (args != NULL && tokenscount > 0)
                    {
                        // execute command
                        quit = execute(args, tokenscount, 0, 1);
                    }
                    else
                    {
                        printerror("unknow error : one command found but not able to run command.\n");
                    }
                }
            }
            // if multiple pipes
            else if (commandscount > 1)
            {
                quit = executemultipipes(commands, commandscount);
            }

            if (quit != 1)
            {
                his.pos += 1;
                his.commands[his.pos] = (char *)malloc(sizeof(char) * MAXLINE);
                his.commands[his.pos] = command;
            //     // if last command executed successfully
            //     // add to histoy
            //     waitpid(getpid(), NULL, 0);
            //     char *templine = (char *)malloc(sizeof(char) * sizeof(command));
            //     his.pos += 1;
            //     his.commands[his.pos] = templine;
            //     if (templine)
            //         //free(templine);
            }

            if (command)
                //free(command);
                if (args)
                    //free(args);
                    fflush(stdin);
            fflush(stdout);
        }
    }

    return 0;
}
int executemultipipes(char **commands, int commandscount)
{
    int quit = 0;

    int i, input_fd, FD[2];
    // FD[0] to input file for pipes - 1st and FD[1] is output file for pipes - last
    char **args;
    int argc;

    for (i = 0; i < commandscount - 1; i++)
    {
        if (pipe(FD) == -1)
        {
            printerror("ERROR: Error in pip(FD)\n");
            break;
        }
        else
        {
            // split one pipe into arguments
            args = splitcommand(commands[i], &argc);
            if (argc > 0)
            {
                // execute one pipe
                quit = execute(args, argc, input_fd, FD[1]);
            }

            close(FD[1]);

            input_fd = FD[0];
            //seting FD[0] input file for rest of the pipes
        }
    }
    // execute last command

    args = splitcommand(commands[commandscount - 1], &argc);
    if (argc > 0)
    {
        quit = execute(args, argc, input_fd, 1);
    }

    return quit;
}

// read history command from history.txt file
void collecthistory()
{
    his.n = 1000;
    his.commands = (char **)malloc(sizeof(char *) * his.n);
    his.pos = -1;
    FILE *file = fopen("history.txt", "r");
    char *command;
    int stop = 1;

    //store all commands in his struct
    while (stop == 1)
    {
        command = (char *)malloc(sizeof(char) * 1024);
        if (!fgets(command, 1024, file))
            stop = 0;
        else
        {
            command[strlen(command) - 1] = '\0';
            his.pos += 1;
            his.commands[his.pos] = (char *)malloc(sizeof(char) * 1024);
            his.commands[his.pos] = command;
        }
    }
    fclose(file);
}

// testing function to print history
void printhistory()
{
    int i;
    for (i = 0; i <= his.pos; i++)
    {
        printf("%d %s\n", i, his.commands[i]);
    }
}

// write back history
void writehistory()
{
    int i;
    FILE *f = fopen("history.txt", "w");
    if (f == NULL)
    {
        printerror("ERROR : error while storing history\n");
        return;
    }
    for (i = 0; i <= his.pos; i++)
    {
        fprintf(f, "%s\n", his.commands[i]);
    }
    fclose(f);
    // //free(his.commands);
}

// function to take command char by char
int takecommand(char *command)
{
    char c = 'a';
    int i = 0;
    int size = MAXLINE;
    // initscr();
	
    // endwin();
    while (1)
    {
        // system("/bin/stty raw");
        // cbreak();
   
        c = getchar();
        // system("/bin/stty cooked");
        // printf("%c",c);
        if(c == 10){
            //return ctrlr(command);
            
        }
        else if (c == '\t')
        {
            // system("/bin/stty cooked");
           
            return autocomplete(command);
        }
        else
        {
            if (c != '\n')
            {
                command[i] = c;
                i++;
                if (i >= MAXLINE)
                {
                    size += MAXLINE;
                    // if command line pass the memory size realloc
                    command = (char *)realloc(command, size);
                }
            }
            else
            {
              
                command[i] = '\0';
                system("/bin/stty cooked");
                return i;
            }
        }
    }

    // number of chars
    system("/bin/stty cooked");
   
    return i;
}

// function to split pipes
char **splitcommands(char *command, int *commandscount)
{
    int size = MAXLINE;
    char sperator = '|';
    char **commands = (char **)malloc(sizeof(char *) * size);

    int i;
    for (i = 0; i < size; i++)
    {
        commands[i] = (char *)malloc(sizeof(char) * size);
    }
    int p1 = 0, p2 = 0;
    int len = strlen(command);
    for (i = 0; i < len; i++)
    {
        // printf("%c", command[i]);
        if (command[i] == '|')
        {
            p1++;
            p2 = 0;
        }
        else
        {
            if (p2 == 0 && command[i] == ' ')
            {
            }
            else
            {
                commands[p1][p2] = command[i];
                p2++;
            }
        }
    }
    if (p2 == 0 && p1 == 0)
    {
        *commandscount = 0;
    }
    else
    {
        *commandscount = p1 + 1;
    }
    return commands;
}

// function to get words from one pipe(command)
char **splitcommand(char *command, int *argc)
{
    char **argv;
    if (command == NULL)
        return argv;

    int len = strlen(command);

    int size = MAXLINE;

    argv = (char **)malloc(sizeof(char *) * size);
    int i;
    for (i = 0; i < size; i++)
    {
        argv[i] = (char *)malloc(sizeof(char) * size);
    }
    int p = 0, q = 0;

    for (i = 0; i < len; i++)
    {
        if (command[i] == ' ')
        {
            p++;
            q = 0;
        }
        else
        {
            argv[p][q] = command[i];
            q++;
        }
    }

    // argv array of arguments and argc count of argumetns
    *argc = p + 1;
    return argv;
}

//print error function
void printerror(const char *error)
{
    printf("%s", error);
}

//testing funciton to monitor execution
int runcommand(char **args, int argc)
{
    printf("run command:\n");
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s ", args[i]);
    }
    printf("\n");
    if (args[0] == NULL)
        return 1;
    return execute(args, argc, 0, 1);
}

//execute one command
int execute(char **argv, int argc, int inputfd, int outputfd)
{
    pid_t pid;
    int quit;

    char *argument;
    pid = fork();
    if (pid == 0)
    {

        //adjust input and output file descriptors

        if (inputfd != 0)
        {
            dup2(inputfd, 0);
            close(inputfd);
        }

        if (outputfd != 1)
        {
            dup2(outputfd, 1);
            close(outputfd);
        }

        int maincommand = 0, firstsymbol = 0, i;

        for (i = 0; i < argc; i++)
        {
            argument = argv[i];
            if (firstsymbol == 0)
            {
                if (strcmp(argument, "&") == 0 || strcmp(argument, "<") == 0 || strcmp(argument, ">") == 0)
                {
                    maincommand = i;
                    firstsymbol = 1;
                }
            }

            // redirection of input output streams
            if (strcmp(argument, ">") == 0)
            {
                int redirectoutputfd = open(argv[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                dup2(redirectoutputfd, STDOUT_FILENO);
            }

            if (strcmp(argument, "<") == 0)
            {
                int redirectinputfd = open(argv[i + 1], O_RDONLY);
                dup2(redirectinputfd, STDIN_FILENO);
            }
        }

        if (firstsymbol == 0)
        {
            maincommand = argc;
        }

        char **args = (char **)malloc(sizeof(char *) * (maincommand + 1));
        for (i = 0; i < maincommand; i++)
            args[i] = argv[i];
        args[i] = NULL;

        // run proccess
        if (execvp(args[0], args) == -1)
        {
            printerror("Error: execvp error");
            return 1; // fail
        }
    }
    else if (pid < 0)
    {
        printerror("Error: while fork!\n");
        return 1; // fail
    }
    else
    {
    }
    if (strcmp(argv[argc - 1], "&") != 0)
        waitpid(pid, NULL, 0);
    //if command ends with & then don't wait
    return 0;
}

// handle ctrl + c
void handle_sigint(int sig)
{

    printerror("\nCTRL+C : Fore stop  \n");
    // kill(getpid(), sig);
    // waitpid(getpid(), NULL, 0);
}

// print suggestions
int autocomplete(char *command)
{
    // char ** ideal = (char **)malloc(sizeof(char *)*2);
    // ideal[0] = (char *)malloc(sizeof(char)*MAXLINE);
    // ideal[1] = (char *)malloc(sizeof(char)*MAXLINE);
    // ideal[0]="ls";
    // ideal[1]="&";
    // execute(ideal, 2, 0, 1);
    char * incompletename = getrunning(command);
    DIR *d;
    struct dirent *dir;
    char **names = (char **)malloc(sizeof(char *)*100);
    int i =0;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
        if(strstr(dir->d_name, incompletename)){
            command[strlen(command)-strlen(incompletename)]='\0';
            
            printf("%d).%s%s\n",i,command,dir->d_name);
            i++;
        }
        }
        closedir(d);
    }
    // free(ideal);
    return 0;
}
char * getrunning(char * command){
    char * ans = (char *)malloc(sizeof(char )*MAXLINE);

    int i=strlen(command)-1, j=0;
    while(i>-1 && command[i]!=' '){
        ans[j]=command[i];
        i--;
        j++;
    }
    revstr(ans);
    return ans;
}
void revstr(char *str1)  
{  
    // declare variable  
    int i, len, temp;  
    len = strlen(str1); // use strlen() to get the length of str string  
      
    // use for loop to iterate the string   
    for (i = 0; i < len/2; i++)  
    {  
        // temp variable use to temporary hold the string  
        temp = str1[i];  
        str1[i] = str1[len - i - 1];  
        str1[len - i - 1] = temp;  
    }  
}  
char *substring(char *string, int position, int length)
{
    char *p;
    int c;

    p = malloc(length + 1);

    if (p == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }

    for (c = 0; c < length; c++)
    {
        p[c] = string[position + c];
    }

    p[c] = '\0';
    return p;
}

int multiwatch(char *command)
{
    int quit = 0;
    char **subcmds;
    char *remaincommand = substring(command, 11, strlen(command) - 11);
    int i, n = 0, p = -1, len = strlen(remaincommand);
    subcmds = (char **)malloc(sizeof(char *) * len);

    for (i = 0; i < len; i++)
    {
        if (p == -1)
        {
            subcmds[n] = (char *)malloc(sizeof(char) * MAXLINE);
            p = 0;
        }
        if (remaincommand[i] == '&' && i + 1 < len && remaincommand[i + 1] == '&')
        {
            n++;
            p = -1;
            i++;
        }
        else
        {
            if (p != 0 || (p == 0 && remaincommand[i] != ' '))
            {
                subcmds[n][p] = remaincommand[i];
                p++;
            }
        }
    }

    //subcmds has n commands each command has some pipes

    char **commands;
    int commandc;
    char **args;
    int argc;

    //uptime execution
    char **ideal = (char **)malloc(sizeof(char *) * 2);
    ideal[0] = (char *)malloc(sizeof(char) * MAXLINE);
    ideal[1] = (char *)malloc(sizeof(char) * MAXLINE);
    ideal[0] = "uptime";
    ideal[1]="&";

    int j;

    for (i = 0; i <= n; i++)
    {
        commands = splitcommands(subcmds[i], &commandc);
        //make every command background
        for (j = 0; j <= commandc; j++)
        {
            strcat(commands[i], " &");
        }
        // split pipes
        if (commandc == 1)
        {
            //execute one command

            args = splitcommand(commands[0], &argc);
            if (argc > 0)
            {
                //execute more pipes
                quit = execute(args, argc, 0, 1);
            }
        }
        else if (commandc > 1)
        {
            quit = executemultipipes(commands, commandc);
        }
        else
        {
            printf("uncovered ERROR\n");
        }
        execute(ideal, 2, 0, 1);
    }
    // int end;
    // for (i = 0; i <= n; i++)
    // {
    //     strcat(subcmds[i], " &");
    // }

    return quit;
}

int ctrlr(char * command){
    char * searchline = (char *)malloc(sizeof(char )*MAXLINE);
    printf("Enter to search : ");
    scanf("%[^\n]",searchline);
    int n;
    char ** ans = searchfunction(searchline, &n);
    int i;
    for(i=0; i<=n; i++){
        printf("%s\n",ans[i]);
    }
    return 0;
}

char ** searchfunction(char * search, int* n){
    char ** ans ;
    *n=-1;
    ans = (char **)malloc(sizeof(char *)*(his.pos+1));
    int i;
    int is;
    
    for(i=his.pos; i>=0; i--){
        if(his.commands[i] && strstr(his.commands[i],search) && notinhistory(ans, his.commands[i], *n)==-1){
            *n+=1;
            ans[*n]=his.commands[i];
        }
    }
    return ans;
}
int notinhistory(char ** history, char * command, int size){
    int i;
    for(i=0; i<size; i++){
        if(strcmp(history[i],command)==0)return i;
    }

    return -1;
}

//gcc -o shell ass2_gp_8_19CS30022_19CS30002.c -lncurses
