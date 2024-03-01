######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../../bin
INCLUDE  = ../../include
INCLUDE_LIB_MXML  = "../mxml-2.8"

CFLAGS = -Wall $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I inc -I $(INCLUDE) -I $(INCLUDE_LIB_MXML)
TARGET = $(LIBPATH)/XMLParser.a

SRC = src/XMLParser_ext.c src/XMLParser_helpers.c src/XMLParser_int.c src/XMLParser_typedefs.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

