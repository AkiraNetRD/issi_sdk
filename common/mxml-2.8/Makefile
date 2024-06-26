#
# "$Id: Makefile.in 451 2014-01-04 21:50:06Z msweet $"
#
# Makefile for Mini-XML, a small XML-like file parsing library.
#
# Copyright 2003-2014 by Michael R Sweet.
#
# These coded instructions, statements, and computer programs are the
# property of Michael R Sweet and are protected by Federal copyright
# law.  Distribution and use rights are outlined in the file "COPYING"
# which should have been included with this file.  If this file is
# missing or damaged, see the license at:
#
#     http://www.msweet.org/projects.php/Mini-XML
#

#
# Compiler tools definitions...
#

AR		=	/home/parallels/Work/git/akira/openwrt/sdk/staging_dir/toolchain-aarch64_cortex-a53_gcc-13.1.0_musl/bin/aarch64-openwrt-linux-ar
ARFLAGS		=	crvs
ARCHFLAGS	=	
CC		=	/home/parallels/Work/git/akira/openwrt/sdk/staging_dir/toolchain-aarch64_cortex-a53_gcc-13.1.0_musl/bin/aarch64-openwrt-linux-gcc
CFLAGS		=	$(OPTIM) $(ARCHFLAGS) -Wall  -D__alpha -D_THREAD_SAFE -D_REENTRANT
CP		=	/bin/cp
DSO		=	:
DSOFLAGS	=	
LDFLAGS		=	$(OPTIM) $(ARCHFLAGS) 
INSTALL		=	/usr/bin/install -c
LIBMXML		=	libmxml.a
LIBS		=	 -lpthread
LN		=	/bin/ln -s
MKDIR		=	/bin/mkdir
OPTIM		=	-Os -g
RANLIB		=	ranlib
RM		=	/bin/rm -f
SHELL		=	/bin/sh


#
# Configured directories...
#

prefix		=	/usr/local
exec_prefix	=	/usr/local
bindir		=	${exec_prefix}/bin
datarootdir	=	${prefix}/share
includedir	=	${prefix}/include
libdir		=	${exec_prefix}/lib
mandir		=	${datarootdir}/man
docdir		=	${datarootdir}/doc/mxml
BUILDROOT	=	$(DSTROOT)


#
# Install commands...
#

INSTALL_BIN	=	$(LIBTOOL) $(INSTALL) -m 755 -s
INSTALL_DATA	=	$(INSTALL) -m 644
INSTALL_DIR	=	$(INSTALL) -d
INSTALL_LIB	=	$(LIBTOOL) $(INSTALL) -m 755
INSTALL_MAN	=	$(INSTALL) -m 644
INSTALL_SCRIPT	=	$(INSTALL) -m 755


#
# Rules...
#

.SILENT:
.SUFFIXES:	.c .man .o
.c.o:
	echo Compiling $<
	$(CC) $(CFLAGS) -c -o $@ $<


#
# Targets...
#

DOCFILES	=	doc/0.gif doc/1.gif doc/2.gif doc/3.gif doc/4.gif \
			doc/A.gif doc/B.gif doc/C.gif doc/D.gif \
			doc/mxml.html doc/mxmldoc.xsd \
			README COPYING CHANGES
PUBLIBOBJS	=	mxml-attr.o mxml-entity.o mxml-file.o mxml-get.o \
			mxml-index.o mxml-node.o mxml-search.o mxml-set.o
LIBOBJS		=	$(PUBLIBOBJS) mxml-private.o mxml-string.o
OBJS		=	mxmldoc.o testmxml.o $(LIBOBJS)

#Use this line for non Cross-Compiler
#TARGETS	=	$(LIBMXML) mxmldoc testmxml mxml.xml doc/mxml.man
#Use this line for Cross-Compiler
TARGETS		=	$(LIBMXML)


#
# Make everything...
#

all:		Makefile config.h $(TARGETS)


#
# Clean everything...
#

clean:
	echo Cleaning build files...
	$(RM) $(OBJS) $(TARGETS)
	$(RM) mxmldoc-static libmxml.a libmxml.so.1.5 libmxml.sl.1 libmxml.1.dylib


