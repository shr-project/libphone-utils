MAINTAINERCLEANFILES = Makefile.in

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -Wextra -Werror @GLIB_CFLAGS@ -lm

lib_LTLIBRARIES = libphone-utils.la

libphone_utils_la_SOURCES = phone-utils.c phone-utils.h \
			    str-utils.c str-utils.h \
			    phone-utils-gsm.c phone-utils-gsm.h
libphone_utils_HEADERS = phone-utils.h phone-utils-gsm.h


libphone_utilsdir = $(includedir)/phone-utils

libphone_utils_la_LDFLAGS = $(all_libraries) -ldl

libphone_utils_la_LIBADD = @GLIB_LIBS@ 
