######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../../bin
INCLUDE  = ../../include 
INCLUDE_FW1  = ../../include/platform/pub
INCLUDE_FW2  = ../../include/services/pub
INCLUDE_FW3  = ../../include/protocol/ghn/pub
INCLUDE_FW4  = ../../include/golan/pcu/pub
INCLUDE_FW5  = ../../include/golan/pub

INCLUDE_LIB_COMMON  = "../common/inc"
INCLUDE_LIB_MD5  = "../MD5/inc"

CFLAGS = -Wall $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I inc -I $(INCLUDE) -I $(INCLUDE_FW1) -I inc -I $(INCLUDE_FW2) -I inc -I $(INCLUDE_FW3) -I inc -I $(INCLUDE_FW4) -I inc -I $(INCLUDE_FW5) -I $(INCLUDE_LIB_COMMON) -I $(INCLUDE_LIB_MD5)
TARGET = $(LIBPATH)/Image_LIB.a

LIBS = $(LIBPATH)/common.a $(LIBPATH)/MD5.a

SRC = src/Image_LIB_ext.c src/Image_LIB_TLV.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

