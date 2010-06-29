MAINTAINERCLEANFILES = Makefile.in

localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -Wextra -Werror @GLIB_CFLAGS@ @LIBPHONE_UTILS_CFLAGS@

bin_PROGRAMS = phoneutils_test
phoneutils_test_SOURCES = phoneutils_test.c

phoneutils_test_LDFLAGS = @LIBPHONE_UTILS_LIBS@

