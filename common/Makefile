######################################################################
# Compiler / Cross-Compiler Declarations:                            #
######################################################################
include ../CompilerDeclaration

all:
	# Compile the libcurl external-library
	cd curl-7.29.0; \
	chmod +x ./configure; \
	export CC='$(CC)' && export AR='$(AR)' && export CFLAGS='$(CFLAGS)' && export CPPFLAGS='$(CPPFLAGS)' && ./configure $(CROSS_HOST) --disable-shared --without-ssl --disable-ldap --disable-manual --without-zlib --without-libidn --disable-verbose; \
	cd ..
	$(MAKE) -C curl-7.29.0 all
	cp curl-7.29.0/lib/.libs/libcurl.a ../bin

	# Compile the mini-xml external-library
	cd mxml-2.8; \
	chmod +x ./configure; \
	export CC='$(CC)' && export AR='$(AR)' && export CFLAGS='$(CFLAGS)' && export CPPFLAGS='$(CPPFLAGS)' && ./configure $(CROSS_HOST)
	$(MAKE) -C mxml-2.8 all
	cp mxml-2.8/libmxml.a ../bin

	$(MAKE) -C "HTTP_LIB" all
	$(MAKE) -C "XMLParser" all
	$(MAKE) -C "MD5" all
	$(MAKE) -C common all
	$(MAKE) -C "Image_LIB" all
	$(MAKE) -C "CMD_Helpers" all

clean:

	$(MAKE) -C curl-7.29.0 clean
	rm ../bin/libcurl.a
	$(MAKE) -C mxml-2.8 clean
	rm ../bin/libmxml.a

	$(MAKE) -C "HTTP_LIB" clean
	$(MAKE) -C "XMLParser" clean
	$(MAKE) -C "MD5" clean
	$(MAKE) -C common clean
	$(MAKE) -C "Image_LIB" clean
	$(MAKE) -C "CMD_Helpers" clean
