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

#ifndef _PHONE_UTILS_GSM_H
#define _PHONE_UTILS_GSM_H

/* sms stuff */
#define PHONE_UTILS_GSM_SMS_TEXT_LIMIT		160
#define PHONE_UTILS_GSM_SMS_TEXT_SPLIT_LIMIT	153
#define PHONE_UTILS_GSM_SMS_UCS_LIMIT		70
#define PHONE_UTILS_GSM_SMS_UCS_SPLIT_LIMIT	67

/* Receives a utf8 encoded string and returns the string len of this string
 * if it'll be sent via an sms. This function detects whether this string
 * should be represented in gsm7 or ucs.
 * If it should be represnetd in gsm7 this function returns the number
 * of bytes needed in order to represent this string (i.e ] is counted as 2)
 * If this string should be sent using the ucs2 encoding the number of chars
 * needed in order to represent this, i.e the equiv of a regular utf8-strlen
 *
 * This function assumes srting is a valid utf8 string*/
int
phone_utils_gsm_sms_strlen(const char *string);

/* Receives a utf8 encoded string and returns true if this string has to
 * be represented in ucs2 or false if it can be representehd in gsm7
 *
 * This function assumes srting is a valid utf8 string*/
int
phone_utils_gsm_is_ucs(const char *string);

/* This function recieves a unicode char and returns the number of bytes
 * this char takes in order to be represented in the gsm 7bit charset
 *
 * returns 0 when this char can not be represented using this charset.
 */
int
phone_utils_gsm_get_char_size(int chr);

/* takes a message and splits it to buffers that can be used
 * for sending split messages. pdu is true if should split as ucs.
 * len is the len of the message (got from phone_utils_gsm_sms_strlen)
 * the function allocates the array and the strings inside
 * the array (as every string inside it) is null terminated */
char **
phone_utils_gsm_sms_split_message(const char *message, int len, int ucs);

int
phone_utils_gsm_number_is_ussd(const char *number);

#endif
