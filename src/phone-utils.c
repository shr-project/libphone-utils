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
struct PrefixField {
	const char *value;
	int len;
};

struct Prefix {
	struct PrefixField *fields;
	char *value;
	int len;
};

static struct Prefix international_prefix = {NULL, NULL, 0};
static struct Prefix national_prefix = {NULL, NULL, 0};
static struct Prefix country_code = {NULL, NULL, 0};
static struct Prefix area_code = {NULL, NULL, 0};


/* all the information needed for processing a number */
static char *trailing_delimiters = "wWpP;,";
#define POSSIBLE_DIGITS "0123456789ABCD"
static char *possible_chars = POSSIBLE_DIGITS "+";
static char *filler_chars = " -()";

int
phone_utils_init()
{
	international_prefix.fields = NULL;
	international_prefix.value = NULL;
	national_prefix.fields = NULL;
	national_prefix.value = NULL;
	country_code.fields = NULL;
	country_code.value = NULL;
	area_code.fields = NULL;
	area_code.value = NULL;

	/*
	trailing_delimiters = NULL;
	possible_chars = NULL;
	filler_chars = NULL;
	 */
	return phone_utils_init_from_file(PHONE_UTILS_CONFIG);
}

static void
_phone_utils_free_prefix(struct Prefix *prefix)
{
	if (prefix->value) {
		free(prefix->value);
	}
}


void
phone_utils_deinit()
{
	_phone_utils_free_prefix(&international_prefix);
	_phone_utils_free_prefix(&national_prefix);
	_phone_utils_free_prefix(&country_code);
	_phone_utils_free_prefix(&area_code);
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
		tmp = phone_utils_get_user_international_prefixes();
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "international_prefix", tmp);

		tmp = phone_utils_get_user_national_prefixes();	
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "national_prefix", tmp);

		tmp = phone_utils_get_user_country_codes();
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "country_code", tmp);

		tmp = phone_utils_get_user_area_codes();
		g_key_file_set_value(keyfile, PHONE_UTILS_CONFIG_CODES_GROUP, 
		                     "area_code", tmp);
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
	return international_prefix.value && national_prefix.value
		&& country_code.value && area_code.value
		&& trailing_delimiters && possible_chars && filler_chars;
}

static const char *
_phone_utils_get_user_prefix(const struct Prefix *field)
{
	if (!field || !field->fields)
		return NULL;
		
	return (field[0].value) ? field[0].value : NULL;
}

static char *
_phone_utils_get_strdup_user_prefix(const struct Prefix *field)
{
	const char *ret;
	ret = _phone_utils_get_user_prefix(field);
	return (ret) ? strdup(ret) : NULL;
}


char *
phone_utils_get_strdup_user_international_prefix() 
{
	return _phone_utils_get_strdup_user_prefix(&international_prefix);
}

char *
phone_utils_get_strdup_user_national_prefix() 
{
	return _phone_utils_get_strdup_user_prefix(&national_prefix);
}

char *
phone_utils_get_strdup_user_country_code()
{
	return _phone_utils_get_strdup_user_prefix(&country_code);
}

char * 
phone_utils_get_strdup_user_area_code() 
{
	return _phone_utils_get_strdup_user_prefix(&area_code);
}

const char *
phone_utils_get_user_international_prefix() 
{
	return _phone_utils_get_user_prefix(&international_prefix);
}

const char *
phone_utils_get_user_national_prefix() 
{
	return _phone_utils_get_user_prefix(&national_prefix);
}

const char *
phone_utils_get_user_country_code()
{
	return _phone_utils_get_user_prefix(&country_code);
}

const char * 
phone_utils_get_user_area_code() 
{
	return _phone_utils_get_user_prefix(&area_code);
}

