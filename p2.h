#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "getword.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>

#define STORAGE 255
#define MAXITEM 100
#define TRUE 1
#define FALSE 0
#define STDOUT_FILENO 1

// Parsing variables
#define REGULAR_CHAR 0 //[a-z, A-Z, 0-9]
#define TAB_SPACE 2    //['\t', ' ']
#define BACKSLASH 3    //['\\']
#define SINGLE_QUOTE 4 //['\']
#define NEWLINE 5      //['\n']
#define GREATER_EXCL 6 //[>!]
#define GREATER_THAN 7 //[>]
#define LESSTHAN 8     //[<]
#define PIPELINE 9     //[|]
#define AMPERSAND 10   //[&]
#define SEMICOL 11     //[;]
#define EOFILE 12      //[^D]

//DEFINES
#define Close(FD)                                         \
    do                                                    \
    {                                                     \
        int Close_fd = (FD);                              \
        if (close(Close_fd) == -1)                        \
        {                                                 \
            perror("close");                              \
            fprintf(stderr, "%s:%d: close(" #FD ") %d\n", \
                    __FILE__, __LINE__, Close_fd);        \
        }                                                 \
    } while (0)

//PROTOTYPES
int parse();
static void redirect(int oldfd, int newfd);
static void executePipeline(int pos, int in_fd);
void getCommand(int pos);
void myhandler(int signum);
int setFlags(int rt);
void setRedirection();
void closeRedirection();
int checkForBuiltInFuntions(int *pos);
void execCommand();
