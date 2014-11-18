#--------------------------------------------------------------------
# Makefile - Kore Library
#---------------------------------------------------------------------
# $Id: Makefile,v 1.12 2006/10/27 09:19:13 flambercy Exp $
#---------------------------------------------------------------------
# $Author: flambercy $
# $Date: 2006/10/27 09:19:13 $
# $Revision: 1.12 $
#---------------------------------------------------------------------
# $Log: Makefile,v $
# Revision 1.12  2006/10/27 09:19:13  flambercy
# Makefile update
#
# Revision 1.11  2006/08/18 12:32:51  flambercy
# *** empty log message ***
#
# Revision 1.10  2006/04/08 15:54:25  pbureau
# Completed koala.c command file
# Corrected x86 compilation problem
#
# Revision 1.9  2006/03/07 17:39:57  pbureau
# Makefile update for clean x86 compiling
# i2c compilation cleaned for 2.6 kernel
#
# Revision 1.8  2005/04/28 16:33:44  pbureau
# kb_sound update
#
# Revision 1.7  2005/02/05 17:31:45  pbureau_local
# *** empty log message ***
#
# Revision 1.6  2005/01/30 16:57:52  pbureau
# Using O2 optimizazion by default
#
# Revision 1.5  2005/01/26 08:45:28  amaye
# added mp3 support files
#
# Revision 1.4  2004/11/01 13:53:43  pbureau
# Updated doxy files.
#
# Revision 1.3  2004/09/05 14:05:13  pbureau
# updated src/tests with new test programs
#
# Revision 1.2  2004/08/16 20:05:22  cgaudin
# Corrected bugs in kmot.c
#
# Revision 1.1  2004/07/29 10:51:54  cgaudin
# New libkorebot release 1.2
#
#---------------------------------------------------------------------

SRCS		= $(wildcard *.c)
OBJS		= $(patsubst %.c,%.o,${SRCS})

ifeq ($(TARGET_SYSTEM),korebot)
OBJS		+= mpeg/kb_mp3.o
OBJS		+= mpeg/layer3/mpeg-l3.o
endif

BUILD		= ../build-${TARGET_SYSTEM}
CFLAGS 		= -O2
LIBS		= -lm 

export SHELL CC AR LD SYS_INCLDES BUILD AS
export KOREBOT_TOOLS LIBNAME LIBVER TARGET_SYSTEM
export KB_FLAGS

#---------------------------------------------------------------------
# Rules
#---------------------------------------------------------------------
all: 	${LIBNAME}
	@cd tests && make -w
	@cd utils && make -w
	@cd spca  && make -w
 


mpeg/layer3/mpeg-l3.o:	 
	@cd mpeg/layer3 && make -w


doc: docs
docs:
	doxygen

${LIBNAME}: ${OBJS}
	@echo "Targetting KoreBot Library for ${TARGET_SYSTEM}"
	@echo "Building ${LIBNAME}-${LIBVER}.so"
	@mkdir -p ${BUILD}/lib
	@mkdir -p ${BUILD}/include/korebot
	$(CC) -o ${BUILD}/lib/${LIBNAME}-${LIBVER}.so -shared $(LIBS) ${OBJS}
	@echo "Building ${LIBNAME}.a"
	@$(AR) r ${BUILD}/lib/${LIBNAME}.a ${OBJS}
	@echo "Building ${LIBNAME}-${LIBVER}.so"
	@rm -f ${BUILD}/lib/${LIBNAME}.so
	@ln -s ${LIBNAME}-${LIBVER}.so ${BUILD}/lib/${LIBNAME}.so
	@echo "Adding includes"
	@cp *.h ${BUILD}/include/korebot

clean: 
	@echo "Cleaning"
	@rm -f *.o *~ .depend core*
	@cd tests && make -w clean
	@cd utils && make -w clean
	@cd spca  && make -w clean
	@rm -f mpeg/kb_mp3.o
	@cd mpeg/layer3 && make -w clean

depend:	
	@echo "Building dependencies"
	@rm -f .depend
	@touch .depend
	@makedepend ${SYS_INCLUDES} -Y -f .depend ${SRCS}
	@cd tests && make -w depend
	@cd utils && make -w depend
	@cd spca  && make -w depend
	#@cd mpeg/layer3 && make -w clean

%.o: %.c %.h
	@echo "Building $@"
	@$(CC) -c $(CFLAGS) $(KB_FLAGS) $< -o $@

.PHONY: all clean depend docs

ifeq (.depend,$(wildcard .depend))
include .depend 
endif

