######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../bin
INCLUDE  = ../include 

INCLUDE_LIB_CONSOLE       = "../console/inc"
INCLUDE_LIB_COMMON        = "../common/common/inc"
INCLUDE_LIB_HTTP_LIB      = "../common/HTTP_LIB/inc"
INCLUDE_LIB_XMLParser     = "../common/XMLParser/inc"
INCLUDE_LIB_Image_LIB     = "../common/Image_LIB/inc"

CFLAGS = -Wall $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I inc -I $(INCLUDE) -I $(INCLUDE_LIB_CONSOLE) -I $(INCLUDE_LIB_COMMON) -I $(INCLUDE_LIB_HTTP_LIB) -I $(INCLUDE_LIB_XMLParser) -I $(INCLUDE_LIB_Image_LIB)
TARGET = $(LIBPATH)/GHN_LIB.a

LIBS = $(LIBPATH)/common.a $(LIBPATH)/MD5.a

SRC = src/GHN_LIB_ext.c src/GHN_LIB_UpgradeFW.c src/GHN_LIB_int.c src/GHN_LIB_netinf.c src/GHN_LIB_LinkStatistics.c src/GHN_LIB_Layer2Connection.c src/GHN_LIB_Flash.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

