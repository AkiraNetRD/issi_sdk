#
# Some macros
#

all:
	$(MAKE) -C common all
	$(MAKE) -C console all
	$(MAKE) -C GHN_LIB all
	$(MAKE) -C SDK all

clean:
	$(MAKE) -C common clean
	$(MAKE) -C console clean
	$(MAKE) -C GHN_LIB clean
	$(MAKE) -C SDK clean
