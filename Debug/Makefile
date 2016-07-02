
#################
#################
MAKEFILE       = Makefile
THIS	       = Debug
#SHELL         = /bin/tcsh
INCLUDES      = ../includes 
OBJDIR        = "../$(THIS)"
OSTYPE      = `uname -s`
#################
#################
# Compiler parameters
#################
#GLIBS     = -lGL -lGLU
GLUTLIB    = -lglut
LIBS      = $(GLIBS) 
LIBPATH       = .;$(JOGL_LIB)
CLASSPATH    = ".;$(JOGL_LIB)/jogl.jar;$(JOGL_LIB)/gluegen-rt.jar"
INCS  		= -I../jpeglib
OPTLVL     = -g

#DEFINES	      = -D$(PLATFORM)  -w -DUSE_GLEE -DUSE_MGR_MATRIX
DEFINES	      = -D$(PLATFORM) $(MAXRGB) -Wall -Wno-non-virtual-dtor -fno-strict-aliasing -DLINUX -fpermissive

GLDEFS	   = -DGLEW_STATIC -DGLEW
#GFILE         = GLee.cpp
GFILE         = glew.cpp
LFLAGS        = $(GLDFLAGS)
CFLAGS        = -w $(GCCFLAGS) $(OPTLVL) $(DEFINES) $(GLDEFS)
MFLAGS        = -j10
CC            = g++
LD	       	  = $(CC)

WXLIBS        = `wx-config --libs  --gl_libs`
WXFLAGS       = `wx-config --cxxflags`

#################
PROGRAM       = vtx.exe
#################
# JAVA 
#################
JFLAGS        = -deprecation
NLIB          = ../jni/$(LIBPRE)vtx$(LIBEXT)
JFILES        = VtxMain.java VtxScene.java
JOBJS         = $(JFILES:.java=.class)
#################
# .SUFFIXES   - implicit dependeny rules
#################
.SUFFIXES:
.SUFFIXES: .java .class
.SUFFIXES: .cpp .o
#################
# Target Rules
#################
build_wx:   parsers wx vtxw
build_jni:  parsers jpeglib jni vtxlib
build_glut: parsers jpeglib glut includes vtxg

wx:;	    cd  ../wx; \
            make $(MFLAGS) \
			INCS="-I../includes -I../ -I../Resources" \
			CFLAGS="$(CFLAGS)" \
			CC="$(CC)" \
			OBJDIR="../$(THIS)" \
			build; \
			cd ../$(THIS); \
			make $(MFLAGS) CFLAGS="$(CFLAGS) -DWXWIN" includes;

jni:;	    @cd ../jni; \
            make \
			CFLAGS="$(CFLAGS) -DJNI $(INCS)" \
			CC="$(CC)" \
			build; \
			cd ../$(THIS); \
			make INCS="-I../jpeglib" includes;

glut:;      @cd  ../glut; \
            make \
			INCS="-I../includes -I../" \
			CFLAGS="$(CFLAGS)" \
			CC="$(CC)" \
			build; \
			cd ../$(THIS); \
			make INCS="-I../jpeglib" includes;

jpeglib:;   @cd  ../jpeglib; \
            make \
			CFLAGS="$(CFLAGS)" \
			CC="$(CC)" \
			OBJDIR="../$(THIS)" \
			LIBS="$(LIBS)" \
			build

parsers:;   @cd  ../parsers; \
            make $(MFLAGS) \
			CFLAGS="$(CFLAGS) -I../includes -I../" \
			OBJDIR="../$(THIS)" \
			build	

includes:;	cd  ../includes; \
            make $(MFLAGS) \
			INCS="-I../ -I../includes " \
			CFLAGS="$(CFLAGS)"\
			GFILE="$(GFILE)" \
			CC="$(CC)" \
			OBJDIR="../$(THIS)" \
			LIBS="$(LIBS)" \
			build

vtxlib:;	@echo "### Archiving $(NLIB) ...";
		    $(CC) $(LFLAGS) -shared -o $(NLIB) *.o ../jni/*.o $(LIBS)
			@echo "### done"

vtxg:;	    @echo "### Building vtx-glut ...";
			$(CC) $(CFLAGS) -o $(PROGRAM) *.o ../glut/*.o $(LIBS) $(GLUTLIB);
			@echo "### done"

vtxw:;	    @echo "### Building vtx-wxWidgets ...";
			$(CC) $(CFLAGS) -o $(PROGRAM) *.o  `wx-config --libs --gl_libs` ;
			@echo "### done"

showenv:;   @echo "CFLAGS = $(CFLAGS)"
			@echo "LFLAGS = $(LFLAGS)"
			@echo "  INCS = $(INCS)"
clean:;		rm -f *.class *.o $(NLIB) $(PROGRAM)  ../jni/*.o ../glut/*.o ../parsers/*.o $(NHDRS)

#################
# Make Utilities
#################
jni_test:;     cd ../jni; java  -Djava.library.path='$(LIBPATH)' -classpath $(CLASSPATH) VtxMain

test:;         ./vtx.exe --m alpine2.spx
#test:;         ./vtx.exe --m  last.spx

### DEPENDENCIES ###

