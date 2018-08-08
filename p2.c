/**************************************************************
 2017 ConnorGuy <connor_guy@icloud.com>
 Carroll | CS 570
 Due 12/6/17 @ 11pm

C program called p2, which acts as a simple command line 
interpreter for our UNIX system.  

p2 can handle redirects in and out. Can do single piping to 
new command. Commands can handle multiple args. Has built in 
functionality for [ls-F] and [cd] commands that do not fork
a new proccess.

In most cases if an error is encountered p2 will output an 
and give user a new prompt.

Some flag types have been defined and used without, these are 
either defined in p2.h or getword.h.
**************************************************************/

#include "p2.h"

//GLOBAL VARS
static int child = 0; /* whether it is a child process relative to main() */
static char cmdArray[15][MAXITEM][STORAGE];
static char *command[STORAGE];
static int numOfCmds;
static int redirectInSet;
static char redirectIn[STORAGE];
static int redirectOutSet;
static char redirectOut[STORAGE];
static int forceWrite;
static int amperSet;
static int input_fd;
static int output_fd;
static int saved_stdout;
static int saved_stdin;

int main()
{
    setpgrp();
    while (1)
    {
        // Clean up for next loop
        if (redirectOutSet == TRUE) // reset stdout if needed
            dup2(saved_stdout, 1);
        else
            saved_stdout = dup(1);
        if (redirectInSet == TRUE) // reset stdin if needed
            dup2(saved_stdin, 0);
        else
            saved_stdin = dup(0);
        memset(cmdArray, 0, 15 * MAXITEM * STORAGE);
        redirectInSet = FALSE;
        redirectOutSet = FALSE;
        amperSet = FALSE;
        forceWrite = FALSE;
        input_fd = 0;
        output_fd = 0;
        redirectOut[0] = '\0';
        redirectIn[0] = '\0';
        ////////////////////////

        (void)printf("p2: ");
        ////////////
        int rt = 0;
        int numCmds = 0;
        int rtval = parse();
        // Get commands from user
        if (rtval == -1)
        {
            break;
        }
        if (rtval == -2)
        {
            perror("Redirect Ambiguous");
            continue;
        }
        setRedirection();

        fflush(stderr);
        fflush(stdout);
        int kidpid = fork();
        switch (kidpid)
        {
        case -1:
            perror("fork");
        case 0: // Child
            // Set Redirects
            if (redirectInSet == TRUE)
                executePipeline(0, input_fd);
            else
                executePipeline(0, STDIN_FILENO);
        default: // Parent
            if (amperSet == FALSE)
            {
                wait(&kidpid);
            }
            else
                printf("echo [%d]\n", kidpid);
        }
    }
    printf("p2 terminated.\n");
    killpg(getpgrp(), SIGTERM); // Terminate any children
    exit(0);
}

/*****************************************************************************
Catches sigterm in the case that it gets a signal.
*****************************************************************************/
void myhandler(int signum)
{
    // Do Nothing
}

/*****************************************************************************
Calls the getword function and checks input and stores each words type in 
a flag array.
*****************************************************************************/
int parse()
{
    char w[STORAGE];
    int wordCount = 0;
    numOfCmds = 0;
    int rt = 0;
    int flag = 0;
    while (1)
    {
        rt = getword(w); // Return is the type of word returned
        // if (w[0] == '>' && w[1] == '!')
        //     printf("patchy\n");
        if (setFlags(rt) == -2)
            flag = -2; // Ambiguous redirect
        if (rt == PIPELINE)
        {
            numOfCmds++;
            wordCount = 0;
        }
        else if (rt == 0)
        {
            strcpy(cmdArray[numOfCmds][wordCount++], w);
        }
        if (rt == NEWLINE || rt == EOFILE || rt == AMPERSAND || rt == SEMICOL)
            break;
    }
    cmdArray[numOfCmds][wordCount + 1][0] = '\0';
    if (wordCount == 0 && rt == EOFILE && redirectInSet == FALSE)
        return -1;
    return flag;
}

/*****************************************************************************
For checking whether any special action needs to take place with input.
*****************************************************************************/
int setFlags(int rt)
{
    if (rt == AMPERSAND)
    {
        amperSet = TRUE;
        return 2;
    }
    // If its a redirect, get the file name and place in storage
    if (rt == GREATER_THAN || rt == LESSTHAN)
    {
        char tmpRedirectLocation[STORAGE];
        getword(tmpRedirectLocation);
        if ((rt == GREATER_THAN || rt == GREATER_EXCL) && redirectOutSet == FALSE)
        {
            redirectOutSet = TRUE;
            strcpy(redirectOut, tmpRedirectLocation);
            if (rt == GREATER_EXCL)
            {
                forceWrite = TRUE;
            }
            return 0;
        }
        else if ((rt == GREATER_THAN || rt == GREATER_EXCL) && redirectOutSet == TRUE)
        {
            return -2;
        }
        if (rt == LESSTHAN && redirectInSet == FALSE)
        {
            redirectInSet = TRUE;
            strcpy(redirectIn, tmpRedirectLocation);
            return 0;
        }
        else if (rt == LESSTHAN && redirectInSet == TRUE)
        {
            return -2;
        }
    }
    return 1;
}

