#################
#
#
# DESCRIPTION	Makefile for Eclipse managed (pre-build step) or unmanaged build (release)
#
#################
MAKEFILE    = Makefile
#PARSERS	    = ../PARSERS
#################
#################
# Lex/Yacc parameters
#################

LEX         = flex
YACC        = bison -y
#OBJDIR      = PARSERS
#INCLUDES    =  ../includes

PPFLAGS     = -w -DYYDEBUG -DYY_ALWAYS_INTERACTIVE=1
LFLAGS      = -l
YFLAGS      = -d -v
YYFLAGS     = -W -N
CFLAGS      = -DGLEW -DGLEW_STATIC ${GCCBUILD}

#################
# Program parameters
#################
CC         = g++
C          = gcc

HDRS	    = $(INCLUDES)/SceneClass.h \
		      $(INCLUDES)/ObjectClass.h

#################
# Object files
################# 
OBJS = $(OBJDIR)/yytopp \
      $(OBJDIR)/sx_y.o \
      $(OBJDIR)/sx_l.o
#################
# Target Rules
#################
build:      $(OBJS)

$(OBJDIR)/yytopp: 	$(PARSERS)/yytopp.l 
			$(LEX) -o $(OBJDIR)/yytopp.c -l $(PARSERS)/yytopp.l
			$(C) -Bstatic $(OBJDIR)/yytopp.c -o $(OBJDIR)/yytopp
			@rm -f yytopp.c

$(OBJDIR)/sx_y.o:  $(OBJDIR)/yytopp   $(PARSERS)/sx.y $(HDRS) 
			$(YACC) $(YFLAGS) $(PARSERS)/sx.y -o $(OBJDIR)/y.tab.c
			cat $(OBJDIR)/y.tab.c | $(OBJDIR)/yytopp $(YYFLAGS) > $(OBJDIR)/sx_y.cpp
			@rm $(OBJDIR)/y.tab.c
			@mv $(OBJDIR)/y.tab.h $(OBJDIR)/sx_y.h
			$(CC) $(CFLAGS) -I$(OBJDIR) -I$(INCLUDES) $(PPFLAGS) -c -o $(OBJDIR)/sx_y.o $(OBJDIR)/sx_y.cpp

$(OBJDIR)/sx_l.o: $(OBJDIR)/yytopp $(PARSERS)/sx.l 
			$(LEX) -l -t $(PARSERS)/sx.l | $(OBJDIR)/yytopp $(YYFLAGS) > $(OBJDIR)/sx_l.cpp
			$(CC) $(CFLAGS) -I$(OBJDIR) -I$(INCLUDES) $(PPFLAGS) -c -o $(OBJDIR)/sx_l.o $(OBJDIR)/sx_l.cpp
			

#################
# dependencies
#################
$(OBJDIR)/sx_y.h: $(PARSERS)/sx.y
$(OBJDIR)/yytopp: $(PARSERS)/yytopp.l
$(OBJDIR)/sx_l.o: $(PARSERS)/sx.l
$(OBJDIR)/sx_y.o: $(PARSERS)/sx.y
