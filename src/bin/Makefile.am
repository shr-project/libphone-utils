MAINTAINERCLEANFILES = Makefile.in

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DPKGDATADIR=\"$(pkgdatadir)\" \
	      -I$(top_srcdir)/src/lib @GLIB_CFLAGS@ -lm 
AM_CFLAGS = -Wall -Wextra -Werror

bin_PROGRAMS = phoneutils_test
phoneutils_test_SOURCES = phoneutils_test.c

phoneutils_test_LDADD = $(top_builddir)/src/lib/libphone-utils.la

phoneutils_test_LDFLAGS = 

