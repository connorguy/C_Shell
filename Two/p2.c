/**************************************************************
 p2.c
 
 2017 ConnorGuy <connor_guy@icloud.com>
 Carroll | CS 570
 Due 10/11/17 @ 11pm

C program called p2, which acts as a simple command line 
interpreter for our UNIX system.  

p2 can hadndle redirects in and out. Can do single piping to 
new command. Commands can handle multiple args. Has built in 
functionality for [ls-F] and [cd] commands that do not fork
a new proccess.

In most cases if an error is incountered p2 will output an 
and give user a new prompt.

Some flag types have been defined and used without, these are 
either defined in p2.h or getword.h.
**************************************************************/

#include "p2.h"

// #define DEBUG

int main()
{
	setpgrp();
	while(1) 
	{
		char wordArray[MAXITEM][STORAGE];
		int flags[STORAGE];
		int wordCount = 0;
		int isPipeSet = FALSE;
		int isRedirectIn = FALSE;
		int isRedirectOut = FALSE;
		int redirectIn = 0;
		int redirectOut = 0;
		int aPipe = 0;
		int theCommand = 0;
		char **commandArgs = (char**)malloc(sizeof(char) * 255);
		int commandArgsCount = 0;
		char **pipeCommandArgs;
		int pipeCommandArgsCount = 0;
		int i = 0; // Strictly used to index in for loops, why c99 why.
		int fildes[2];
		int input_fd;
		int output_fd;


		(void) printf("p2: ");
		parse(wordArray, flags, &wordCount);
		printf("\n");

/*****************************************************************************
 Check initial inputs based on flags array that has inputs categorized. See 
 getword.h for the defined interger value deffinitions. Each index in the flags 
 array corresponds to one index in the wordArray array.
*****************************************************************************/
		// Check initial break and continue conditions
		if(flags[0] == EOFILE) break;
		if(flags[0] == NEWLINE) continue;
		if(flags[0] == SEMICOL) continue;
		if(flags[0] == -3) // In the case a single quote doesn't close
		{
			perror("syntactical error, single quote was not closed.");
			continue;
		}
		// Check for syntactical errors
		if(syntaxSanityChecker(flags,wordCount) == -1)
		{
			perror("syntactical error, check input.");
			continue;
		}

/*****************************************************************************
 Parse commands redirects and piplines.
 Save location of inputs within wordArray using int variables.
 Set booleans to true if redirects found.
*****************************************************************************/
		// Find redirects and pipeline
		for(i = 0; i < wordCount; i++)
		{
			if(flags[i] == LESSTHAN) 
			{
				redirectIn = i+1;
				isRedirectIn = TRUE;
			}
			if(flags[i] == GREATER_THAN) 
			{
				redirectOut = i+1;
				isRedirectOut = TRUE;
			}
			if(flags[i] == PIPELINE)
			{
				isPipeSet = TRUE;
				aPipe = i+1;
			}
		}

/*****************************************************************************
 Parsing commands into individual arg arrays, and denoting counters if needed.
*****************************************************************************/
		// Need to check if the pipe command has any inputs.
		if(isPipeSet == TRUE && flags[aPipe+1] == REGULAR_CHAR)
		{
			pipeCommandArgs = (char**)malloc(sizeof(char) * 255);
			pipeCommandArgs[pipeCommandArgsCount++] = strdup(wordArray[aPipe]);
			for(i = aPipe+1; i < wordCount; i++)
			{
				if(flags[i] == REGULAR_CHAR) pipeCommandArgs[pipeCommandArgsCount++] = strdup(wordArray[i]);
				else break;
			}
			pipeCommandArgs[pipeCommandArgsCount++] = NULL;
		}
		else
		{
			pipeCommandArgs = NULL;
		}
		// Find command in the input
		if(flags[0] == REGULAR_CHAR) theCommand = 0;
		else
		{
			// Need to find the command within array
			int wasRegChar = FALSE; // Used for something like "< in > out command"
			for(i = 0; i < wordCount; i++)
			{
				if(flags[i] != REGULAR_CHAR) wasRegChar = FALSE;
				if(flags[i] == REGULAR_CHAR)
				{
					if(wasRegChar == TRUE) theCommand = i;
					else wasRegChar = TRUE;
				}
			}
		}
		// Find any command input
		if(!redirectIn)
		{
			commandArgs[commandArgsCount++] = strdup(wordArray[theCommand]); // Strangly if not set arg[0] never gets run.
			for(i = theCommand+1;;i++)
			{
				if(flags[i] == REGULAR_CHAR) commandArgs[commandArgsCount++] = strdup(wordArray[i]);
				else break;
			}
			commandArgs[commandArgsCount++] = NULL;
		}

		#ifdef DEBUG /*************DUMP*EVERYTHING*YOU*KNOW*PLZ*SAVE*ME*************************DEBUG*/
			printf("\n");
			for(int i = 0; i < wordCount; i++)
			{
				printf("Word Type: %d, Word Out: %s\n", flags[i], wordArray[i]);
			}
			printf("Word count: %d\n", wordCount);

			if(isRedirectIn == TRUE) printf("R in: %s\n", wordArray[redirectIn]);// FOR DEGUB
			if(isRedirectOut == TRUE) printf("R out: %s\n", wordArray[redirectOut]);// FOR DEGUB
			if(isPipeSet == TRUE)
			{
				printf("PIpen: %s\n", wordArray[aPipe]);// FOR DEGUB
				for(int i = 0; i < pipeCommandArgsCount; i++)
				{
					printf("pipe arg @%d: %s\n", i, pipeCommandArgs[i]);
				}
			} 
			if(redirectIn != TRUE)
			{
				printf("\n");
				for(i = 0; i < commandArgsCount; i++)
				{
					printf("Command args: %s\n", commandArgs[i]);
				}
			}
			printf("Command: %s\n", wordArray[theCommand]);// FOR DEGUB
			printf("\n");
			continue;
		#endif /****************************************************************************DEBUG*/

/*****************************************************************************
Check if user commands are built in functionality ie: [ls-F] or [cd].
If any match call corresponding function and continue after function returns
*****************************************************************************/
		// Check if user commands are built in functionality
		int valueCheck = 0;
		if((valueCheck = checkForBuiltInFuntions(wordArray)) != 0)
		{
			if(valueCheck == 1) lsCommand(commandArgs, commandArgsCount);
			if(valueCheck == 2) cdCommand(commandArgs, commandArgsCount);
			continue;
		}

/*****************************************************************************
Fork the process. Modify redirects or pipelinging if neccessary. Then execute 
execvp. If ampersands are set parent process will background the operation otherwise 
parent will wait till child dies to continue operation.
*****************************************************************************/
   		int kidpid = fork();
		/* do the fork, remember the value returned by fork(), and test it */
		if (kidpid == -1) 
		{ 
			/* -1 indicates the fork was unsuccessful */
			perror("Cannot fork");
			continue;
		}
		else if(kidpid == 0)
		{
			printf("child says: my pid = %d, parent pid = %d\n", getpid(), getppid());
			// redirect I/O as requested;
			if(isRedirectIn == TRUE)
			{
				input_fd=open(wordArray[redirectIn], O_RDONLY, S_IRUSR | S_IWUSR);
				if(input_fd < 0) 
				{
					perror("open failed.");
					exit(1);
  				}
  				if(dup2(input_fd,STDOUT_FILENO) == -1)
  				{
					perror("dup2 failed.");
					exit(1);
  				}

			}
			if(isRedirectOut == TRUE && isPipeSet == FALSE)
			{
				output_fd=open(wordArray[redirectOut], O_CREAT|O_EXCL|O_WRONLY, S_IRUSR | S_IWUSR);
				if(output_fd < 0) 
				{
					perror("open failed.");
					exit(1);
  				}
  				if(dup2(output_fd,STDOUT_FILENO) == -1)
  				{
					perror("dup2 failed.");
					exit(1);
  				}
			}
			if(isPipeSet == TRUE)
			{
				pipe(fildes);

				// Redirect output for pipe command
		  		if(dup2(fildes[0], STDOUT_FILENO) == -1)
				{
					perror("dup2 failed.");
					exit(1);
				}
				if(close(fildes[0]) != 0)
				{
					perror("close failed.");
					exit(1);
				}
				if(close(fildes[1]) != 0)
				{
					perror("close failed.");
					exit(1);
				}
				pipeCommand(pipeCommandArgs, fildes);
			}

			// use execvp() to start requested process;
			if(execvp(wordArray[theCommand], commandArgs) < 0)
			{
				perror("Cannot execute command.");
				exit(9);
			}
			else
			{
				// Close out everything
				if(isRedirectOut == TRUE)
				{
					if(close(output_fd) != 0)
					{
						perror("close failed.");
						continue;
					}
				}
				if(isRedirectIn == TRUE)
				{
					if(close(input_fd) != 0)
					{
						perror("close failed.");
						continue;
					}
				}
			}
		}
		else
		{
			if(!checkForFlag(flags, wordCount, AMPERSAND))// check if the & is NOT set
			{
				wait(&kidpid); // Not sure if this is right
			}
		}
		fflush(stderr);
		// Free memory that has been put on the stack.
		if(commandArgsCount != 0) free(commandArgs);
		if(pipeCommandArgsCount != 0) free(pipeCommandArgs);
		signal(SIGTERM, myhandler);
	}
	killpg(getpgrp(), SIGTERM); // Terminate any children that are
	// still running. WARNING: giving bad args
	// to killpg() can kill the autograder!
	printf("p2 terminated.\n");

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
void parse(char wordArray[][STORAGE], int flags[], int *wordCountpointer)
{
	int wordCount = 0;
	int getwordReturnValue = 0;
	char word[STORAGE];
	// call getword
	while(getwordReturnValue != EOFILE && getwordReturnValue != NEWLINE && getwordReturnValue != -3 && getwordReturnValue != SEMICOL)
	{
		getwordReturnValue = getword(word);
		flags[wordCount] = getwordReturnValue;
		strcpy(wordArray[wordCount++], word);
	}
	if(getwordReturnValue == -3) flags[0] = -3; // In the case single quotes don't get closed return -3
	*wordCountpointer = wordCount;
}

/*****************************************************************************
Insures that the input given matches the various rules outlined for syntax in p2.
*****************************************************************************/
int syntaxSanityChecker(int flags[], int wordCount)
{
	int isPipeHere = FALSE;
	int isDuplicateGreaterThan = FALSE;
	int isDuplicateLessThan = FALSE;
	int isDuplicateOption = FALSE;
	int i = 0;
	for(i = 0; i < wordCount; i++)
	{
		// Check for multiple pipelines
		if(flags[i] == PIPELINE)
		{
			if(isPipeHere == TRUE) return -1;
			else isPipeHere = TRUE;
			if(flags[i+1] != REGULAR_CHAR) return -1;
		}

		// Check if double [>]
		if(flags[i] == GREATER_THAN)
		{
			if(isDuplicateGreaterThan == TRUE) return -1;
			else isDuplicateGreaterThan = TRUE;
		}
		else if(flags[i] != REGULAR_CHAR) isDuplicateGreaterThan = FALSE;

		// Check if double [<]
		if(flags[i] == LESSTHAN)
		{
			if(isDuplicateLessThan == TRUE) return -1;
			else isDuplicateLessThan = TRUE;
		}
		else if(flags[i] != REGULAR_CHAR) isDuplicateLessThan = FALSE;

		// Check if you have two metachars in a row
		if(flags[i] != REGULAR_CHAR && isDuplicateOption == FALSE) isDuplicateOption = TRUE;
		else if(flags[i] != REGULAR_CHAR && isDuplicateOption == TRUE) return -1;
		else isDuplicateOption = FALSE;
	}

	// Check for "< input > output command" pattern
	if(flags[0] != REGULAR_CHAR)
	{
		int pattern[5] = {LESSTHAN, REGULAR_CHAR, GREATER_THAN, REGULAR_CHAR, REGULAR_CHAR};
		int i = 0;
		for(i = 0; i < 6; i++)
		{
			if(flags[i] != pattern[i]) return -1;
		}
	}
	return 0;	
}

/*****************************************************************************
Check if we are doing any ls-F [return 1] or cd [return 2] commands
*****************************************************************************/
int checkForBuiltInFuntions(char wordArray[][STORAGE])
{
	char lsfCommand[] = "ls-F";
	char cdCommand[] = "cd";
	// strcmp returns a 0 if true -> !0 is what is being checked for.
	if(!strcmp(lsfCommand,wordArray[0])) return 1;
	if(!strcmp(cdCommand,wordArray[0])) return 2;
	return 0;
}

/*****************************************************************************
Built in functionality that runs the cd command without forking a child.
*****************************************************************************/
void cdCommand(char **commandArgs, int commandArgsCount)
{
	// char *homePath = getenv("HOME");
	// TODO
	// Method still gets called in the case we find the command 
	// But since unemplemented will continue the main method which
	// intern forks a command and runs execvp.
}

/*****************************************************************************
Built in functionality that runs the ls-F command without forking a child.
*****************************************************************************/
void lsCommand(char **commandArgs, int commandArgsCount)
{
	// TODO
	// Method still gets called in the case we find the command 
	// But since unemplemented will continue the main method which
	// intern forks a command and runs execvp.
}

/*****************************************************************************
Iterate flags and see if flagType is there. See getword.h for flags
*****************************************************************************/
int checkForFlag(int flags[], int wordCount, int flagType)
{
	int i = 0;
	for(i = 0; i < wordCount; i++)
	{
		if(flags[i] == flagType) return 1;
	}
	return 0;
}

/*****************************************************************************
Fork the process. Modify pipelinging. Then execute 
execvp. If ampersands are set parent process will background the operation otherwise 
parent will wait till child dies to continue operation.
*****************************************************************************/
void pipeCommand(char **pipeCommandArgs, int fildes[])
{
	   	int kidpid = fork();
		/* do the fork, remember the value returned by fork(), and test it */
		if (kidpid == -1) 
		{ 
			// -1
			perror("Cannot fork");
			return;
		}
		else if(kidpid == 0)
		{
			// Redirect input for pipe command and close input output
	  		if(dup2(fildes[0], STDIN_FILENO) == -1)
			{
				perror("dup2 failed.");
				exit(1);
			}
			if(close(fildes[0]) != 0)
			{
				perror("close failed.");
				exit(1);
			}
			if(close(fildes[1]) != 0)
			{
				perror("close failed.");
				exit(1);
			}

			// use execvp() to start requested process;
			if(execvp(pipeCommandArgs[0], pipeCommandArgs) < 0)
			{
				perror("Cannot execute command.");
				exit(9);
			}
		}
}