#
# Really clean everything...
#

distclean:	clean
	echo Cleaning distribution files...
	$(RM) config.cache config.log config.status
	$(RM) Makefile config.h
	$(RM) -r autom4te*.cache
	$(RM) *.bck *.bak
	$(RM) -r clang


#
# Run the clang.llvm.org static code analysis tool on the C sources.
#

.PHONY: clang clang-changes
clang:
	echo Doing static code analysis of all code using CLANG...
	$(RM) -r clang
	scan-build -V -k -o `pwd`/clang $(MAKE) $(MFLAGS) clean all
clang-changes:
	echo Doing static code analysis of changed code using CLANG...
	scan-build -V -k -o `pwd`/clang $(MAKE) $(MFLAGS) all


#
# Install everything...
#

install:	$(TARGETS) install-$(LIBMXML) install-libmxml.a
	echo Installing mxmldoc in $(BUILDROOT)$(bindir)...
	$(INSTALL_DIR) $(BUILDROOT)$(bindir)
	$(INSTALL_BIN) mxmldoc $(BUILDROOT)$(bindir)
	echo Installing documentation in $(BUILDROOT)$(docdir)...
	$(INSTALL_DIR) $(BUILDROOT)$(docdir)
	for file in $(DOCFILES); do \
		$(INSTALL_MAN) $$file $(BUILDROOT)$(docdir); \
	done
	echo Installing header files in $(BUILDROOT)$(includedir)...
	$(INSTALL_DIR) $(BUILDROOT)$(includedir)
	$(INSTALL_DATA) mxml.h $(BUILDROOT)$(includedir)
	echo Installing pkgconfig files in $(BUILDROOT)$(libdir)/pkgconfig...
	$(INSTALL_DIR) $(BUILDROOT)$(libdir)/pkgconfig
	$(INSTALL_DATA) mxml.pc $(BUILDROOT)$(libdir)/pkgconfig
	echo Installing man pages in $(BUILDROOT)$(mandir)...
	$(INSTALL_DIR) $(BUILDROOT)$(mandir)/man1
	$(INSTALL_MAN) doc/mxmldoc.man $(BUILDROOT)$(mandir)/man1/mxmldoc.1
	$(INSTALL_DIR) $(BUILDROOT)$(mandir)/man3
	$(INSTALL_MAN) doc/mxml.man $(BUILDROOT)$(mandir)/man3/mxml.3

install-libmxml.a:
	echo Installing libmxml.a to $(BUILDROOT)$(libdir)...
	$(INSTALL_DIR) $(BUILDROOT)$(libdir)
	$(INSTALL_LIB) libmxml.a $(BUILDROOT)$(libdir)
	$(RANLIB) $(BUILDROOT)$(libdir)/libmxml.a

install-libmxml.so.1.5:
	echo Installing libmxml.so to $(BUILDROOT)$(libdir)...
	$(INSTALL_DIR) $(BUILDROOT)$(libdir)
	$(INSTALL_LIB) libmxml.so.1.5 $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libmxml.so
	$(LN) libmxml.so.1.5 $(BUILDROOT)$(libdir)/libmxml.so
	$(RM) $(BUILDROOT)$(libdir)/libmxml.so.1
	$(LN) libmxml.so.1.5 $(BUILDROOT)$(libdir)/libmxml.so.1

install-libmxml.sl.1:
	echo Installing libmxml.sl to $(BUILDROOT)$(libdir)...
	$(INSTALL_DIR) $(BUILDROOT)$(libdir)
	$(INSTALL_LIB) libmxml.sl.1 $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libmxml.so
	$(LN) libmxml.sl.1 $(BUILDROOT)$(libdir)/libmxml.sl

install-libmxml.1.dylib:
	echo Installing libmxml.dylib to $(BUILDROOT)$(libdir)...
	$(INSTALL_DIR) $(BUILDROOT)$(libdir)
	$(INSTALL_LIB) libmxml.1.dylib $(BUILDROOT)$(libdir)
	$(RM) $(BUILDROOT)$(libdir)/libmxml.dylib
	$(LN) libmxml.1.dylib $(BUILDROOT)$(libdir)/libmxml.dylib


