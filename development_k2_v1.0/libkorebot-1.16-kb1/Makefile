#---------------------------------------------------------------------
# Makefile - Kore Library
#---------------------------------------------------------------------
# $Id: Makefile,v 1.17 2006/09/11 08:05:01 pbureau Exp $
#---------------------------------------------------------------------
# $Author: pbureau $
# $Date: 2006/09/11 08:05:01 $
# $Revision: 1.17 $
#---------------------------------------------------------------------
# $Log: Makefile,v $
# Revision 1.17  2006/09/11 08:05:01  pbureau
# changed LIBVER to 1.9
#
# Revision 1.16  2006/03/07 17:39:38  pbureau
# Makefile update for clean x86 compiling
#
# Revision 1.15  2006/02/02 00:35:53  bberger
# *** empty log message ***
#
# Revision 1.14  2005/02/05 17:31:43  pbureau_local
# *** empty log message ***
#
# Revision 1.13  2005/01/26 08:45:07  amaye
# Added rules for mpeg compilation
#
# Revision 1.12  2005/01/20 08:41:24  pbureau
# Release 1.8
#
# Revision 1.11  2004/11/12 14:34:53  pbureau
# installation instructions
#
# Revision 1.10  2004/11/01 13:53:36  pbureau
# Updated doxy files.
#
# Revision 1.9  2004/10/14 08:23:41  pbureau
# Adapted for release
#
# Revision 1.8  2004/09/07 21:17:51  cgaudin
# Corrected a conflict in Makefile.
#
# Revision 1.7  2004/09/05 14:05:10  pbureau
# updated src/tests with new test programs
#
# Revision 1.6  2004/07/29 10:51:44  cgaudin
# New libkorebot release 1.2
#
#---------------------------------------------------------------------

# Please read README.kteam for more information about libkorebot 
# installation procedure.

# The shell to use for scripts
SHELL		= /bin/sh

# Specify the target system. The corresponding sytem specify
# section bellow should be edited as well.
TARGET_SYSTEM	= korebot-2.6
#TARGET_SYSTEM	= korebot
#TARGET_SYSTEM	= x86

# libkorebot configuration files directory
KB_CONFIG_DIRECTORY=/etc/libkorebot

KB_FLAGS = -DKB_CONFIG_DIRECTORY=\"$(KB_CONFIG_DIRECTORY)\"

#---------------------------------------------------------------------
# Default section (use TARGET variable) if you want to use gcc
#---------------------------------------------------------------------

ifneq (,$(findstring korebot,$(TARGET_SYSTEM)))

CC		= gcc
LD		= ld
AR		= ar

# Edit these to point on your target system include directory if necessary
KOREBOT_TOOLS 	=
SYS_INCLUDES 	= 

endif

#---------------------------------------------------------------------
# KoreBot specific section (use TARGET variable)
#---------------------------------------------------------------------

ifeq ($(TARGET_SYSTEM),korebot-2.6)

CC		= arm-angstrom-linux-gnueabi-gcc
LD		= arm-angstrom-linux-gnueabi-ld
AR		= arm-angstrom-linux-gnueabi-ar
AS		= arm-angstrom-linux-gnueabi-as

else

CC		= arm-linux-gcc
LD		= arm-linux-ld
AR		= arm-linux-ar
AS		= arm-linux-as

# This variable must point to your korebot-tools directory 
KOREBOT_TOOLS	= /usr/local/korebot-tools-2.1

SYS_INCLUDES	=  -I ${KOREBOT_TOOLS}/include 
SYS_INCLUDES 	+= -I ${KOREBOT_TOOLS}/arm-linux/include
SYS_INCLUDES  	+= -I ${KOREBOT_TOOLS}/arm-linux/sys-include
SYS_INCLUDES 	+= -I ${KOREBOT_TOOLS}/arm-linux/lib/gcc-lib/arm-linux/2.95.3/include

endif 

#---------------------------------------------------------------------

LIBVER		= 1.16
LIBNAME		= libkorebot

SRCS		= $(wildcard *.c)
OBJS		= $(patsubst %.c,%.o,${SRCS})

.PHONY: all clean depend doc docs

# Exports to submakes
export SHELL CC AR LD SYS_INCLUDES AS
export KOREBOT_TOOLS LIBNAME LIBVER TARGET_SYSTEM
export KB_FLAGS

#
# Rules
#
all: 	
	@cd src && make -w 

doc: docs
docs:
	@cd src && make -w docs

clean: 
	@cd src && make -w clean

depend:	
	@cd src && make -w depend

