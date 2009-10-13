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

#ifndef _PHONE_UTILS_H
#define _PHONE_UTILS_H

/* sms stuff */
#define PHONE_UTILS_GSM_SMS_TEXT_LIMIT		160
#define PHONE_UTILS_GSM_SMS_TEXT_SPLIT_LIMIT	153
#define PHONE_UTILS_GSM_SMS_UCS_LIMIT		70
#define PHONE_UTILS_GSM_SMS_UCS_SPLIT_LIMIT	67

/* string is assumed to be in utf8 */
int
phone_utils_gsm_sms_strlen(const char *string);

/* returns true if the function will have to be encoded in ucs */
int
phone_utils_gsm_is_ucs(const char *string);

/* returns the char size in bytes (in gsm encoding) of the passed unicode char */
int
phone_utils_gsm_get_char_size(int chr);

/* takes a message and splits it to buffers that can be used
 * for sending split messages. pdu is true if should split as ucs.
 * len is the len of the message (got from phone_utils_gsm_sms_strlen)
 * the function allocates the array and the strings inside
 * the array (as every string inside it) is null terminated */
char **
phone_utils_gsm_sms_split_message(const char *message, int len, int ucs);

#endif
