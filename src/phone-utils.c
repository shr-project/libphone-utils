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
#include <math.h>

#include <glib.h>

#include "phone-utils.h"
#include "str_utils.h"


static char *international_prefix = NULL;
static int international_prefix_len = 0;
static char *national_prefix = NULL;
static int national_prefix_len = 0;
static char *country_code = NULL;
static int country_code_len = 0;
static char *area_code = NULL;
static int area_code_len = 0;

static char *trailing_delimiters = "wWpP;,";
static char *possible_chars = "0123456789+";
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
		g_warning (error->message);
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
		g_warning (error->message);
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

/* ************************************ FIXME: split to a different file */
static int gsm_char_size[][2] = {
	{0x0040, 1},	/* #	COMMERCIAL AT */
	{0x00A3, 1},	/* #	POUND SIGN */
	{0x0024, 1},	/* #	DOLLAR SIGN */
	{0x00A5, 1},	/* #	YEN SIGN */
	{0x00E8, 1},	/* #	LATIN SMALL LETTER E WITH GRAVE */
	{0x00E9, 1},	/* #	LATIN SMALL LETTER E WITH ACUTE */
	{0x00F9, 1},	/* #	LATIN SMALL LETTER U WITH GRAVE */
	{0x00EC, 1},	/* #	LATIN SMALL LETTER I WITH GRAVE */
	{0x00F2, 1},	/* #	LATIN SMALL LETTER O WITH GRAVE */
	{0x00E7, 1},	/* #	LATIN SMALL LETTER C WITH CEDILLA */
	{0x000A, 1},	/* #	LINE FEED */
	{0x00D8, 1},	/* #	LATIN CAPITAL LETTER O WITH STROKE */
	{0x00F8, 1},	/* #	LATIN SMALL LETTER O WITH STROKE */
	{0x000D, 1},	/* #	CARRIAGE RETURN */
	{0x00C5, 1},	/* #	LATIN CAPITAL LETTER A WITH RING ABOVE */
	{0x00E5, 1},	/* #	LATIN SMALL LETTER A WITH RING ABOVE */
	{0x0394, 1},	/* #	GREEK CAPITAL LETTER DELTA */
	{0x005F, 1},	/* #	LOW LINE */
	{0x03A6, 1},	/* #	GREEK CAPITAL LETTER PHI */
	{0x0393, 1},	/* #	GREEK CAPITAL LETTER GAMMA */
	{0x039B, 1},	/* #	GREEK CAPITAL LETTER LAMDA */
	{0x03A9, 1},	/* #	GREEK CAPITAL LETTER OMEGA */
	{0x03A0, 1},	/* #	GREEK CAPITAL LETTER PI */
	{0x03A8, 1},	/* #	GREEK CAPITAL LETTER PSI */
	{0x03A3, 1},	/* #	GREEK CAPITAL LETTER SIGMA */
	{0x0398, 1},	/* #	GREEK CAPITAL LETTER THETA */
	{0x039E, 1},	/* #	GREEK CAPITAL LETTER XI */
	{0x00A0, 1},	/* #	ESCAPE TO EXTENSION TABLE (or displayed as NBSP, see note above) */
	{0x000C, 2},	/* #	FORM FEED */
	{0x005E, 2},	/* #	CIRCUMFLEX ACCENT */
	{0x007B, 2},	/* #	LEFT CURLY BRACKET */
	{0x007D, 2},	/* #	RIGHT CURLY BRACKET */
	{0x005C, 2},	/* #	REVERSE SOLIDUS */
	{0x005B, 2},	/* #	LEFT SQUARE BRACKET */
	{0x007E, 2},	/* #	TILDE */
	{0x005D, 2},	/* #	RIGHT SQUARE BRACKET */
	{0x007C, 2},	/* #	VERTICAL LINE */
	{0x20AC, 2},	/* #	EURO SIGN */
	{0x00C6, 1},	/* #	LATIN CAPITAL LETTER AE */
	{0x00E6, 1},	/* #	LATIN SMALL LETTER AE */
	{0x00DF, 1},	/* #	LATIN SMALL LETTER SHARP S (German) */
	{0x00C9, 1},	/* #	LATIN CAPITAL LETTER E WITH ACUTE */
	{0x0020, 1},	/* #	SPACE */
	{0x0021, 1},	/* #	EXCLAMATION MARK */
	{0x0022, 1},	/* #	QUOTATION MARK */
	{0x0023, 1},	/* #	NUMBER SIGN */
	{0x00A4, 1},	/* #	CURRENCY SIGN */
	{0x0025, 1},	/* #	PERCENT SIGN */
	{0x0026, 1},	/* #	AMPERSAND */
	{0x0027, 1},	/* #	APOSTROPHE */
	{0x0028, 1},	/* #	LEFT PARENTHESIS */
	{0x0029, 1},	/* #	RIGHT PARENTHESIS */
	{0x002A, 1},	/* #	ASTERISK */
	{0x002B, 1},	/* #	PLUS SIGN */
	{0x002C, 1},	/* #	COMMA */
	{0x002D, 1},	/* #	HYPHEN-MINUS */
	{0x002E, 1},	/* #	FULL STOP */
	{0x002F, 1},	/* #	SOLIDUS */
	{0x0030, 1},	/* #	DIGIT ZERO */
	{0x0031, 1},	/* #	DIGIT ONE */
	{0x0032, 1},	/* #	DIGIT TWO */
	{0x0033, 1},	/* #	DIGIT THREE */
	{0x0034, 1},	/* #	DIGIT FOUR */
	{0x0035, 1},	/* #	DIGIT FIVE */
	{0x0036, 1},	/* #	DIGIT SIX */
	{0x0037, 1},	/* #	DIGIT SEVEN */
	{0x0038, 1},	/* #	DIGIT EIGHT */
	{0x0039, 1},	/* #	DIGIT NINE */
	{0x003A, 1},	/* #	COLON */
	{0x003B, 1},	/* #	SEMICOLON */
	{0x003C, 1},	/* #	LESS-THAN SIGN */
	{0x003D, 1},	/* #	EQUALS SIGN */
	{0x003E, 1},	/* #	GREATER-THAN SIGN */
	{0x003F, 1},	/* #	QUESTION MARK */
	{0x00A1, 1},	/* #	INVERTED EXCLAMATION MARK */
	{0x0041, 1},	/* #	LATIN CAPITAL LETTER A */
	{0x0042, 1},	/* #	LATIN CAPITAL LETTER B */
	{0x0043, 1},	/* #	LATIN CAPITAL LETTER C */
	{0x0044, 1},	/* #	LATIN CAPITAL LETTER D */
	{0x0045, 1},	/* #	LATIN CAPITAL LETTER E */
	{0x0046, 1},	/* #	LATIN CAPITAL LETTER F */
	{0x0047, 1},	/* #	LATIN CAPITAL LETTER G */
	{0x0048, 1},	/* #	LATIN CAPITAL LETTER H */
	{0x0049, 1},	/* #	LATIN CAPITAL LETTER I */
	{0x004A, 1},	/* #	LATIN CAPITAL LETTER J */
	{0x004B, 1},	/* #	LATIN CAPITAL LETTER K */
	{0x004C, 1},	/* #	LATIN CAPITAL LETTER L */
	{0x004D, 1},	/* #	LATIN CAPITAL LETTER M */
	{0x004E, 1},	/* #	LATIN CAPITAL LETTER N */
	{0x004F, 1},	/* #	LATIN CAPITAL LETTER O */
	{0x0050, 1},	/* #	LATIN CAPITAL LETTER P */
	{0x0051, 1},	/* #	LATIN CAPITAL LETTER Q */
	{0x0052, 1},	/* #	LATIN CAPITAL LETTER R */
	{0x0053, 1},	/* #	LATIN CAPITAL LETTER S */
	{0x0054, 1},	/* #	LATIN CAPITAL LETTER T */
	{0x0055, 1},	/* #	LATIN CAPITAL LETTER U */
	{0x0056, 1},	/* #	LATIN CAPITAL LETTER V */
	{0x0057, 1},	/* #	LATIN CAPITAL LETTER W */
	{0x0058, 1},	/* #	LATIN CAPITAL LETTER X */
	{0x0059, 1},	/* #	LATIN CAPITAL LETTER Y */
	{0x005A, 1},	/* #	LATIN CAPITAL LETTER Z */
	{0x00C4, 1},	/* #	LATIN CAPITAL LETTER A WITH DIAERESIS */
	{0x00D6, 1},	/* #	LATIN CAPITAL LETTER O WITH DIAERESIS */
	{0x00D1, 1},	/* #	LATIN CAPITAL LETTER N WITH TILDE */
	{0x00DC, 1},	/* #	LATIN CAPITAL LETTER U WITH DIAERESIS */
	{0x00A7, 1},	/* #	SECTION SIGN */
	{0x00BF, 1},	/* #	INVERTED QUESTION MARK */
	{0x0061, 1},	/* #	LATIN SMALL LETTER A */
	{0x0062, 1},	/* #	LATIN SMALL LETTER B */
	{0x0063, 1},	/* #	LATIN SMALL LETTER C */
	{0x0064, 1},	/* #	LATIN SMALL LETTER D */
	{0x0065, 1},	/* #	LATIN SMALL LETTER E */
	{0x0066, 1},	/* #	LATIN SMALL LETTER F */
	{0x0067, 1},	/* #	LATIN SMALL LETTER G */
	{0x0068, 1},	/* #	LATIN SMALL LETTER H */
	{0x0069, 1},	/* #	LATIN SMALL LETTER I */
	{0x006A, 1},	/* #	LATIN SMALL LETTER J */
	{0x006B, 1},	/* #	LATIN SMALL LETTER K */
	{0x006C, 1},	/* #	LATIN SMALL LETTER L */
	{0x006D, 1},	/* #	LATIN SMALL LETTER M */
	{0x006E, 1},	/* #	LATIN SMALL LETTER N */
	{0x006F, 1},	/* #	LATIN SMALL LETTER O */
	{0x0070, 1},	/* #	LATIN SMALL LETTER P */
	{0x0071, 1},	/* #	LATIN SMALL LETTER Q */
	{0x0072, 1},	/* #	LATIN SMALL LETTER R */
	{0x0073, 1},	/* #	LATIN SMALL LETTER S */
	{0x0074, 1},	/* #	LATIN SMALL LETTER T */
	{0x0075, 1},	/* #	LATIN SMALL LETTER U */
	{0x0076, 1},	/* #	LATIN SMALL LETTER V */
	{0x0077, 1},	/* #	LATIN SMALL LETTER W */
	{0x0078, 1},	/* #	LATIN SMALL LETTER X */
	{0x0079, 1},	/* #	LATIN SMALL LETTER Y */
	{0x007A, 1},	/* #	LATIN SMALL LETTER Z */
	{0x00E4, 1},	/* #	LATIN SMALL LETTER A WITH DIAERESIS */
	{0x00F6, 1},	/* #	LATIN SMALL LETTER O WITH DIAERESIS */
	{0x00F1, 1},	/* #	LATIN SMALL LETTER N WITH TILDE */
	{0x00FC, 1},	/* #	LATIN SMALL LETTER U WITH DIAERESIS */
	{0x00E0, 1},	/* #	LATIN SMALL LETTER A WITH GRAVE */
	{0     , 0}	/* END */
	};

