
pkglib_LTLIBRARIES=srv_allow2.la

AM_CPPFLAGS=-I$(top_srcdir)/ -I$(top_srcdir)/include/ -I$(top_builddir)/include/

srv_echo_la_LIBADD = @MODULES_LIBADD@
srv_echo_la_CFLAGS=  @MODULES_CFLAGS@
srv_echo_la_LDFLAGS= -module -avoid-version
srv_echo_la_SOURCES = srv_allow2.c


EXTRA_DIST= makefile.w32 srv_allow2.def