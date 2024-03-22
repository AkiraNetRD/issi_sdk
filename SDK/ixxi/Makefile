ROOT_DIR=/home/naruto/owrt/sdk_1222

ifeq ($(PLATFORM), aarch64-openwrt)
CROSS_COMPILE = aarch64-openwrt-linux
export STAGING_DIR = $(ROOT_DIR)/staging_dir
export TOOLCHAIN_DIR = $(STAGING_DIR)/toolchain-aarch64_cortex-a53_gcc-13.1.0_musl
export TARGET_DIR = $(STAGING_DIR)/target-aarch64_cortex-a53_musl
else
CROSS_COMPILE = x86
endif

ifneq ($(ARCH),)
_ARCH = ARCH=$(ARCH)
endif

ifeq ($(CROSS_COMPILE), aarch64-openwrt-linux)
	CC = $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)-gcc
	AR = $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)-ar
	LD = $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)-ld
	AS = $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)-as
	STRIP = $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)-strip
	RANLIB = $(TOOLCHAIN_DIR)/bin/$(CROSS_COMPILE)-ranlib
	OWRTINC = $(TOOLCHAIN_DIR)/usr/include
	OWRTLIB = $(TOOLCHAIN_DIR)/usr/lib
	TARGETINC = $(TARGET_DIR)/usr/include
	TARGETLIB = $(TARGET_DIR)/usr/lib
	TARGETLIB_MEDIATEK = $(TARGET_DIR)/root-mediatek/lib
else
	ifeq ($(CROSS_COMPILE), x86)
		CC = gcc
		AR = ar
		LD = ld
		AS = as
		STRIP = strip
		RANLIB = ranlib
	endif
endif

LIBPATH  =  ../../bin
INCLUDE  = ../../include
INCLUDE_HTTP_LIB        = "../../common/HTTP_LIB/inc"
INCLUDE_LIB_COMMON      = "../../common/common/inc"
INCLUDE_LIB_GHN_LIB   = "../../GHN_LIB/inc"
INCLUDE_LIB_CMD_HELPERS = "../../common/CMD_Helpers/inc"
INCLUDE_XML_Parser      = "../../common/XMLParser/inc"

CPPFLAGS = -I $(INCLUDE) -I $(INCLUDE_HTTP_LIB) -I $(INCLUDE_LIB_COMMON) -I $(INCLUDE_LIB_GHN_LIB) -I $(INCLUDE_LIB_CMD_HELPERS) -I $(INCLUDE_XML_Parser)

LIBS = $(LIBPATH)/CMD_Helpers.a $(LIBPATH)/GHN_LIB.a $(LIBPATH)/Image_LIB.a $(LIBPATH)/console.a $(LIBPATH)/HTTP_LIB.a $(LIBPATH)/libcurl.a $(LIBPATH)/XMLParser.a $(LIBPATH)/libmxml.a $(LIBPATH)/common.a $(LIBPATH)/MD5.a -lm -lrt

_DBG = 
ifneq ($(DBG),)
ifeq ($(DBG),1)
_DBG = -D _DEBUG 
endif
endif

LIBRARY = libixxi.so
SRC = ixxi.c
OBJS = $(SRC:.c=.o)

LDLIBS := -lcrypto -lssl -ljson-c -lubox -lubus -pthread

ifeq ($(CROSS_COMPILE), aarch64-openwrt-linux)
$(LIBRARY) : $(OBJS)
	$(CC) -shared -fPIC -o $@ $^ -L $(OWRTLIB) -L $(TARGETLIB) $(LDLIBS) $(LIBS)

%.o : %.c
	$(CC) $(_DBG) $(CFLAGS) $(CPPFLAGS) -I $(TARGETINC)/ -I $(OWRTINC)/ -c -o $@ $<
else
$(LIBRARY) : $(OBJS)
	$(CC) -shared -fPIC -o $@ $^ $(LDLIBS) $(LIBS)

%.o : %.c 
	$(CC) $(_DBG) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
endif

.PHONY: clean
clean:
	rm $(LIBRARY) -f; rm -f *.o