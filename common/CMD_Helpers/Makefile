######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../../bin
INCLUDE  = ../../include
INCLUDE_LIB_COMMON   = "../common/inc"
INCLUDE_LIB_GHN_LIB   = "../../GHN_LIB/inc"

CFLAGS = -Wall $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I inc -I $(INCLUDE) -I $(INCLUDE_LIB_COMMON) -I $(INCLUDE_LIB_GHN_LIB)
TARGET = $(LIBPATH)/CMD_Helpers.a

SRC = src/CMD_Helpers.c src/CMD_Helpers_Coexistence.c src/CMD_Helpers_DeviceName.c src/CMD_Helpers_MasterSelection.c src/CMD_Helpers_PowerSaveMode.c src/CMD_Helpers_SecurityParams.c src/CMD_Helpers_Utilization.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

