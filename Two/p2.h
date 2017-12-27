#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "getword.h"
#include <fcntl.h>

#define STORAGE 255
#define MAXITEM 100
#define TRUE 1
#define FALSE 0
#define STDOUT_FILENO 1

void myhandler(int signum);
void parse(char wordArray[][STORAGE], int flags[], int *wordCountpointer);
int syntaxSanityChecker(int flags[], int wordCount);
int checkForBuiltInFuntions(char wordArray[][STORAGE]);
void cdCommand(char **commandArgs, int commandArgsCount);
void lsCommand(char **commandArgs, int commandArgsCount);
int checkForFlag(int flags[], int wordCount, int flagType);
void pipeCommand(char **pipeCommandArgs, int fildes[]);
void myhandler(int signum);
