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

#include <stdio.h>
#include <ctype.h>

int getword(char *w)
{
	int counter = 0;
	char in;

	while ( ( in = getchar() ) != EOF ) // Taken from Foster's inout2.c
	{
		if(in != '\t' && in != '\n')
		{
			w[counter] = in;
			counter++;
		}

		// Tab handler
		if(in == '\t')
		{
			if(counter != 0)
			{
				w[counter] = '\0';
				return counter;
			}
		}

		// New line handler
		if(in == '\n')
		{
			if(counter != 0)
			{
				ungetc('\n', stdin);
				w[counter] = '\0';
				return counter;
			}
			else
			{
				w[0] = '\0';
				return 0;
			}
		}
	}

	// If eof is reached before text returned
	if(counter != 0)
	{
		w[counter] = '\0';
		return counter;
	}

	w[0] = '\0';
	return -1;

}