/*****************************************************************************
Create pipes to be used in executePipeline.
*****************************************************************************/
static void redirect(int oldfd, int newfd)
{
    if (oldfd != newfd)
    {
        if (dup2(oldfd, newfd) != -1)
            Close(oldfd); /* successfully redirected */
        else
            perror("dup2");
    }
}

/*****************************************************************************
Recursive execution command that can do multiple or single commands.
*****************************************************************************/
static void executePipeline(int pos, int in_fd)
{
    getCommand(pos);
    char *const *theCmd = command;

    // The last command of the cmdArray
    if (cmdArray[pos + 1][0][0] == '\0')
    {
        redirect(in_fd, STDIN_FILENO); /* read from in_fd, write to STDOUT */

        // Do any built in functions
        if (checkForBuiltInFuntions(&pos) == 1)
        {
            exit(0); // kill the process
        }
        else
            execvp(theCmd[0], theCmd);

        // Neither of the commands exicuted and were in trouble
        perror("execvp last");
    }
    else
    {
        int fd[2]; /* output pipe */
        if (pipe(fd) == -1)
            perror("pipe");
        switch (fork())
        {
        case -1:
            perror("fork");
        case 0: /* child: execute current command `cmds[pos]` */
            child = 1;
            Close(fd[0]);                   /* unused */
            redirect(in_fd, STDIN_FILENO);  /* read from in_fd */
            redirect(fd[1], STDOUT_FILENO); /* write to fd[1] */
            // Do any built in functions
            if (checkForBuiltInFuntions(&pos) == 1)
            {
                pos++;   // Need to increment the command
                exit(0); // kill the process
            }
            else
                execvp(theCmd[0], theCmd);
            perror("exevp failed to execute");
        default:                             /* parent: execute the rest of the commands */
            Close(fd[1]);                    /* unused */
            Close(in_fd);                    /* unused */
            executePipeline(pos + 1, fd[0]); /* execute the rest */
        }
    }
}

/*****************************************************************************
getCommand will put the command at pos from cmdArray into the command pointer 
that execvp can execute.
*****************************************************************************/
void getCommand(int pos)
{
    int x = 0;
    int y = 0;

    while (cmdArray[pos][x][0] != '\0')
    {
        command[x] = cmdArray[pos][x];
        x++;
    }
    if (cmdArray[pos][0][0] == '\0')
    {
        command[0] = NULL;
        return;
    }
    if (cmdArray[pos][x][y] != '\0')
        command[x + 1] = NULL;
}

/*****************************************************************************
Check if we are doing any ls-F [return 1] or cd [return 2] commands
*****************************************************************************/
int checkForBuiltInFuntions(int *pos)
{
    char lsfCommand[] = "ls-F";
    char cdCommand[] = "cd";
    char execCommands[] = "exec";
    // strcmp returns a 0 if true -> !0 is what is being checked for.
    if (!strcmp(lsfCommand, command[0]))
    {
        (*pos)++;
        printf("ls command\n");
        return 1;
    }
    if (!strcmp(cdCommand, command[0]))
    {
        (*pos)++;
        printf("cd command\n");
        return 1;
    }
    if (!strcmp(execCommands, command[0]))
    {
        (*pos)++;
        execCommand();
        return 1;
    }
    return 0;
}

/*****************************************************************************
Built in functionality that runs the cd command without forking a child.
*****************************************************************************/
void cdCommand(char **commandArgs, int commandArgsCount)
{
    //TODO
}

/*****************************************************************************
Built in functionality that runs the ls-F command without forking a child.
*****************************************************************************/
void lsCommand(char **commandArgs, int commandArgsCount)
{
    //TODO
}

/*****************************************************************************
Built in functionality that runs the cd command without forking a child.
*****************************************************************************/
void execCommand()
{
    execvp(command[1], command + 1);
}

/*****************************************************************************
 Set up redirects using dup2.
*****************************************************************************/
void setRedirection()
{
    if (redirectInSet == TRUE)
    {
        int flags = O_RDONLY;
        input_fd = open(redirectIn, flags, 0755);
        if (input_fd < 0)
        {
            perror("open failed.");
        }
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            perror("dup2 failed.");
        }
    }
    if (redirectOutSet == TRUE)
    {
        if (forceWrite == FALSE)
        {
            output_fd = open(redirectOut, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);
        }
        else
        {
            output_fd = open(redirectOut, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        }
        if (output_fd < 0)
        {
            perror("open failed.");
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2 failed.");
        }
    }
}

/*****************************************************************************
 Close any open redirects.
*****************************************************************************/
void closeRedirection()
{
    if (redirectOutSet == TRUE)
    {
        if (close(output_fd) != 0)
        {
            perror("close failed.");
        }
    }
    if (redirectInSet == TRUE)
    {
        if (close(input_fd) != 0)
        {
            perror("close failed.");
        }
    }
}