#################
#
#
#################
#################
# 'make' build parameters
#################
MAKEFILE      = Makefile
THIS	      = jpeg
#################
# Compiler parameters
#################
# passed in by build target in $(OBJDIR)/Makefile
######################

CC            =
CFLAGS        =
INCS          =
OBJDIR        = 

#################
# Program parameters
#################
#################
# RCS files
#################
HDR_FILES =     \
				jchuff.h \
				jconfig.h \
				jdct.h \
				jdhuff.h \
				jerror.h \
				jinclude.h \
				jmemsys.h \
				jmorecfg.h \
				jpegint.h \
				jpeglib.h \
				jversion.h

SRC_FILES=      \
				jcapimin.c \
				jcapistd.c \
				jccoefct.c \
				jccolor.c \
				jcdctmgr.c \
				jchuff.c \
				jcinit.c \
				jcmainct.c \
				jcmarker.c \
				jcmaster.c \
				jcomapi.c \
				jcparam.c \
				jcphuff.c \
				jcprepct.c \
				jcsample.c \
				jctrans.c \
				jdapimin.c \
				jdapistd.c \
				jdatadst.c \
				jdatasrc.c \
				jdcoefct.c \
				jdcolor.c \
				jddctmgr.c \
				jdhuff.c \
				jdinput.c \
				jdmainct.c \
				jdmarker.c \
				jdmaster.c \
				jdmerge.c \
				jdphuff.c \
				jdpostct.c \
				jdsample.c \
				jdtrans.c \
				jerror.c \
				jfdctflt.c \
				jfdctfst.c \
				jfdctint.c \
				jidctflt.c \
				jidctfst.c \
				jidctint.c \
				jidctred.c \
				jmemmgr.c \
				jmemnobs.c \
				jquant1.c \
				jquant2.c \
				jutils.c

RCS_FILES  =   $(MAKEFILE) $(HDR_FILES) $(SRC_FILES)
             
#################
# Object files
#################

OBJS= $(SRC_FILES:%.c=$(OBJDIR)/%.o)
DEFINES      = -DINT32="int"
DEPEND_SRC   =  $(SRC_FILES)
COMPILE.c   = $(CC) $(CFLAGS) $(DEFINES) $(INCS) -c
#################
# Target Rules
#################

all:;       @echo "Nothing to do. To build objects, run make from sparc or linux"

build:		    $(OBJS)

$(OBJDIR)/%.o: %.c
	$(COMPILE.c) -o $(@) $(@F:%.o=%.c)

clean:;			@rm -fr $(OBJS) .sb
depend:;         @makedepend -Y./ $(DEPEND_SRC) -p"../Debug/" 2>/dev/null; \
                 makedepend -a  -Y./ $(DEPEND_SRC) -p"../Release/" 2>/dev/null

#################
# dependencies
#################

# DO NOT DELETE THIS LINE -- make depend depends on it.

../Debug/jcapimin.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcapimin.o: jerror.h
../Debug/jcapistd.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcapistd.o: jerror.h
../Debug/jccoefct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jccoefct.o: jerror.h
../Debug/jccolor.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jccolor.o: jerror.h
../Debug/jcdctmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcdctmgr.o: jerror.h jdct.h
../Debug/jchuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jchuff.o: jerror.h jchuff.h
../Debug/jcinit.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcinit.o: jerror.h
../Debug/jcmainct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcmainct.o: jerror.h
../Debug/jcmarker.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcmarker.o: jerror.h
../Debug/jcmaster.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcmaster.o: jerror.h
../Debug/jcomapi.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcomapi.o: jerror.h
../Debug/jcparam.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcparam.o: jerror.h
../Debug/jcphuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcphuff.o: jerror.h jchuff.h
../Debug/jcprepct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcprepct.o: jerror.h
../Debug/jcsample.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jcsample.o: jerror.h
../Debug/jctrans.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jctrans.o: jerror.h
../Debug/jdapimin.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdapimin.o: jerror.h
../Debug/jdapistd.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdapistd.o: jerror.h
../Debug/jdatadst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdatadst.o: jerror.h
../Debug/jdatasrc.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdatasrc.o: jerror.h
../Debug/jdcoefct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdcoefct.o: jerror.h
../Debug/jdcolor.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdcolor.o: jerror.h
../Debug/jddctmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jddctmgr.o: jerror.h jdct.h
../Debug/jdhuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdhuff.o: jerror.h jdhuff.h
../Debug/jdinput.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdinput.o: jerror.h
../Debug/jdmainct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdmainct.o: jerror.h
../Debug/jdmarker.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdmarker.o: jerror.h
../Debug/jdmaster.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdmaster.o: jerror.h
../Debug/jdmerge.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdmerge.o: jerror.h
../Debug/jdphuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdphuff.o: jerror.h jdhuff.h
../Debug/jdpostct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdpostct.o: jerror.h
../Debug/jdsample.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdsample.o: jerror.h
../Debug/jdtrans.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jdtrans.o: jerror.h
../Debug/jerror.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jerror.o: jerror.h jversion.h
../Debug/jfdctflt.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jfdctflt.o: jerror.h jdct.h
../Debug/jfdctfst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jfdctfst.o: jerror.h jdct.h
../Debug/jfdctint.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jfdctint.o: jerror.h jdct.h
../Debug/jidctflt.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jidctflt.o: jerror.h jdct.h
../Debug/jidctfst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jidctfst.o: jerror.h jdct.h
../Debug/jidctint.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jidctint.o: jerror.h jdct.h
../Debug/jidctred.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jidctred.o: jerror.h jdct.h
../Debug/jmemmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jmemmgr.o: jerror.h jmemsys.h
../Debug/jmemnobs.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jmemnobs.o: jerror.h jmemsys.h
../Debug/jquant1.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jquant1.o: jerror.h
../Debug/jquant2.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jquant2.o: jerror.h
../Debug/jutils.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Debug/jutils.o: jerror.h

