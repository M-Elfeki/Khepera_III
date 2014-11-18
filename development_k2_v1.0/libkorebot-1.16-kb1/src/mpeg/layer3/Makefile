#---------------------------------------------------------------------
# Makefile - Makefile for mp3 stuff
#---------------------------------------------------------------------
# $Id: Makefile,v 1.3 2006/03/07 17:40:25 pbureau Exp $
#---------------------------------------------------------------------
# Original Author: , 2004
# $Author: pbureau $
# $Revision: 1.3 $
# $Date: 2006/03/07 17:40:25 $
#---------------------------------------------------------------------
# $Log: Makefile,v $
# Revision 1.3  2006/03/07 17:40:25  pbureau
# Makefile update for clean x86 compiling
# i2c compilation cleaned for 2.6 kernel
# kb_mp3 moved in src/mpeg
#
# Revision 1.2  2005/02/05 17:31:47  pbureau_local
# *** empty log message ***
#
# Revision 1.1  2005/01/26 09:15:32  amaye
# *** empty log message ***
#
# Revision 0.1  2004/08/27 20:13:44  cgaudin
#
#---------------------------------------------------------------------

ifeq ($(BUILD), )
BUILD = ../build-korebot
endif

ifeq ($(CC), cc)
CC = arm-linux-gcc
AS = arm-linux-as
AR = arm-linux-ar
ARMFLAGS = -mcpu=xscale -mfpu=fpa
endif

INCS	= -I ${BUILD}/include -I ..
LIBS	= -L ${BUILD}/lib -lkorebot
CFLAGS	= 

OBJ		= tables.o layer3.o l3subband.o l3mdct.o tables.o \
		  l3loop.o formatbits.o bitstream.o l3bitstrea.o huffman.o reservoir.o mult.o

# general
whole: ${OBJ}
	@echo "Building with $@"
	@$(AR) rcs mpeg-l3.o $(OBJ)


mp3enc:	$(OBJ)
	@echo "Building $@"
	@$(CC) -o $@ $? -lm $(LIBS) $(INCS) $(CFLAGS)


mp3enc-static:	$(OBJ)
	@echo "Building $@"
	@$(CC) -o $@ $? -lm -static $(LIBS) $(INCS) $(CFLAGS)

mult.o:   mult.S
	@echo "assembling $@, required multiplication library..."
	@$(AS) -o $@ $? $(ARMFLAGS)



.PHONY: depend
depend:	
	@rm -f .depend
	@touch .depend
	@makedepend ${SYS_INCLUDES} ${INCLUDES} -Y -f .depend ${SRCS}

.PHONY: clean
clean:
	@rm -f *.o *~ .depend

ifeq (.depend,$(wildcard .depend))
include .depend 
endif