#
# Uninstall everything...
#

uninstall: uninstall-$(LIBMXML) uninstall-libmxml.a
	echo Uninstalling mxmldoc from $(BUILDROOT)$(bindir)...
	$(RM) $(BUILDROOT)$(bindir)/mxmldoc
	echo Uninstalling documentation from $(BUILDROOT)$(docdir)...
	$(RM) -r $(BUILDROOT)$(docdir)
	echo Uninstalling headers from $(BUILDROOT)$(includedir)...
	$(RM) $(BUILDROOT)$(includedir)/mxml.h
	echo Uninstalling pkgconfig files from $(BUILDROOT)$(libdir)/pkgconfig...
	$(RM) $(BUILDROOT)$(libdir)/pkgconfig/mxml.pc
	echo Uninstalling man pages from $(BUILDROOT)$(mandir)...
	$(RM) $(BUILDROOT)$(mandir)/man1/mxmldoc.1
	$(RM) $(BUILDROOT)$(mandir)/man3/mxml.3

uninstall-libmxml.a:
	echo Uninstalling libmxml.a from $(BUILDROOT)$(libdir)...
	$(RM) $(BUILDROOT)$(libdir)/libmxml.a

uninstall-libmxml.so.1.5:
	echo Uninstalling libmxml.so from $(BUILDROOT)$(libdir)...
	$(RM) $(BUILDROOT)$(libdir)/libmxml.so
	$(RM) $(BUILDROOT)$(libdir)/libmxml.so.1
	$(RM) $(BUILDROOT)$(libdir)/libmxml.so.1.4

uninstall-libmxml.sl.1:
	echo Uninstalling libmxml.sl from $(BUILDROOT)$(libdir)...
	$(RM) $(BUILDROOT)$(libdir)/libmxml.sl
	$(RM) $(BUILDROOT)$(libdir)/libmxml.sl.1

uninstall-libmxml.1.dylib:
	echo Uninstalling libmxml.dylib from $(BUILDROOT)$(libdir)...
	$(RM) $(BUILDROOT)$(libdir)/libmxml.dylib
	$(RM) $(BUILDROOT)$(libdir)/libmxml.1.dylib


#
# Make packages using EPM (http://www.epmhome.org/)
#

epm:	all
	echo Creating distribution packages...
	epm --output-dir dist -v -f native mxml
	epm --output-dir dist -v -f portable mxml


#
# autoconf stuff...
#

Makefile:	configure Makefile.in
	echo Updating makefile...
	if test -f config.status; then \
		./config.status --recheck; \
		./config.status; \
	else \
		./configure; \
	fi
	touch config.h


config.h:	configure config.h.in
	echo Updating config.h...
	autoconf
	if test -f config.status; then \
		./config.status --recheck; \
		./config.status; \
	else \
		./configure; \
	fi
	touch config.h


#
# Figure out lines-of-code...
#

.PHONY: sloc

sloc:
	echo "libmxml: \c"
	sloccount $(LIBOBJS:.o=.c) mxml-private.c mxml.h 2>/dev/null | \
		grep "Total Physical" | awk '{print $$9}'


#
# libmxml.a
#

libmxml.a:	$(LIBOBJS)
	echo Creating $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@

$(LIBOBJS):	mxml.h
mxml-entity.o mxml-file.o mxml-private.o: mxml-private.h


#
# libmxml.so.1.5
#

libmxml.so.1.5:	$(LIBOBJS)
	echo Creating $@...
	$(DSO) $(DSOFLAGS) -o libmxml.so.1.5 $(LIBOBJS)
	$(RM) libmxml.so libmxml.so.1
	$(LN) libmxml.so.1.5 libmxml.so
	$(LN) libmxml.so.1.5 libmxml.so.1


#
# libmxml.sl.1
#

libmxml.sl.1:	$(LIBOBJS)
	echo Creating $@...
	$(DSO) $(DSOFLAGS) -o libmxml.sl.1 $(LIBOBJS)
	$(RM) libmxml.sl
	$(LN) libmxml.sl.1 libmxml.sl


