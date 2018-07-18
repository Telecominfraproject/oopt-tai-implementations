# Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include -I$(srcdir) \
           -I$(srcdir)/../inc \
           -I$(APP_LIB_PATH)/include

# Path to TAI include files
INCLUDES += -I$(top_srcdir)/oopt-tai/inc

if DEBUG
DBGFLAGS = -ggdb -D_DEBUG_
else
DBGFLAGS = -g
endif

CFLAGS = @CFLAGS@ $(CFLAGS_TAI_INTERFACE_COMMON) $(DBGFLAGS)

lib_LTLIBRARIES = libtai.la

libtai_la_SOURCES = \
                       ac400_tai_interface.c \
                       ac400_tai_module.c \
                       ac400_tai_networkif.c \
                       ac400_tai_hostif.c \
                       ac400_tai_utils.c \
                       ac400_voyager.c


libtai_la_LIBADD =

libtai_apiincludedir = $(includedir)/tai
libtai_apiinclude_HEADERS = $(top_srcdir)/oopt-tai/inc/*.h


libtai_api_version=$(shell grep LIBVERSION= $(top_srcdir)/tai_interface.ver | sed 's/LIBVERSION=//')
libtai_la_LDFLAGS = -version-info $(libtai_api_version)
