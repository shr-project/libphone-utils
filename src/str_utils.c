#include <string.h>
#include <stdlib.h>

#include "str_utils.h"

int
filter_string(char **string, char filters_array[])
{
	char *result = *string;
	char *end = *string;

	while (*end) {
		char *pos;
		/* go through the null terminated char array
		 * stop after went trough it all or found a match */
		for (pos=filters_array ; *pos && (*pos != *end) ; pos++);
		/* if not matched, the char shouldn't be filtered */
		if (!*pos) {
			*result++ = *end;
		}
		end++;
	}
	*result = '\0';

	*string = realloc(*string, result - *string + 1);

	return result - *string;
}

char *
strchrs(char *string, char filters_array[], int negate)
{
	while (*string) {
		/* check if in the array */
		if (!strchr(filters_array, *string) != !negate)
			break;
		string++;
	}

	return (*string) ? string : NULL;

}

char *
strrchrs(char *string, char filters_array[], int negate)
{
	char *start = string;

	if (!*string)
		return NULL;

	/* find the last non null char */
	while (*++string);
	string--;

	while (string >= start) {
		/* check if in the array */
		if (!strchr(filters_array, *string) != !negate)
			break;
		string--;
	}

	return (*string) ? string : NULL;

}

int
remove_from_chrs(char **string, char filters_array[], int negate) 
{
	char *start = *string;
	char *pos;
	int len;
	pos = strchrs(*string, filters_array, negate);
	if (!pos)
		return strlen(*string);
	*pos = 0;
	
	len = pos - start;
	*string = realloc(*string, len + 1);
	return len;
}

int
remove_from_chrs_r(char **string, char filters_array[], int negate)
{
	char *pos = strrchrs(*string, filters_array, negate);
	if (!pos)
		return strlen(*string);
	if (!*pos) {
		*string = strdup("");
		return 0;
	}
	pos++;
	pos = strdup(pos);

	free(*string);
	*string = pos;
	return strlen(*string);	

}