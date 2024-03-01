# ISSI SDK (GHN_SDK-3.2.15-x86_Linux)
This repository is a submodule for who wants to use ISSI sdk

### [How to setup build environment]

Modify ./CompilerDeclaration as following block

###############################
# Common Declarations         #
###############################

CC             = $(CROSS)gcc
AR             = $(CROSS)ar
ARFLAGS        = rcs

CFLAGS         = $(CROSS_CFLAGS)
CPPFLAGS       = $(CROSS_CPPFLAGS)
DEBUG          = $(CROSS_DEBUG)

----------------------------------------------
###############################
# Common Declarations         #
###############################

ARM_ROOT       = /openwrt/sdk/staging_dir/toolchain-aarch64_cortex-a53_gcc-13.1.0_musl
CROSS          = $(ARM_ROOT)/bin/aarch64-openwrt-linux-
CROSS_HOST     = --host=aarch64-openwrt-linux
CROSS_CFLAGS   =
CROSS_CPPFLAGS =
CROSS_LIBS     = -lrt
CROSS_DEBUG    =

CC             = $(CROSS)gcc
AR             = $(CROSS)ar
ARFLAGS        = rcs

CFLAGS         = $(CROSS_CFLAGS)
CPPFLAGS       = $(CROSS_CPPFLAGS)
DEBUG          = $(CROSS_DEBUG)

