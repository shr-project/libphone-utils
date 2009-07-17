/*
 *  Copyright (C) 2008
 *      Authors (alphabetical) :
 *              Tom Hacohen (tom@stosb.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 */

#ifndef _STR_UTILS_H
#define _STR_UTILS_H

int
filter_string(char **string, char filters_array[]);

char *
strchrs(char *string, char filters_array[], int negate);

char *
strrchrs(char *string, char filters_array[], int negate);

int
remove_from_chrs(char **string, char filters_array[], int negate);

int
remove_from_chrs_r(char **string, char filters_array[], int negate);

#endif