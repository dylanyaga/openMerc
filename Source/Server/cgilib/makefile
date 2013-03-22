#
# Author: Noel V Aguilar
# CGI-LIB Library
# Release 1.4
#
# This library is free softare; you can redistribute it
# and/or modify it.
#
# You can compile this library by simply running 'make'
# and then copy the cgi-lib.a to the your lib directory
# and the cgi-lib.h and html-lib.h files into your 
# include directory manually.  This will allow the linker
# to automatically include the library or you can copy the
# files to where your development is being done and link it
# manually.
#
# For updates or to report bugs please go to:
# http://www.geocities.com/SiliconValley/Vista/6493/
#
# This library is distributed in the hope that it will be
# useful but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.
#

CGI-LIB=cgi-lib.a
CC=gcc
CCFLAGS=-c
AR=ar
ARFLAGS=rc
OBJS=cgi-lib.o html-lib.o list-lib.o

all: $(CGI-LIB)

$(CGI-LIB): $(OBJS)
	@echo Building library
	$(AR) $(ARFLAGS) $@ $(OBJS)
	@echo Building complete

cgi-lib.o: cgi-lib.c cgi-lib.h cgi-priv.h
	$(CC) $(CCFLAGS) cgi-lib.c

html-lib.o: html-lib.c html-lib.h
	$(CC) $(CCFLAGS) html-lib.c

list-lib.o: list-lib.c list-lib.h
	$(CC) $(CCFLAGS) list-lib.c

clean:
	@echo Cleaning up
	rm -f *.o
