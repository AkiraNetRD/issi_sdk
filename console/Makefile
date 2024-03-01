######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../bin
INCLUDE  = ../include
INCLUDE_FW1  = ../include/platform/pub
INCLUDE_FW2  = ../include/services/pub
INCLUDE_FW3  = ../include/protocol/ghn/pub
INCLUDE_FW4  = ../include/golan/pcu/pub
INCLUDE_FW5  = ../include/golan/pub
INCLUDE_LIB_COMMON  = "../common/common/inc"

CFLAGS = -Wall $(CROSS_CFLAGS)
CPPFLAGS_CONSOLE = -DCONSOLE_NO_LAYER2_RETRANSMISSION
CPPFLAGS = $(CROSS_CPPFLAGS) $(CPPFLAGS_CONSOLE)  -I inc -I $(INCLUDE) -I $(INCLUDE_LIB_COMMON) -I $(INCLUDE_FW1) -I inc -I $(INCLUDE_FW2) -I inc -I $(INCLUDE_FW3) -I inc -I $(INCLUDE_FW4) -I inc -I $(INCLUDE_FW5)
TARGET = $(LIBPATH)/console.a

SRC = src/cmgr.c src/console_CommandFunc.c src/console_Helpers.c src/console_Logger.c src/console_PacketHandling.c src/eth.c src/nic.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

