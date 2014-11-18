#
# libkorebot utils Makefile
#
#
.PHONY: clean

# Pointer to the libkorebot build directory
LIBKOREBOT = ../build-korebot-2.6
# Cross compiler
CC = arm-angstrom-linux-gnueabi-gcc

SRCS	= $(wildcard *.c)
OBJS	= $(patsubst %.c,%.o,${SRCS})
INCS	= -I ${LIBKOREBOT}/include -I ..
LIBS	= -L ${LIBKOREBOT}/lib -lkorebot
CFLAGS 	= 

TARGET	= generated generated-static

.PHONY: all clean depend

generated: generated.o 
	@echo "Building $@"
	@$(CC) -o $@ $? $(LIBS) $(INCS) $(CFLAGS)

generated-static: generated.o
	@echo "Building $@"
	@$(CC) -o $@ $? $(LIBS) -static $(INCS) $(CFLAGS)

all: 	${TARGET}

clean : 
	@echo "Cleaning"
	@rm -f *.o .depend ${TARGET} *~

depend:	
	@echo "Building dependencies"
	@rm -f .depend
	@touch .depend
	@makedepend ${SYS_INCLUDES} ${INCS} -Y -f .depend ${SRCS}

%.o:	%.c
	@echo "Compiling $@"
	$(CC) $(INCS) -c $(CFLAGS) $< -o $@

ifeq (.depend,$(wildcard .depend))
include .depend 
endif
