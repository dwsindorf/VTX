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

# Install bundle -> $(DISTDIR)/VTX-win64-<git describe>.zip
# ships only git-tracked, runtime-used files (see ./mkdist.sh -h)
#   make dist                          package $(EXE)
#   make dist EXE=Release/vtx.exe      package a different build
#   make dist DISTFLAGS="-n -v"        dry run: list what would ship
EXE           = Release/vtx.exe
DISTDIR       = dist
.PHONY: dist distribution
dist distribution:; ./mkdist.sh -e $(EXE) -o $(DISTDIR) $(DISTFLAGS)

# Windows installer (Inno Setup 6) -> $(DISTDIR)/VTX-win64-<version>-setup.exe
.PHONY: installer
installer:; ./mkdist.sh -i -e $(EXE) -o $(DISTDIR) $(DISTFLAGS)

# User manual: regenerate Part II (reference) of Docs/VTX-User-Manual.odt from
# the source, in place, and write a preview PDF to $(DISTDIR)/
.PHONY: manual
manual:; bash tools/manual/build.sh refresh Docs/VTX-User-Manual.odt && \
		  bash tools/manual/build.sh pdf Docs/VTX-User-Manual.odt $(DISTDIR)/VTX-User-Manual.pdf

showenv:;   @echo "JAVA_HOME = $(JAVA_HOME)";
		    @echo "  JAVAINC = $(JAVAINC)"
		    @echo "    GLIBS = $(GLIBS)"
		    @echo "   GLINCS = $(GLINCS)"
			@echo " GCCFLAGS = $(GCCFLAGS)"
			@echo " GLDFLAGS = $(GLDFLAGS)"
			@echo " PLATFORM = $(PLATFORM)"
			@echo "  LIBTEST = $(LIBPRE)test$(LIBEXT)"						
			@echo "PATH:$(PATH)"
			