/*
 *  Copyright (C) 2009
 *      Authors (alphabetical) :
 *              Tom Hacohen (tom@stosb.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 */

#ifndef _STR_UTILS_H
#define _STR_UTILS_H

/* This function receives a pointer to a string
 * and a NULL terminated array of chars and updates the string
 * to not include the chars from the filters_array.
 *
 * returns the length of the new string
 */
int
filter_string(char *string, char filters_array[]);

char *
strchrs(char *string, char filters_array[], int negate);

char *
strrchrs(char *string, char filters_array[], int negate);

int
remove_from_chrs(char **string, char filters_array[], int negate);

int
remove_from_chrs_r(char **string, char filters_array[], int negate);

int
utf8_get_next(const char *buf, int *iindex);

#endif
