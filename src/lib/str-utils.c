#include <string.h>
#include <stdlib.h>

#include "str-utils.h"

int
filter_string(char *string, char filters_array[])
{
	char *result = string;
	char *end = string;

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

	return result - string;
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

	return (string >= start) ? string : NULL;

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

/* Many thanks to evas */
int
utf8_get_next(const char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the next code point after this.
    *
    * Returns 0 to indicate there is no next char
    */
   int index = *iindex, len, r;
   unsigned char d, d2, d3, d4;

   /* if this char is the null terminator, exit */
   if (!buf[index])
     return 0;
     
   d = buf[index++];

   while (buf[index] && ((buf[index] & 0xc0) == 0x80))
     index++;
   len = index - *iindex;

   if (len == 1)
      r = d;
   else if (len == 2)
     {
	/* 2 bytes */
        d2 = buf[*iindex + 1];
	r = d & 0x1f; /* copy lower 5 */
	r <<= 6;
	r |= (d2 & 0x3f); /* copy lower 6 */
     }
   else if (len == 3)
     {
	/* 3 bytes */
        d2 = buf[*iindex + 1];
        d3 = buf[*iindex + 2];
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
     }
   else
     {
	/* 4 bytes */
        d2 = buf[*iindex + 1];
        d3 = buf[*iindex + 2];
        d4 = buf[*iindex + 3];
	r = d & 0x0f; /* copy lower 4 */
	r <<= 6;
	r |= (d2 & 0x3f);
	r <<= 6;
	r |= (d3 & 0x3f);
	r <<= 6;
	r |= (d4 & 0x3f);
     }

   *iindex = index;
   return r;
}
