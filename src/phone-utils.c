/*
 *  Copyright (C) 2009
 *	Authors (alphabetical) :
 *		Tom Hacohen (tom@stosb.com)
 *
 *	Thanks (alphabetical) :
 *		Joerg "DocScrutinizer" Reisenweber (joerg@openmoko.org / reisenweber@gmx.net) - The number normalization method is mostly based on an algorithm suggested by him.
 *              
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <glib.h>

#include "phone-utils.h"
#include "str-utils.h"

/* the internal prefixes */
static char *international_prefix = NULL;
static int international_prefix_len = 0;
static char *national_prefix = NULL;
static int national_prefix_len = 0;
static char *country_code = NULL;
static int country_code_len = 0;
static char *area_code = NULL;
static int area_code_len = 0;

/* all the information needed for processing a number */
static char *trailing_delimiters = "wWpP;,";
#define POSSIBLE_DIGITS "0123456789ABCD"
static char *possible_chars = POSSIBLE_DIGITS "+";
static char *filler_chars = " -()";

int
phone_utils_init()
{
	international_prefix = NULL;
	international_prefix_len = 0;
	national_prefix = NULL;
	national_prefix_len = 0;
	country_code = NULL;
	country_code_len = 0;
	area_code = NULL;
	area_code_len = 0;

	/*
	trailing_delimiters = NULL;
	possible_chars = NULL;
	filler_chars = NULL;
	 */
	return phone_utils_init_from_file(PHONE_UTILS_CONFIG);
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
	if (area_code)
		free(area_code);

	international_prefix = national_prefix = country_code = area_code = NULL;
}


int
phone_utils_set_codes(char *_international_prefix, char *_national_prefix, 
                      char *_country_code, char *_area_code)
{
	int ret = 0;
	if (_international_prefix)
		ret = ret || phone_utils_set_user_international_prefix(_international_prefix);
	if (_national_prefix)
		ret = ret || phone_utils_set_user_national_prefix(_national_prefix);
	if (_country_code)
		ret = ret || phone_utils_set_user_country_code(_country_code);
	if (_area_code)
		ret = ret || phone_utils_set_user_area_code (_area_code);

	return ret;
}

/*void 
phone_utils_set_delimiters(char *trailing_delimiters, char *possible_chars, 
                       char *filler_chars) */

int
phone_utils_init_from_file(const char *filename) 
{
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;
	char *tmp;
	int ret = 0;

	keyfile = g_key_file_new ();
	if (!keyfile) {
		g_debug("Can't allocate memory! %s:%d\n"
		        "Initializing key from file failed\n", __FILE__, __LINE__);
		return 1;
	}
	flags = G_KEY_FILE_KEEP_TRANSLATIONS;
	
	if (!g_key_file_load_from_file (keyfile, filename, flags, &error)) {
		g_key_file_free(keyfile);
		g_warning ("%s", error->message);
		g_error_free(error);
		return 1;
	}

	/* Init all the fields */
	tmp = g_key_file_get_string(keyfile,PHONE_UTILS_CONFIG_CODES_GROUP,
		                            "international_prefix",NULL);
	ret = ret || phone_utils_set_user_international_prefix(tmp);
	g_free(tmp);
	
	tmp = g_key_file_get_string(keyfile,PHONE_UTILS_CONFIG_CODES_GROUP,
		                            "national_prefix",NULL);
	ret = ret || phone_utils_set_user_national_prefix(tmp);
	g_free(tmp);


	tmp = g_key_file_get_string(keyfile,PHONE_UTILS_CONFIG_CODES_GROUP,
		                            "country_code",NULL);
	ret = ret || phone_utils_set_user_country_code(tmp);
	g_free(tmp);
	
	tmp = g_key_file_get_string(keyfile,PHONE_UTILS_CONFIG_CODES_GROUP,
		                            "area_code",NULL);
	ret = ret || phone_utils_set_user_area_code(tmp);
	g_free(tmp);

	g_key_file_free(keyfile);
	return ret;
}

int
phone_utils_save_config()
{
	return phone_utils_save_config_to_file(PHONE_UTILS_CONFIG);
}