static char *
_phone_utils_get_user_prefixes(struct Prefix *prefix)
{
	int i;
	char *ret = NULL;
	char *src;
	char *dst;
	if (!prefix)
		return NULL;

	ret = malloc(prefix->len + 1);
	if (!ret) {
		g_debug("Can't allocate memory! %s:%d\n"
		        "Getting user prefixes failed\n", __FILE__, __LINE__);
		return NULL;
	}
	dst = ret;
	src = prefix->value;
	for (i = 0 ; i < prefix->len ; i++) {
		if (*src == '\0') {
			*dst = PHONE_UTILS_FIELD_DELIMITER;
		}
		else {
			*dst = *src;
		}
		src++;
		dst++;
	}
	*dst = '\0';
	
	return ret;
}

char *
phone_utils_get_user_international_prefixes() 
{
	return _phone_utils_get_user_prefixes(&international_prefix);
}

char *
phone_utils_get_user_national_prefixes() 
{
	return _phone_utils_get_user_prefixes(&national_prefix);
}

char *
phone_utils_get_user_country_codes()
{
	return _phone_utils_get_user_prefixes(&country_code);
}

char *
phone_utils_get_user_area_codes() 
{
	return _phone_utils_get_user_prefixes(&area_code);
}

static int
_phone_utils_set_user_prefix(struct Prefix *prefix, const char *value)
{ /* Set both len and actual value */
	char *pos, *prev;
	int i;
	int num_of_fields;

	if (!value)
		value = "";

	_phone_utils_free_prefix(prefix);
	prefix->value = strdup(value);
	if (!prefix->value) {
		g_debug("Can't allocate memory! %s:%d\n"
		          	"Rolling back the setting\n", __FILE__, __LINE__);
		return 1;
	}
	prefix->len = strlen(prefix->value);

	num_of_fields = 1; /* at least one, as we have the basic one */
	for (pos = prefix->value ; *pos ; pos++) {
		if (*pos == PHONE_UTILS_FIELD_DELIMITER) {
			num_of_fields++;
		}
	}
	/* if we had more than one fields, we should add one more because the number
	 * of fields is the number of delimiters +1 */
	num_of_fields += (num_of_fields > 1) ? 1 : 0;
	
	/* Allocate fields and null terminate the array */
	prefix->fields = malloc(sizeof(struct PrefixField) * (num_of_fields + 1));
	if (!prefix->fields) {
		g_debug("Can't allocate memory! %s:%d\n"
		          	"Rolling back the setting\n", __FILE__, __LINE__);
		free(prefix->value);
		prefix->value = NULL;
		return 1;
	}

	prev = pos = prefix->value;
	i = 0;
	while (1) {
		if (*pos == PHONE_UTILS_FIELD_DELIMITER || !*pos) {
			prefix->fields[i].value = prev;
			prefix->fields[i].len = pos - prev;
			
			i++;
			if (!*pos) {
				break;
			}
			*pos = '\0';
			prev = ++pos;
		}
		else {
			pos++;
		}
		
	}
	/* after we finish, null terminate the array */
	prefix->fields[i].value = NULL;
	
	return 0;
}

int
phone_utils_set_user_international_prefix(const char *value) 
{
	return _phone_utils_set_user_prefix(&international_prefix, value);
	
}

int
phone_utils_set_user_national_prefix(const char *value) 
{
	return _phone_utils_set_user_prefix(&national_prefix, value);
}

int
phone_utils_set_user_country_code(const char *value)
{
	return _phone_utils_set_user_prefix(&country_code, value);
}

int
phone_utils_set_user_area_code(const char *value) 
{
	return _phone_utils_set_user_prefix(&area_code, value);
}

static int
_phone_utils_get_prefix_of_number(struct Prefix *prefix, const char *number)
{
	struct PrefixField *field;
	int i;
	int empty_index = -1;
	
	for (i = 0, field = prefix->fields ; field->value ; field++, i++) {
		if (field->len == 0) {
			empty_index = i;
		}
		else if (!strncmp(field->value, number, field->len)) {
			return i;
		}
		
	}
	
	return empty_index;
}

