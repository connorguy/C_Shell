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

// Prototypes
int checkCharIn(char in);
int regularDelimiters(char in, char *w, int counter);
int ignoreDelimiters(char *w, int counter);
int newLine(char *w, int counter);

int getword(char *w)
{
	int counter = 0;
	int charValue = 0;
	char in;
	// Boolean for whether the single quote has been set.
	int quoteSet = 0;

	while (((in = getchar()) != EOF) && (counter < 254))
	{
		charValue = checkCharIn(in);
		// Handle if were handling a quote. Adding in as norm if not char that breaks quote.
		if(quoteSet == 1 && charValue != 4 && charValue != 5 && charValue != 3 && charValue != 6)
		{
			charValue = 0;
		}
		
		switch(charValue)
		{
			case 0 : // Normal char
			{
				w[counter++] = in;
				break;
			}
			case 1 : // Regular delimiter
			{
				return regularDelimiters(in, w, counter);
				break;
			}
			case 2 : // Spaces tabs
			{
				if(counter == 0)
				{
					break;
				}
				if(ignoreDelimiters(w, counter) != 0)
					return counter;
			}
			case 3 : // Backslash char
			{
				in = getchar();
				if(quoteSet == 1 && in != '\'' && in != '\n')
				{ // Handle in quote backslash
					char tmp = '\\';
					w[counter++] = tmp;
					w[counter++] = in;
				}
				else
					if(in == '\n') // If you get a backslash then a newline
						return newLine(w, counter);
					else
						w[counter++] = in;
				break;
			}
			case 4 : // Single quote
			{
				if(quoteSet == 1)
					quoteSet = 0;
				else
					quoteSet = 1;
				break;
			}
			case 5 : // New line
			{
				return newLine(w, counter);
			}
			case 6 : // Handle >! expressions
			{
				if(counter != 0 && quoteSet != 1)
				{
					ungetc('!', stdin);
					ungetc('>', stdin);
					return ignoreDelimiters(w, counter);
				}
				w[counter++] = '>';
				w[counter++] = '!';
				w[counter] = '\0';
				if(quoteSet != 1)
					return counter;
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


// Checks in for Delimiters and if they are ones with special instructions.
int checkCharIn(char in)
{
	if(in == '<' || in == '|' || in == '&')
		return 1;
	if(in == ' ' || in == '\t')
		return 2;
	if(in == '\\')
		return 3;
	if(in == '\'')
		return 4;
	if(in == '\n' || in == ';')
		return 5;
	if(in == '>')
	{
		if((in = getchar()) == '!')
		{
			return 6;
		}
		else
		{
			ungetc(in, stdin);
			return 1;
		}
	}
	return 0;
}

// For any characters that split up tokens but also need to be outputted to console.
int regularDelimiters(char in, char *w, int counter)
{
	// If there is anything that is in w already put input back to stdin and null terminate w
	if(counter != 0)
	{
		ungetc(in, stdin); // Push in back to stdin for next pass.
		w[counter] = '\0';
		return counter;
	}
	else
	{
		// We are just putting in into w if nothing is in w
		w[0] = in;
		w[1] = '\0';
		return 1;
	}
}

// For chars that break up tokens but will not be printed to console (tabs and spaces).
int ignoreDelimiters(char *w, int counter)
{
	if(counter != 0)
	{
		w[counter] = '\0';
		return counter;
	}
	return 0;
}

// Handle new line actions
int newLine(char *w, int counter)
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