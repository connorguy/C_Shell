/*
 * getword.c
 *
 * 2017 ConnorGuy <connor_guy@icloud.com>
 * Carroll | CS 570
 *
 * getword.c reads in from stdin and parses words on 
 * tabs and newlines.
 * 
*/

#include "getword.h"

int getword(char *w)
{
	int counter = 0;
	int charValue = 0;
	char in;
	// Boolean for whether the single quote has been set.
	int quoteSet = 0;

	while ((counter < 254) && ((in = getchar()) != EOF))
	{
		charValue = checkCharIn(in);
		// Handle if were handling a quote. Adding in as norm if not char that breaks quote.
		if(quoteSet == 1 && charValue != SINGLE_QUOTE && charValue != NEWLINE && charValue != BACKSLASH && charValue != GREATER_EXCL && charValue != EOF)
		{
			charValue = REGULAR_CHAR;
		}
		// In the case single quotes don't get closed return -3
		if(quoteSet == 1 && charValue == NEWLINE)
		{
			return -3;
		}
		if ((quoteSet != 1 && charValue != REGULAR_CHAR && charValue != BACKSLASH && counter != 0))
		{
			ungetc(in, stdin);
			break;
		}
		
		switch(charValue)
		{
			case REGULAR_CHAR :
			{
				w[counter++] = in;
				break;
			}
			case TAB_SPACE :
			{
				break;
			}
			case BACKSLASH :
			{
				char tmp = getchar(); 
				// if(backslash(w, &counter, quoteSet) == 1) return counter;
				if(quoteSet == 1)
				{
					// if(tmp == '\n') 
					if(tmp == '\'')
					{
						w[counter++] = tmp;
					}
					else
					{
						w[counter++] = in;
						w[counter++] = tmp;
					}
				}
				else
					w[counter++] = tmp;
				
				break;
			}
			case SINGLE_QUOTE :
			{
				if(quoteSet == 1)
					quoteSet = 0;
				else
					quoteSet = 1;
				break;
			}
			case NEWLINE :
			{
				w[0] = '\0';
				return NEWLINE; //EDIT og 0
			}
			case GREATER_THAN :
			{
				w[0] = '>';
				w[1] = '\0';
				return GREATER_THAN;
			}
			case LESSTHAN :
			{
				w[0] = '<';
				w[1] = '\0';
				return LESSTHAN;
			}
			case PIPELINE :
			{
				w[0] = '|';
				w[1] = '\0';
				return PIPELINE;
			}
			case AMPERSAND :
			{
				w[0] = '&';
				w[1] = '\0';
				return AMPERSAND;
			}
			case GREATER_EXCL :
			{
				if(quoteSet == 0)
				{
					w[0] = '>';
					w[1] = '!';
					w[2] = '\0';
					return GREATER_EXCL;
				}
				else
				{
					w[counter++] = '>';
					w[counter++] = '!';
				}
				
				break;
			}
			case SEMICOL :
			{
				w[0] = '\0';
				return SEMICOL; //EDIT og 0
			}
		}
	}
	// If eof is reached before text returned
	if(counter != 0)
	{
		w[counter] = '\0';
		if(quoteSet == 0)
			return REGULAR_CHAR;
		else // In the case single quotes don't get closed
			return -3;
	}


	w[0] = '\0';
	return EOFILE;
}


// Checks in for Delimiters and if they are ones with special instructions.
int checkCharIn(char in)
{
	if(in == ' ' || in == '\t') return TAB_SPACE;
	if(in == '\\') return BACKSLASH;
	if(in == '\'') return SINGLE_QUOTE;
	if(in == '\n') return NEWLINE;
	if(in == '<') return LESSTHAN;
	if(in == '|') return PIPELINE;
	if(in == '&') return AMPERSAND;
	if(in == ';') return SEMICOL;

	if(in == '>')
	{
		if((in = getchar()) == '!')
		{
			return GREATER_EXCL;
		}
		else
		{
			ungetc(in, stdin);
			return GREATER_THAN;
		}
	}
	return REGULAR_CHAR;
}

int backslash(char *w, int *counter, int quoteSet)
{
	char in = getchar();
	if(quoteSet == 1 && in != '\'' && in != '\n')
	{ // Handle in quote backslash
		char tmp = '\\';
		w[*counter++] = tmp;
		w[*counter++] = in;
		return 0;
	}
	else
	{
		if(in == '\n') // If you get a backslash then a newline
		{
			if(*counter != 0)
			{
				ungetc('\n', stdin);
				w[*counter] = '\0';
				return 1;
			}
			else
			{
				w[0] = '\0';
				*counter = 0;
				return 1;
			}
		}
		else
		{
			w[*counter++] = in;
			return 0;
		}
	}
	return 0;
}