static int
utf8_get_next(const char *buf, int *iindex);

/* returns the char size in bytes (in gsm encoding) of the passed unicode char */
int
phone_utils_gsm_get_char_size(int chr)
{
	int j = 0;
	while (gsm_char_size[j][1] && gsm_char_size[j][0] != chr) {
		j++;
	}
	if (gsm_char_size[j][1]) { /*this means it's in the table, count it in size. */
		return gsm_char_size[j][1];
	}

	return 0;
}

/* returns true if the function will have to be encoded in ucs */
int
phone_utils_gsm_is_ucs(const char *string)
{
	int i;
	int chr;

	i=0;
	chr = utf8_get_next(string, &i);
	
	for ( ; chr ; chr = utf8_get_next(string, &i)) {
		if (!phone_utils_gsm_get_char_size(chr)) {
			return 1;
		}		
	}

	return 0;
}

/* string is assumed to be in utf8 */
int
phone_utils_gsm_sms_strlen(const char *string)
{
	int i;
	int len, size;
	int ucs;
	int chr;

	i=0;
	chr = utf8_get_next(string, &i);
	
	for (len = size = ucs = 0 ; chr ; chr = utf8_get_next(string, &i), len++) {
		
		int char_size;

		char_size = phone_utils_gsm_get_char_size(chr);
		if (char_size) {
			size += char_size;
		}
		else {
			ucs = 1;
		}		
	}

	return (ucs) ? len : size;	
}

