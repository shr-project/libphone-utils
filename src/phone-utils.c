/*
 *  Copyright (C) 2008
 *      Authors (alphabetical) :
 *              Tom Hacohen (tom@stosb.com)
 *              
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <glib.h>

#include "phone-utils.h"
#include "str_utils.h"


static char *international_prefix = NULL;
static int international_prefix_len = 0;
static char *national_prefix = NULL;
static int national_prefix_len = 0;
static char *country_code = NULL;
static int country_code_len = 0;
static char *home_code = NULL;
static int home_code_len = 0;

static char *trailing_delimiters = "wWpP;,";
static char *possible_chars = "0123456789+";
static char *filler_chars = " -()";

void 
phone_utils_init()
{
	international_prefix = NULL;
	international_prefix_len = 0;
	national_prefix = NULL;
	national_prefix_len = 0;
	country_code = NULL;
	country_code_len = 0;
	home_code = NULL;
	home_code_len = 0;

	/*
	trailing_delimiters = NULL;
	possible_chars = NULL;
	filler_chars = NULL;
	 */
	phone_utils_init_from_file(PHONE_UTILS_CONFIG);
}

void
phone_utils_deinit()
{
	if (international_prefix)
		free(international_prefix);
	if (national_prefix)
		free(national_prefix);
	if (country_code)
		free(country_code);
	if (home_code)
		free(home_code);
}


void 
phone_utils_set_codes(char *_international_prefix, char *_national_prefix, 
                      char *_country_code, char *_home_code)
{
	if (_international_prefix)
		phone_utils_set_user_international_prefix(_international_prefix);
	if (_national_prefix)
		phone_utils_set_user_national_prefix(_national_prefix);
	if (_country_code)
		phone_utils_set_user_country_code(_country_code);
	if (_home_code)
		phone_utils_set_user_home_code (_home_code);
}

/*void 
phone_utils_set_delimiters(char *trailing_delimiters, char *possible_chars, 
                       char *filler_chars) */

void 
phone_utils_init_from_file(const char *filename) 
{
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;

	keyfile = g_key_file_new ();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
	if (!g_key_file_load_from_file (keyfile, filename, flags, &error)) {
		g_error (error->message);
		return;
	}
	char *tmp;

	phone_utils_set_user_international_prefix(
		tmp = g_key_file_get_string(keyfile,"local","international_prefix",NULL));
	free(tmp);
	phone_utils_set_user_national_prefix(
		tmp = g_key_file_get_string(keyfile,"local","national_prefix",NULL));
	free(tmp);
	phone_utils_set_user_country_code(
		tmp = g_key_file_get_string(keyfile,"local","country_code",NULL));
	free(tmp);
	phone_utils_set_user_home_code(
		tmp = g_key_file_get_string(keyfile,"local","home_code",NULL));
	free(tmp);
	
	g_key_file_free(keyfile);
}


const char *
phone_utils_get_user_international_prefix() 
{
	return international_prefix;
}

const char *
phone_utils_get_user_national_prefix() 
{
	return national_prefix;
}

const char *
phone_utils_get_user_country_code()
{
	return country_code;
}

const char * 
phone_utils_get_user_home_code() 
{
	return home_code;
}

int
phone_utils_set_user_international_prefix(const char *value) 
{
	char *tmp;
	if (!value)
		return 1;
	

	tmp = strdup(value);

	if (!tmp) {
		g_debug("Can't allocate memory! %s:%d\n"
		          "Rolling back the setting\n", __FILE__, __LINE__);
		return 1;
	}
	
	if (international_prefix) 
		free(international_prefix);

	international_prefix = tmp;	
	international_prefix_len = strlen(international_prefix);
	return 0;
	
}

int
phone_utils_set_user_national_prefix(const char *value) 
{
	char *tmp;
	if (!value)
		return 1;
	

	tmp = strdup(value);

	if (!tmp) {
		g_debug("Can't allocate memory! %s:%d\n"
		          "Rolling back the setting\n", __FILE__, __LINE__);
		return 1;
	}
	
	if (national_prefix) 
		free(national_prefix);

	national_prefix = tmp;	
	national_prefix_len = strlen(national_prefix);
	return 0;
}

int
phone_utils_set_user_country_code(const char *value)
{
	char *tmp;
	if (!value)
		return 1;
	

	tmp = strdup(value);

	if (!tmp) {
		g_debug("Can't allocate memory! %s:%d\n"
		          "Rolling back the setting\n", __FILE__, __LINE__);
		return 1;
	}
	
	if (country_code) 
		free(country_code);

	country_code = tmp;	
	country_code_len = strlen(country_code);
	return 0;
}

int
phone_utils_set_user_home_code(const char *value) 
{
	char *tmp;
	if (!value)
		return 1;
	

	tmp = strdup(value);

	if (!tmp) {
		g_debug("Can't allocate memory! %s:%d\n"
		          "Rolling back the setting\n", __FILE__, __LINE__);
		return 1;
	}
	
	if (home_code) 
		free(home_code);

	home_code = tmp;	
	home_code_len = strlen(home_code);
	return 0;
}

char *
phone_utils_normalize_number(const char *_number)
{
	char *number = strdup(_number);
	char *tmp;
	int len;

	remove_from_chrs(&number, trailing_delimiters, 0);
	filter_string(&number, filler_chars);
	len = remove_from_chrs_r(&number, possible_chars, 1);

	/* if normalized, skip */
	if (number[0] == '+') {
	
	}
	/* step 1: normalize 00 to + */
	else if (international_prefix_len > 0 && !strncmp(number, international_prefix, international_prefix_len)) {
		tmp = number;

		number = strdup(&number[international_prefix_len - 1]);
		*number = '+';

		free(tmp);
	}
	/* step 2: normalize national prefix to +<CC>
	* if national_prefix = "" assume it's a match */
	else if (national_prefix_len >= 0 && !strncmp(number, national_prefix, national_prefix_len)) {
		int total_len;
		tmp = number;
		/* malloc: +1 for '+' */ 
		total_len = 1 + country_code_len + len - national_prefix_len; 
		number = malloc(total_len + 1);
		number[0] = '+';
		strncpy(&number[1], country_code, country_code_len);
		strncpy(&number[1 + country_code_len], &tmp[national_prefix_len], len - national_prefix_len);
		number[total_len] = '\0';

		free(tmp);
	}
	/* by default, just try to add +<CC> to the start, better than not trying at all. */
	else {
		int total_len;
		tmp = number;
		/* malloc: +1 for '+' */ 
		total_len = 1 + country_code_len + len;
		number = malloc (total_len + 1);
		number[0] = '+';
		strncpy(&number[1], country_code, country_code_len);
		strncpy(&number[1 + country_code_len], tmp, len);
		number[total_len] = '\0';

		free(tmp);
	}

	return number;
}

int 
phone_utils_numbers_equal(const char * _a, const char * _b)
{
	int ret;
	char *a = phone_utils_normalize_number((gchar *)_a);
	char *b = phone_utils_normalize_number((gchar *)_b);

	if (a && b) {
		if (strcmp(a, b) == 0)
			ret = 1;
	
		g_free(a);
		g_free(b);
	}
	else {
		ret = 0;
	}

	return ret;
}


