# name of extension (there must be a $EXTENSION.control file)
EXTENSION = pgast

# random files to install into $PREFIX/share/$MODULEDIR
DATA = pgast--1.0.0.sql     # script files to install

PGFILEDESC = "pgast - Starlink AST library from PostgreSQL"

# REGRESS = pgast_test      # test script file (without extension)

#  list of shared-library objects to be built from source files
#MODULES = pgast            # C module file to build (i.e. pgast.o)
#MODULE_big = pgast			# if building source from multiple files

# prefix of source files
MODULE_big = pgast

# 'patsubst' is a make command; inserts files that match the pattern
#       where '%' is the wildcard, e.g. $(patsubst %.c)
# https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
#
# MODULES = $(patsubst %.c,%o)

#MODULE_big = last

# David's suggestion: omit grf3d_pgplot.c, retain: grf3d.c

#CFLAGS = "-std=c99" --> didn't seem to work

# Include AST library
# -------------------
# flags to add to CPPFLAGS
AST = /usr/custom/ast-8.6.2

#PG_CPPFLAGS += -I/usr/local/ast-8.6.2/include
PG_CPPFLAGS += -I$(AST)/include 
# -static -L.

#PG_LIBS = -last 

# CFLAGS_SL
# ---------
# (SL=shared library)
# Added to CFLAGS for MODULES type
#CFLAGS_SL = -L/usr/local/ast-8.6.2/lib -last -last_drama -last_ems -last_err -last_grf_2.0 -last_grf_3.2 -last_grf_5.6 -last_pal -last_grf3d
AST_LIB = $(AST)/lib
#CFLAGS_SL = $(AST_LIB)/libast.a $(AST_LIB)/libast_grf3d.a $(AST_LIB)/libast_err.a  $(AST_LIB)/libast_grf_2.0.a $(AST_LIB)/libast_grf_3.2.a $(AST_LIB)/libast_grf_5.6.a $(AST_LIB)/libast_pal.a
#LIBS = -static -L$(AST_LIB) $(AST_LIB)/libast.a $(AST_LIB)/libast_grf3d.a $(AST_LIB)/libast_err.a  $(AST_LIB)/libast_grf_2.0.a $(AST_LIB)/libast_grf_3.2.a $(AST_LIB)/libast_grf_5.6.a $(AST_LIB)/libast_pal.a
#CFLAGS_SL = -static -L$(AST_LIB) $(AST_LIB)/libast.a $(AST_LIB)/libast_grf3d.a $(AST_LIB)/libast_err.a  $(AST_LIB)/libast_grf_2.0.a $(AST_LIB)/libast_grf_3.2.a $(AST_LIB)/libast_grf_5.6.a $(AST_LIB)/libast_pal.a
SHLIB_LINK = $(AST_LIB)/libast.a $(AST_LIB)/libast_grf3d.a $(AST_LIB)/libast_err.a  $(AST_LIB)/libast_grf_2.0.a $(AST_LIB)/libast_grf_3.2.a $(AST_LIB)/libast_grf_5.6.a $(AST_LIB)/libast_pal.a
#$(AST_LIB)/libast_drama.a $(AST_LIB)/libast_ems.a
# -last_pgplot3d
#CFLAGS_SL = -static -L. -L/usr/local/ast-8.6.2/lib


# SHLIB_LINK
# ----------
# Added to MODULE_big link line
#SHLIB_LINK += -last -last_drama -last_ems -last_err  -last_grf_2.0 -last_grf_3.2 -last_grf_5.6 -last_pal -last_grf3d
#-last_pgplot3d
# CFLAGS_SL doesn't seem to be the designated place to add the library
# flags, but it works here.

#PG_CPPFLAGS += -L/usr/local/ast-8.6.2/lib
#SHLIB_LINK = -L/usr/local/ast-8.6.2/lib -last
#PG_LIBS = -L/usr/local/ast-8.6.2/lib -last

#CFLAGS = -std=c99
#CXXFLAGS = -std=c99
#CXXFLAGS = -L/usr/local/ast-8.6.2/lib -I/usr/local/ast-8.6.2/include
#LIBS = libast.a -last

#LIBS = libast.a libast_drama.a

# Added to PROGRAM link line (not MODULES) - not used here
#PG_LIBS = libast.a libast_drama.a libast_ems.a libast_err.a libast_grf_2.0.a libast_grf_3.2.a libast_grf_5.6.a libast_pal.a libast_pgplot3d.a

#SHLIB_LINK += $(filter -last_pass2 -last, $(LIBS))

#SHLIB_LINK += -L/usr/local/ast-8.6.2/lib 
#SHLIB_LINK += -L.
OBJS = _pgast_pgast.o _pgast_header2polygon.o

# -----------------------------------------------------

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# use local copy for testing
#include ./pgxs.mk
