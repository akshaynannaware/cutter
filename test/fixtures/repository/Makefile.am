AM_CPPFLAGS = $(CUTTER_CFLAGS)	\
	-I$(top_builddir)	\
	-I$(top_srcdir)		\
	-I$(top_srcdir)/cutter

check_LTLIBRARIES = 				\
	repository-test-case1.la		\
	repository-test-case2.la		\
	repository-test-case3.la

AM_LDFLAGS =			\
	-module			\
	-rpath $(libdir)	\
	-avoid-version		\
	-no-undefined

LIBS =						\
	$(top_builddir)/cutter/libcutter.la	\
	$(GLIB_LIBS)

repository_test_case1_la_SOURCES = repository-test-case1.c
repository_test_case2_la_SOURCES = repository-test-case2.c
repository_test_case3_la_SOURCES = repository-test-case3.c

echo-tests:
	@echo $(noinst_LTLIBRARIES)
