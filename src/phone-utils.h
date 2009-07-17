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

#ifndef _PHONE_UTILS_H
#define _PHONE_UTILS_H
#include <glib.h>

#define PHONE_UTILS_CONFIG "/usr/etc/phone-utils.conf"

void 
phone_utils_init();

void
phone_utils_deinit();

void 
phone_utils_set_codes(char *_international_prefix, char *_national_prefix, 
                      char *_country_code, char *_home_code);

/*void 
phone_utils_set_delimiters(char *trailing_delimiters, char *possible_chars, 
                       char *filler_chars) */

void 
phone_utils_init_from_file(const char *filename) ;

const char *
phone_utils_get_user_international_prefix() ;

const char *
phone_utils_get_user_national_prefix() ;

const char *
phone_utils_get_user_country_code();

const char * 
phone_utils_get_user_home_code() ;

int
phone_utils_set_user_international_prefix(const char *value) ;

int
phone_utils_set_user_national_prefix(const char *value) ;

int
phone_utils_set_user_country_code(const char *value);

int
phone_utils_set_user_home_code(const char *value) ;

char *
phone_utils_normalize_number(const char *_number);

int 
phone_utils_numbers_equal(const char * _a, const char * _b);



#endif