../Release/jcapimin.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcapimin.o: jerror.h
../Release/jcapistd.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcapistd.o: jerror.h
../Release/jccoefct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jccoefct.o: jerror.h
../Release/jccolor.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jccolor.o: jerror.h
../Release/jcdctmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcdctmgr.o: jerror.h jdct.h
../Release/jchuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jchuff.o: jerror.h jchuff.h
../Release/jcinit.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcinit.o: jerror.h
../Release/jcmainct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcmainct.o: jerror.h
../Release/jcmarker.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcmarker.o: jerror.h
../Release/jcmaster.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcmaster.o: jerror.h
../Release/jcomapi.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcomapi.o: jerror.h
../Release/jcparam.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcparam.o: jerror.h
../Release/jcphuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcphuff.o: jerror.h jchuff.h
../Release/jcprepct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcprepct.o: jerror.h
../Release/jcsample.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jcsample.o: jerror.h
../Release/jctrans.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jctrans.o: jerror.h
../Release/jdapimin.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdapimin.o: jerror.h
../Release/jdapistd.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdapistd.o: jerror.h
../Release/jdatadst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdatadst.o: jerror.h
../Release/jdatasrc.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdatasrc.o: jerror.h
../Release/jdcoefct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdcoefct.o: jerror.h
../Release/jdcolor.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdcolor.o: jerror.h
../Release/jddctmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jddctmgr.o: jerror.h jdct.h
../Release/jdhuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdhuff.o: jerror.h jdhuff.h
../Release/jdinput.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdinput.o: jerror.h
../Release/jdmainct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdmainct.o: jerror.h
../Release/jdmarker.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdmarker.o: jerror.h
../Release/jdmaster.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdmaster.o: jerror.h
../Release/jdmerge.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdmerge.o: jerror.h
../Release/jdphuff.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdphuff.o: jerror.h jdhuff.h
../Release/jdpostct.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdpostct.o: jerror.h
../Release/jdsample.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdsample.o: jerror.h
../Release/jdtrans.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jdtrans.o: jerror.h
../Release/jerror.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jerror.o: jerror.h jversion.h
../Release/jfdctflt.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jfdctflt.o: jerror.h jdct.h
../Release/jfdctfst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jfdctfst.o: jerror.h jdct.h
../Release/jfdctint.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jfdctint.o: jerror.h jdct.h
../Release/jidctflt.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jidctflt.o: jerror.h jdct.h
../Release/jidctfst.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jidctfst.o: jerror.h jdct.h
../Release/jidctint.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jidctint.o: jerror.h jdct.h
../Release/jidctred.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jidctred.o: jerror.h jdct.h
../Release/jmemmgr.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jmemmgr.o: jerror.h jmemsys.h
../Release/jmemnobs.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jmemnobs.o: jerror.h jmemsys.h
../Release/jquant1.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jquant1.o: jerror.h
../Release/jquant2.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jquant2.o: jerror.h
../Release/jutils.o: jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h
../Release/jutils.o: jerror.h
