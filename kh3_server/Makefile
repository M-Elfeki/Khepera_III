#
# libkorebot utils Makefile
#
#
.PHONY: clean

# Pointer to the libkorebot build directory
LIBKOREBOT = ${LIBKOREBOT_ROOT}/build-korebot-2.6
# Cross compiler
CC = arm-angstrom-linux-gnueabi-gcc

SRCS	= $(wildcard *.c)
OBJS	= $(patsubst %.c,%.o,${SRCS})
INCS	= -I ${LIBKOREBOT}/include
LIBS	= -L ${LIBKOREBOT}/lib -lkorebot
CFLAGS 	= -O2

TARGET	= kh3server

.PHONY: all clean depend

kh3server: kh3server.o 
	@echo "Building $@"
	@$(CC) -o $@ $? $(LIBS) $(INCS) $(CFLAGS)

kh3server-static: kh3server.o
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