#
# libmxml.1.dylib
#

libmxml.1.dylib:	$(LIBOBJS)
	echo Creating $@...
	$(DSO) $(DSOFLAGS) -o libmxml.1.dylib \
		-install_name $(libdir)/libmxml.dylib \
		-current_version 1.5.0 \
		-compatibility_version 1.0.0 \
		$(LIBOBJS)
	$(RM) libmxml.dylib
	$(LN) libmxml.1.dylib libmxml.dylib


#
# mxmldoc
#

mxmldoc:	$(LIBMXML) mxmldoc.o
	echo Linking $@...
	$(CC) -L. $(LDFLAGS) -o $@ mxmldoc.o -lmxml $(LIBS)

mxmldoc-static:	libmxml.a mxmldoc.o
	echo Linking $@...
	$(CC) $(LDFLAGS) -o $@ mxmldoc.o libmxml.a $(LIBS)

mxmldoc.o:	mxml.h


#
# testmxml
#

testmxml:	libmxml.a testmxml.o
	echo Linking $@...
	$(CC) $(LDFLAGS) -o $@ testmxml.o libmxml.a $(LIBS)
	@echo Testing library...
	./testmxml test.xml >temp1.xml 2>temp1s.xml
	./testmxml temp1.xml >temp2.xml 2>temp2s.xml
	@if cmp temp1.xml temp2.xml; then \
		echo Stdio file test passed!; \
		$(RM) temp2.xml temp2s.xml; \
	else \
		echo Stdio file test failed!; \
	fi
	@if cmp temp1.xml temp1s.xml; then \
		echo String test passed!; \
		$(RM) temp1.xml temp1s.xml; \
	else \
		echo String test failed!; \
	fi
	@if cmp test.xml test.xmlfd; then \
		echo File descriptor test passed!; \
		$(RM) test.xmlfd; \
	else \
		echo File descriptor test failed!; \
	fi

testmxml-vg:	$(LIBOBJS) testmxml.o
	echo Linking $@...
	$(CC) $(LDFLAGS) -o $@ testmxml.o $(LIBOBJS) $(LIBS)

testmxml.o:	mxml.h


#
# mxml.xml
#

mxml.xml:	mxmldoc-static mxml.h $(PUBLIBOBJS:.o=.c)
	echo Generating API documentation...
	$(RM) mxml.xml
	./mxmldoc-static --header doc/reference.heading mxml.xml mxml.h $(PUBLIBOBJS:.o=.c) >doc/reference.html
	if test "x`uname`" = xDarwin; then \
		./mxmldoc-static --docset org.minixml.docset \
			--docversion 2.8 --feedname minixml.org \
			--feedurl http://www.minixml.org/org.minixml.atom \
			--header doc/docset.header --intro doc/docset.intro \
			--css doc/docset.css --title "Mini-XML API Reference" \
			mxml.xml || exit 1; \
		$(RM) org.minixml.atom; \
		xcrun docsetutil package --output org.minixml.xar \
			--atom org.minixml.atom \
			--download-url http://www.minixml.org/org.minixml.xar \
			org.minixml.docset || exit 1; \
	fi

valgrind:	mxmldoc-static
	echo Doing dynamic code analysis using Valgrind...
	$(RM) valgrind.xml
	valgrind --tool=memcheck --leak-check=yes ./mxmldoc-static \
		valgrind.xml mxml.h $(PUBLIBOBJS:.o=.c) \
		>valgrind.html 2>valgrind.out


#
# doc/mxml.man
#

doc/mxml.man: mxmldoc-static mxml.xml
	echo "Generating mxml(3) man page..."
	$(RM) doc/mxml.man
	./mxmldoc-static --man mxml --title "Mini-XML API" \
		--intro doc/intro.man --footer doc/footer.man \
		mxml.xml >doc/mxml.man


#
# All object files depend on the makefile...
#

$(OBJS):	Makefile config.h


#
# End of "$Id: Makefile.in 451 2014-01-04 21:50:06Z msweet $".
#