/*FIXME: handle failed memory allocations! */
char *
phone_utils_normalize_number_using_params(const char *_number, const char *param_country,
					const char *param_area)
{
	/*FIXME: add support for normalizing completely to what's wanted */
	char *number = strdup(_number);
	char *tmp;
	int found;
	char *pos;
	int len;
	const char *international, *national, *country, *area;
	int international_len, national_len, country_len, area_len;
	/* Optimize, already got those saved, get them */
	international = phone_utils_get_user_international_prefix();
	national = phone_utils_get_user_national_prefix();
	country = phone_utils_get_user_country_code();
	area = phone_utils_get_user_area_code();
	international_len = strlen(international);
	national_len = strlen(national);
	country_len = strlen(country);
	area_len = strlen(area);

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

	/* if normalized, replace the CC and AC to the new ones*/
	if (number[0] == '+') {
		/* Strip CC and AC and then replace them. */
		pos = number + 1;
		found = _phone_utils_get_prefix_of_number(&country_code, pos);
		if (found >= 0) {
			pos += country_code.fields[found].len;
			found = _phone_utils_get_prefix_of_number(&area_code, pos);
			if (found >= 0) {
				int total_len;
				int param_country_len = strlen(param_country);
				int param_area_len = strlen(param_area);
				pos += area_code.fields[found].len;
				len -= pos - number;
				tmp = number;
				/* malloc: +1 for '+' */ 
				total_len = 1 + param_country_len + param_area_len + len;
				number = malloc (total_len + 1);
				number[0] = '+';
				strncpy(&number[1], param_country, param_country_len);
				strncpy(&number[1 + param_country_len], param_area, param_area_len);
				strncpy(&number[1 + param_country_len + param_area_len], pos, len);
				number[total_len] = '\0';

				free(tmp);
			}
		}
	
	}
	/* step 1: normalize 00 to + */
	else if (international_len > 0 && !strncmp(number, international, international_len)) {
		tmp = number;

		number = strdup(&number[international_len - 1]);
		*number = '+';

		free(tmp);
	}
	/* step 2: normalize national prefix to +<CC>
	* if national = "" assume it's a match */
	else if (national_len >= 0 && !strncmp(number, national, national_len)) {
		int total_len;
		int param_country_len = strlen(param_country);
		tmp = number;
		/* malloc: +1 for '+' */ 
		total_len = 1 + param_country_len + len - national_len; 
		number = malloc(total_len + 1);
		number[0] = '+';
		strncpy(&number[1], param_country, param_country_len);
		strncpy(&number[1 + param_country_len], &tmp[national_len], len - national_len);
		number[total_len] = '\0';

		free(tmp);
	}
	/* by default, just try to add +<CC> to the start, better than not trying at all. */
	else {
		int total_len;
		int param_country_len = strlen(param_country);
		tmp = number;
		/* malloc: +1 for '+' */ 
		total_len = 1 + param_country_len + len;
		number = malloc (total_len + 1);
		number[0] = '+';
		strncpy(&number[1], param_country, param_country_len);
		strncpy(&number[1 + param_country_len], tmp, len);
		number[total_len] = '\0';

		free(tmp);
	}

	return number;
}

char *
phone_utils_normalize_number(const char *_number)
{
	const char *country, *area;
	char *ret;
	country = phone_utils_get_user_country_code();
	area = phone_utils_get_user_area_code();
	ret = phone_utils_normalize_number_using_params(_number, country, area);
	return ret;
}

/* _a will be normalized using all the known options _b will be optimized
 * using only the basic */
int 
phone_utils_numbers_equal(const char * _a, const char * _b)
{
	int ret = 0;
	char *a;
	char *b = phone_utils_normalize_number(_b);
	char *pos;
	int cpos, apos;
	
	if (!b) 
		return 0;
	
	pos = b + 1; /* Get to the first char after the + */
	cpos = _phone_utils_get_prefix_of_number(&country_code, pos);
	if (cpos >= 0) {
		pos += country_code.fields[cpos].len;
	}
	apos = _phone_utils_get_prefix_of_number(&area_code, pos);
	if (apos >= 0) {
		pos += area_code.fields[apos].len;
	}
	a = phone_utils_normalize_number_using_params(
		_a, country_code.fields[cpos].value, area_code.fields[apos].value);	

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
