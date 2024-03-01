######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../../CompilerDeclaration

######################################################################
# Other Declarations:                                                #
######################################################################
LIBPATH  = ../../bin
INCLUDE  = ../../include
INCLUDE_LIB_CURL  = "../curl-7.29.0/include"

CFLAGS = -Wall $(CROSS_CFLAGS)
CPPFLAGS = $(CROSS_CPPFLAGS) -I inc -I $(INCLUDE) -I $(INCLUDE_LIB_CURL)
TARGET = $(LIBPATH)/HTTP_LIB.a

SRC = src/HTTP_LIB_ext.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

