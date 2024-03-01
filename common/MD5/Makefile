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
TARGET = $(LIBPATH)/MD5.a

SRC = src/md5.c
OBJ = $(addsuffix .o, $(basename $(SRC)))

.c.o: 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEBUG) -o $@ $<

all: $(OBJ)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

