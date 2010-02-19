# Copyright (c) 2009 Sebastian Krzyszkowiak
#
# This file is part of python-phoneutils, which is part of libphone-utils.
#
# python-phoneutils is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# python-phoneutils is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.

#
# You should have received a copy of the GNU Lesser General Public License
# along with libphone-utils.  If not, see <http://www.gnu.org/licenses/>.
#

cdef extern from "phone-utils.h":
    int phone_utils_init()
    void phone_utils_deinit()
    int phone_utils_set_codes(char *_international_prefix, char *_national_prefix, char *_country_code, char *_area_code)
    int phone_utils_init_from_file(char *filename)
    int phone_utils_save_config()
    int phone_utils_save_config_to_file(char *filename)
    int phone_utils_is_initialized()
    char * phone_utils_get_user_international_prefix()
    char * phone_utils_get_user_national_prefix()
    char * phone_utils_get_user_country_code()
    char * phone_utils_get_user_area_code()
    int phone_utils_set_user_international_prefix(char *value)
    int phone_utils_set_user_national_prefix(char *value)
    int phone_utils_set_user_country_code(char *value)
    int phone_utils_set_user_area_code(char *value)
    char * phone_utils_normalize_number(char *_number)
    int phone_utils_numbers_equal(char * _a, char * _b)
    int phone_utils_numbers_compare(char * _a, char * _b)

def init():
    phone_utils_init()

def deinit():
    phone_utils_deinit()

def is_initialized():
    if phone_utils_is_initialized():
        return True
    else:
        return False

def set_codes(ip, np, cc, ac):
    phone_utils_set_codes(ip, np, cc, ac)

def init_from_file(filename):
    phone_utils_init_from_file(filename)

def save_config():
    phone_utils_save_config()

def save_config_to_file(filename):
    phone_utils_save_config_to_file(filename)

def get_user_international_prefix():
    return phone_utils_get_user_international_prefix()

def get_user_national_prefix():
    return phone_utils_get_user_national_prefix()

def get_user_country_code():
    return phone_utils_get_user_country_code()

def get_user_area_code():
    return phone_utils_get_user_area_code()

def set_user_international_prefix(value):
    phone_utils_set_user_international_prefix(value)

def set_uset_national_prefix(value):
    phone_utils_set_user_national_prefix(value)

def set_user_country_code(value):
    phone_utils_set_user_country_code(value)

def set_user_area_code(value):
    phone_utils_set_user_area_code(value)

def normalize_number(number):
    return phone_utils_normalize_number(number)

def numbers_equal(a, b):
    if phone_utils_numbers_equal(a, b):
        return True
    else:
        return False

def numbers_compare(a, b):
    return phone_utils_numbers_compare(a, b)
