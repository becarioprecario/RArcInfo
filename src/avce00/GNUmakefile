# Makefile for the Arc/Info Vector Coverage (AVC) reader 
#
# $Id: GNUmakefile,v 1.1.1.1 2001/06/27 20:10:53 vir Exp $
#
# $Log: GNUmakefile,v $
# Revision 1.1.1.1  2001/06/27 20:10:53  vir
# Initial release (0.1) under the cvs tree at Sourceforge.
#
# Revision 1.1.1.1  2001/06/27 20:04:14  vir
# Initial release (0.1) under the CVS tree.
#
# Revision 1.3  1999/05/11 02:06:54  daniel
# Added coverage write support (alpha version)
#
# Revision 1.2  1999/02/25 05:19:06  daniel
# Added cpl_dir.c
#
# Revision 1.1  1999/01/29 16:29:55  daniel
# Initial revision
#
#

#
#  Default compiler is gcc, 
#
CC      =       gcc

#
# Link with DBMALLOC when running in a test environment
#
# LIB_DBMALLOC= /home/daniel/proj/dbmalloc-14L2/libdbmalloc.a

#
#  Byte ordering ... default is Intel ordering (CPL_LSB),
#  Select CPL_MSB for SUNs or other platforms with MSB first ordering.
#
# BYTE_ORDER_FL = -DCPL_MSB
BYTE_ORDER_FL = -DCPL_LSB

#
#  
#
CFLAGS= $(BYTE_ORDER_FL) -g -Wall -DDEBUG
LFLAGS= 
AR= ar

LIB_OBJS= avc_e00read.o avc_e00write.o avc_rawbin.o avc_bin.o avc_binwr.o \
	  avc_e00gen.o avc_e00parse.o avc_misc.o \
	  cpl_error.o cpl_conv.o cpl_vsisimple.o cpl_string.o cpl_dir.o

LIB=      avc.a

default: $(LIB) avcimport avcexport avctest ex_avcwrite

$(LIB_OBJS):	avc.h cpl_error.h cpl_port.h cpl_conv.h cpl_vsi.h cpl_string.h

avcimport: avcimport.o $(LIB) avc.h
	$(CC) $(LFLAGS) -o avcimport avcimport.o $(LIB)

avcexport: avcexport.o $(LIB) avc.h
	$(CC) $(LFLAGS) -o avcexport avcexport.o $(LIB)

avctest: avctest.o $(LIB) avc.h
	$(CC) $(LFLAGS) -o avctest avctest.o $(LIB)

ex_avcwrite: ex_avcwrite.o $(LIB) avc.h
	$(CC) $(LFLAGS) -o ex_avcwrite ex_avcwrite.o $(LIB)  $(LIB_DBMALLOC)

%.o:	%.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o

%.o:	%.cpp
	$(CC) -c $(CFLAGS) $*.cpp -o $*.o

$(LIB): $(LIB_OBJS)
	rm -f $(LIB)
	$(AR) rc $(LIB) $(LIB_OBJS)

clean:
	rm -f $(LIB) $(LIB_OBJS)
	rm -f avctest.o avctest avcexport.o avcexport avcimport.o avcimport \
	ex_avcwrite ex_avcwrite.o

