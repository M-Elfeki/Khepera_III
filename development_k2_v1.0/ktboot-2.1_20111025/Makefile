# KT-boot 2.0

# Makefile for flashing the Korebot2

# K-Team SA, 2011

# -- VERSION --------------------------------------------------------------------------------------

# version 1.1: October 2011: corrected kernel revision
# version 1.0: April 2009: first release


# -- DESCRIPTION ----------------------------------------------------------------------------------

# This is the Makefile for flashing the Korebot 2.
# The differents following commands copy the kernel and filesystem from the built sources if exists,
# or from the current directory then flash them to the board.

# -- USAGE ----------------------------------------------------------------------------------------

# usage: 
#   flash the kernel with            : make flashk
#   flash the image file system with : make flashd
#   set the enironment boot variable : make bootargs

# -- VARIABLES ------------------------------------------------------------------------------------

# path to korebot oetools
OETOOLS	= /usr/local/korebot2-oetools-1.0


# --- kernel variables --------------------------

# kernel version
KTEAM_KERNEL_VERSION = 2.6.24

# kernel  revision
KTEAM_KERNEL_REVISION = r1

# kernel path
KERNEL = ${OETOOLS}/tmp/work/gumstix-custom-verdex-angstrom-linux-gnueabi/gumstix-kernel-$(KTEAM_KERNEL_VERSION)-$(KTEAM_KERNEL_REVISION)/linux-$(KTEAM_KERNEL_VERSION)

# path to kernel binary file
PATH_KERNEL_IMAGE_BIN = ${OETOOLS}/tmp/deploy/glibc/images/gumstix-custom-verdex/uImage-$(KTEAM_KERNEL_VERSION)-$(KTEAM_KERNEL_REVISION)-gumstix-custom-verdex.bin

# Only depend on the kernel if exists
UIMAGE =  $(shell [ -e ${PATH_KERNEL_IMAGE_BIN} ] && echo ${PATH_KERNEL_IMAGE_BIN})


# --- file system -------------------------------

# path to image file system
PATH_IMAGE_FILE_SYSTEM = ${OETOOLS}/tmp/deploy/glibc/images/gumstix-custom-verdex/korebot2-image-gumstix-custom-verdex.jffs2

# Only depend on fs image if it exists
FS_IMAGE = $(shell [ -e ${PATH_IMAGE_FILE_SYSTEM} ] && echo ${PATH_IMAGE_FILE_SYSTEM})


# --- other variables ---------------------------

# Port used (default = serial port)
SDEV = /dev/ttyS0

# boot parameters
BOOTARGS= 'console=ttyS0,115200n8 root=1f01 rootfstype=jffs2 reboot=cold,hard'

SHELL = /bin/bash


# --- flash address depending on Gumstix board version -

# card XLP6
PART_DISTRIB_START = 00040000 
PART_DISTRIB_END   = 01EFFFFF
PART_KERNEL_START  = 01F00000
PART_KERNEL_END    = 01FFFFFF

# card XM4
#PART_DISTRIB_START = 00040000 
#PART_DISTRIB_END   = 00EFFFFF
#PART_KERNEL_START  = 00F00000
#PART_KERNEL_END    = 00FFFFFF


# -- COMMANDS -------------------------------------------------------------------------------------

# make all
all: bootargs flashk flashd


# --- kernel commands ---------------------------

# copy kernel uImage
kt-uImage.bin: ${UIMAGE}
	@rm -f kt-uImage.bin
	@echo "Copying kt-uImage from Linux Kernel..."
	@cp ${PATH_KERNEL_IMAGE_BIN} kt-uImage.bin

# flash kernel
flashk: kt-uImage.bin
	expect expect-sendfile ${SDEV} kt-uImage.bin
	sleep 2
	expect expect-script ${SDEV} "protect on 1:0-1"
	sleep 1
#	expect expect-script ${SDEV} "erase all"
	expect expect-script ${SDEV} "erase ${PART_KERNEL_START} ${PART_KERNEL_END}"
	sleep 1
	expect expect-script ${SDEV} "katinstall 100000"


# --- file system commands ----------------------

# copy filesystem image
fs_jffs2.img: ${FS_IMAGE}
	@rm -f fs_jffs2.img
	@echo "Copying jffs2 image from Familiar..."
	@cp ${PATH_IMAGE_FILE_SYSTEM} fs_jffs2.img

# file system image
flashd: fs_jffs2.img
	expect expect-sendfile ${SDEV} fs_jffs2.img
	sleep 2
	expect expect-script ${SDEV} "protect on 1:0-1"
	sleep 1
	expect expect-script ${SDEV} "erase ${PART_DISTRIB_START} ${PART_DISTRIB_END}"
	sleep 1
	expect expect-script ${SDEV} "cp.b a2000000 ${PART_DISTRIB_START} `./hexsize.sh fs_jffs2.img`"


# --- other commands ----------------------------

# set boot environment variable
bootargs:
	expect expect-script ${SDEV} "setenv bootargs ${BOOTARGS}"
	sleep 1
	expect expect-script ${SDEV} "saveenv"

# cleaning
clean:
	@echo "Cleaning ..."
	@rm -f *~ kt-uImage.bin fs_jffs2.img

