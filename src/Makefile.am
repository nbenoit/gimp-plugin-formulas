# Makefile.am
# Process this file with automake to produce Makefile.in

EXTRA_DIST = \
	symbols4gperf

bin_PROGRAMS = formulas

bindir = $(GIMP_LIBDIR)/plug-ins

formulas_SOURCES = \
	plugin-intl.h	\
	interface.c	\
	interface.h	\
	main.c		\
	main.h		\
	render.c	\
	render.h  \
	error.c   \
	error.h   \
	tokens_table.c \
	tokens_table.h \
	maths_tree.h \
	maths_val.c \
	maths_val.h \
	maths_func.c \
	maths_func.h \
	maths_op.c \
	maths_op.h \
	char_masks.h \
	formula.c \
	formula.h

AM_CPPFLAGS = \
	-DLOCALEDIR=\""$(LOCALEDIR)"\"		\
	-DDATADIR=\""$(DATADIR)"\"

INCLUDES =\
	-I$(top_srcdir)		\
	@GIMP_CFLAGS@		\
	-I$(includedir)

LDADD = $(GIMP_LIBS)