/* Many thanks to evas */
static int
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

char **
phone_utils_gsm_sms_split_message(const char *message, int len, int ucs)
{
	char **messages;
	int limit = 1; /* just a safe value so it won't crash */
	int number_of_messages;
	int i, j;
	int start, end;
	
	if (ucs) {
		limit = PHONE_UTILS_GSM_SMS_UCS_LIMIT; /* ucs-2 number of chars limit */
		if (len > limit) {
			limit = PHONE_UTILS_GSM_SMS_UCS_SPLIT_LIMIT;
		}
	}
	else {
		limit = PHONE_UTILS_GSM_SMS_TEXT_LIMIT; /* regular number of chars limit */
		if (len > limit) {
			limit  = PHONE_UTILS_GSM_SMS_TEXT_SPLIT_LIMIT;
		}
	}

	number_of_messages = (int) ceilf((float) len / limit);
	messages = calloc(number_of_messages + 1, sizeof(char *));
	if (!messages) {
		goto end;
	}
	
	/* copy the messages */
	start = 0;
	for (i = 0 ; i < number_of_messages ; i++) {
		/* we want to go "limit" chars */
		for (j = 0, end = start ; j < limit ; j++) {
			utf8_get_next(message, &end);
		}
		messages[i] = malloc((end - start)  + 1); /* the actual utf8 len + 1 for null */
		if (!messages[i]) {
			goto clean_messages;
		}

		/* copy that part of the string */
		strncpy(messages[i], &message[start], end - start);
		messages[i][end - start] = '\0';

		start = end; /* advance start */
	}
	messages[i] = NULL; /* terminate the list with a null */
	i--; /* make i point to the last real message */

	/* reduce the side of the last message to the real size needed */
	if (len % limit) {
		messages[i] = realloc(messages[i], (len % limit) + 1);
	}
	
end:
	return messages;

/* error handling */
clean_messages:
	if (messages) {
		for (i = 0 ; i < number_of_messages ; i++) {
			if (messages[i]) {
				free(messages[i]);
			}
		}

		free(messages);
		messages = NULL;
	}
	
	goto end;
}