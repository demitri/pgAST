# name of extension (there must be a $EXTENSION.control file)
EXTENSION = pgast

# random files to install into $PREFIX/share/$MODULEDIR
DATA = pgast--1.0.0.sql     # script files to install

PGFILEDESC = "pgast - Starlink AST library extension for PostgreSQL"

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

# Include AST library
# -------------------
# flags to add to CPPFLAGS
ifndef AST
$(error Please set the environment variable 'AST' to an installation of the Starlink AST library.)
endif
#AST      = /usr/custom/ast
AST_LIB  = $(AST)/lib

# Adding '-Wno-format-zero-length' since AST can take empty strings as parameters
# Adding '-Wno-declaration-after-statement' since -std=c99 doesn't seem to turn on C99 on Linux
#
PG_CPPFLAGS += -I$(AST)/include -std=c99 -Wall -Wno-declaration-after-statement
# '-Wno-format-zero-length'

# CFLAGS_SL
# ---------
# (SL=shared library)
# Use this for a project of type "MODULES".
# This line is added to the cc linker.
#CFLAGS_SL = $(AST_LIB)/libast.a $(AST_LIB)/libast_grf3d.a $(AST_LIB)/libast_err.a  $(AST_LIB)/libast_grf_2.0.a $(AST_LIB)/libast_grf_3.2.a $(AST_LIB)/libast_grf_5.6.a $(AST_LIB)/libast_pal.a

# SHLIB_LINK
# ----------
# Use this for a project of type "MODULE_big".
# This line is added to the cc linker.
SHLIB_LINK = $(AST_LIB)/libast.a $(AST_LIB)/libast_grf3d.a $(AST_LIB)/libast_err.a  $(AST_LIB)/libast_grf_2.0.a $(AST_LIB)/libast_grf_3.2.a $(AST_LIB)/libast_grf_5.6.a $(AST_LIB)/libast_pal.a
#SHLIB_LINK += $(filter -last_pass2 -last, $(LIBS))

# PG_LIBS
# -------
# Use this for a project of type "PROGRAM".
# This line is added to the cc linker.
# PG_LIBS = somelib.a anotherlib.a

OBJS = pgast_pgast.o pgast_header2polygon.o pgast_bounding_circle.o pgast_getregiondisc.o pgast_util.o pgast_bounding_polygon.o pgast_region_overlaps.o pgast_cutout.o

# -----------------------------------------------------

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# use local copy for testing
#include ./pgxs.mk
