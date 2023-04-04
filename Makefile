#################
#
#
# DESCRIPTION	Makefile for vtxc
#
#################

MAKEFILE      = Makefile
SHELL	      = /bin/sh
THIS	      = vtx

#################
# 'install'/'update' parameters
#################

BUILD_DIRS	  =	Debug Release
RCS_FILES	  = Makefile
VERSION       =
#################
# Target Rules
#################
all:;       @echo "build project from Release or Debug Directories"

release:;   @cd Release; \
            $(MAKE) build; \
			cd ..;

debug:;     @cd Debug; \
			$(MAKE) build; \
			cd ..;

clean:;		@rm -f core
			@for dir in $(BUILD_DIRS); do \
			echo "$(@) $$dir ..."; \
			cd $$dir; $(MAKE) $(@); \
			cd ..; done

test:;   @cd Release; \
            $(MAKE) test; \
			cd ..;

distribution:; \
            tar -cvf binaries/$(PLATFORM)_VTX.tar Release/vtx.exe Resources Shaders Textures Saves

showenv:;   @echo "JAVA_HOME = $(JAVA_HOME)";
		    @echo "  JAVAINC = $(JAVAINC)"
		    @echo "    GLIBS = $(GLIBS)"
		    @echo "   GLINCS = $(GLINCS)"
			@echo " GCCFLAGS = $(GCCFLAGS)"
			@echo " GLDFLAGS = $(GLDFLAGS)"
			@echo " PLATFORM = $(PLATFORM)"
			@echo "  LIBTEST = $(LIBPRE)test$(LIBEXT)"						
			@echo "PATH:$(PATH)"