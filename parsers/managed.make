#################
#
#
# DESCRIPTION	parsers Makefile for Eclipse managed build (pre-build step)
#
#################
MAKEFILE    = Makefile
PARSERS	    = ../parsers
#################
#################
# Lex/Yacc parameters
#################

LEX         = flex
YACC        = bison -y
OBJDIR      = parsers
INCLUDES    =  ../../includes

PPFLAGS     = -w -DYYDEBUG -DYY_ALWAYS_INTERACTIVE=1
LFLAGS      = -l
YFLAGS      = -d -v
YYFLAGS     = -W -N       

#################
# Program parameters
#################
CPP         = g++
CC          = gcc

HDRS	    = ../includes/SceneClass.h \
		      ../includes/ObjectClass.h

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
			gcc
			 -Bstatic $(OBJDIR)/yytopp.c -o $(OBJDIR)/yytopp
			@rm -f yytopp.c

$(OBJDIR)/sx_y.o:     $(PARSERS)/sx.y $(HDRS) 
			$(YACC) $(YFLAGS) $(PARSERS)/sx.y -o $(OBJDIR)/y.tab.c
			cat $(OBJDIR)/y.tab.c | $(OBJDIR)/yytopp $(YYFLAGS) > $(OBJDIR)/sx_y.cpp
			@rm $(OBJDIR)/y.tab.c
			@mv $(OBJDIR)/y.tab.h $(OBJDIR)/sx_y.h
			$(CPP) $(CFLAGS) -I$(OBJDIR) -I$(INCLUDES) $(PPFLAGS) -c -o $(OBJDIR)/sx_y.o $(OBJDIR)/sx_y.cpp

$(OBJDIR)/sx_l.o:     $(PARSERS)/sx.l 
			$(LEX) -l -t $(PARSERS)/sx.l | $(OBJDIR)/yytopp $(YYFLAGS) > $(OBJDIR)/sx_l.cpp
			$(CPP) $(CFLAGS) -I$(OBJDIR) -I$(INCLUDES) $(PPFLAGS) -c -o $(OBJDIR)/sx_l.o $(OBJDIR)/sx_l.cpp
			
clean:;		rm -fr .sb y.output lex.yy.c sx_y.h sx_y.cpp sx_l.cpp yytopp

#################
# dependencies
#################