int
phone_utils_save_config_to_file(const char *filename) 
{
	GKeyFile *keyfile;
	GError *error = NULL;
	FILE *out;
	char *data;
	int ret = 0;

	keyfile = g_key_file_new ();
	if (!keyfile) {
		g_debug("Can't allocate memory! %s:%d\n"
		        "Initializing key from file failed\n", __FILE__, __LINE__);
		ret = 1;
		goto end;
	}

	/* set all the values */
	{
		char *tmp;
		tmp = phone_utils_get_user_international_prefix();
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "international_prefix", tmp);
		free(tmp);

		tmp =  phone_utils_get_user_national_prefix();	
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "national_prefix", tmp);
		free(tmp);

		tmp =  phone_utils_get_user_country_code();
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "country_code", tmp);
		free(tmp);

		tmp =  phone_utils_get_user_area_code();
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "area_code", tmp);
		free(tmp);
	}
	
	if (!(data = g_key_file_to_data (keyfile, NULL, &error))) {
		g_warning ("%s", error->message);
		g_error_free(error);
		ret = 1;
		goto free_key_file;
	}

	out = fopen(filename, "w");
	if (!out) {
		g_debug("%s:%d Can't open file %s! (%s)\n",
		        __FILE__, __LINE__, filename, strerror(errno));
		ret = 1;
		goto free_data;
	}

	fputs(data, out);
	if (ferror(out)) {
		g_debug("%s:%d Can't write to file %s! (%s)\n",
		        __FILE__, __LINE__, filename, strerror(errno));
	}
	
/*clean up and exit */
	
	fclose(out);
free_data:
	g_free(data);
free_key_file:
	g_key_file_free(keyfile);
end:
	return ret;
}

int
phone_utils_is_initialized()
{
	return international_prefix && national_prefix && country_code && area_code
		&& trailing_delimiters && possible_chars && filler_chars;
}

char *
phone_utils_get_user_international_prefix() 
{
	return (international_prefix) ? strdup(international_prefix) : NULL;
}

char *
phone_utils_get_user_national_prefix() 
{
	return (national_prefix) ? strdup(national_prefix) : NULL;
}

char *
phone_utils_get_user_country_code()
{
	return (country_code) ? strdup(country_code) : NULL;
}

char * 
phone_utils_get_user_area_code() 
{
	return (area_code) ? strdup(area_code) : NULL;
}

int
phone_utils_set_user_international_prefix(const char *value) 
{
	char *tmp;
	if (!value)
		value = "";
	

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
		value = "";
	

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
		value = "";
	

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
phone_utils_set_user_area_code(const char *value) 
{
	char *tmp;
	if (!value)
		value = "";
	

	tmp = strdup(value);

	if (!tmp) {
		g_debug("Can't allocate memory! %s:%d\n"
		          "Rolling back the setting\n", __FILE__, __LINE__);
		return 1;
	}
	
	if (area_code) 
		free(area_code);

	area_code = tmp;	
	area_code_len = strlen(area_code);
	return 0;
}

/*FIXME: handle failed memory allocations! */
char *
phone_utils_normalize_number(const char *_number)
{
	char *number = strdup(_number);
	char *tmp;
	int len;

	/* on error */
	if (!number) {
		return NULL;
	}
	
	remove_from_chrs(&number, trailing_delimiters, 0);
	filter_string(&number, filler_chars);
	len = remove_from_chrs_r(&number, possible_chars, 1);

	/* if empty, i.e some sort of code / ilegal number, return the number */
	if (len == 0) {
		free(number);
		return strdup(_number);
	}

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
	int ret = 0;
	char *a = phone_utils_normalize_number(_a);
	char *b = phone_utils_normalize_number(_b);

	if (a && b) {
		if (strcmp(a, b) == 0) {
			ret = 1;
		}
	}
	else {
		ret = 0;
	}

	/* clean up */
	if (a)
		free(a);
	if (b)
		free(b);

	return ret;
}

int
phone_utils_sms_is_valid_number(const char *number)
{
	if (!number)
		return 0;

	/* FIXME: YUCK! I hate hardcoded stuff */
	if (*number == '+')
		number++;

	while (*number) {
		if (!strchr(POSSIBLE_DIGITS, *number)) {
			return 0;
		}
		number++;
	}

	return 1;
}

int
phone_utils_call_is_valid_number(const char *number)
{
	if (!number)
		return 0;

	/* FIXME: YUCK! I hate hardcoded stuff */
	if (*number == '+')
		number++;

	while (*number) {
		if (!strchr(POSSIBLE_DIGITS, *number) &&
			!strchr(trailing_delimiters, *number)) {
			return 0;
		}
		number++;
	}

	return 1;
}
