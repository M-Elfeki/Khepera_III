#---------------------------------------------------------------------
# Makefile - Kore Library
#---------------------------------------------------------------------
# $Id: Makefile,v 1.9 2006/03/07 17:39:57 pbureau Exp $
#---------------------------------------------------------------------
# $Author: pbureau $
# $Date: 2006/03/07 17:39:57 $
# $Revision: 1.9 $
#---------------------------------------------------------------------
# $Log: Makefile,v $
#---------------------------------------------------------------------

SRCS		= $(wildcard *.c)
OBJS		= $(patsubst %.c,%.o,${SRCS})

BUILD		= ../build-${TARGET_SYSTEM}
CFLAGS 		= -O2
LIBS		= -lm -lpthread
SPCA_TARGET	= spcatest

export SHELL CC AR LD SYS_INCLDES BUILD AS
export KOREBOT_TOOLS LIBNAME LIBVER TARGET_SYSTEM
export KB_FLAGS

#---------------------------------------------------------------------
# Rules
#---------------------------------------------------------------------
all: 	${SPCA_TARGET}

doc: docs
docs:
	doxygen


${SPCA_TARGET}: ${OBJS}
	@echo "Targetting KoreBot Library for ${TARGET_SYSTEM}"
	@echo "Building ${SPCA_TARGET}"
	$(CC) -o ${SPCA_TARGET} ${LIBS} ${OBJS}

clean: 
	@echo "Cleaning"
	@rm -f *.o *~ .depend core*

depend:	
	@echo "Building dependencies"
	@rm -f .depend
	@touch .depend
	@makedepend ${SYS_INCLUDES} -Y -f .depend ${SRCS}
	#@cd mpeg/layer3 && make -w clean

%.o: %.c %.h
	@echo "Building $@"
	@$(CC) -c $(CFLAGS) $(KB_FLAGS) $< -o $@

.PHONY: all clean depend docs

ifeq (.depend,$(wildcard .depend))
include .depend 
endif
