######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../../bin
INCLUDE  = ../../include

CFLAGS = -Wall $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I inc -I $(INCLUDE)
TARGET = $(LIBPATH)/common.a

SRC = src/common.c src/Ghn_netinf.c src/strptime.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

