#################
#
#
# DESCRIPTION	Makefile for ../sx/sparc
#
#################
MAKEFILE    = Makefile
THIS	    = ../parsers
#################
#################
# Lex/Yacc parameters
#################

LEX         = flex
YACC        = bison -y
OBJDIR      = .
INCLUDES    =  ../includes

PPFLAGS     = -w -DYYDEBUG -DYY_ALWAYS_INTERACTIVE=1
LFLAGS      = -t -l
YFLAGS      = -d -v
YYFLAGS     = -W -N            

#################
# Program parameters
#################
CC          = g++
HDRS	    = $(INCLUDES)/SceneClass.h \
		      $(INCLUDES)/ObjectClass.h

#################
# Object files
################# 
OBJS = yytopp \
      $(OBJDIR)/sx_y.o \
      $(OBJDIR)/sx_l.o
#################
# Target Rules
#################
all:;       @echo "Nothing to do (run make from sparc or linux)"
 
yytopp: 	yytopp.l 
			$(LEX) $(LFLAGS) yytopp.l > yytopp.c
			gcc -Bstatic yytopp.c -o yytopp
			@rm -f yytopp.c

$(OBJDIR)/sx_y.o:     sx.y $(HDRS) 
			$(YACC) $(YFLAGS) sx.y
			cat y.tab.c | ./yytopp $(YYFLAGS) > sx_y.cpp
			@rm y.tab.c
			@mv y.tab.h sx_y.h
			$(CC) $(CFLAGS) -I$(OBJDIR) $(PPFLAGS) -c -o $(OBJDIR)/sx_y.o sx_y.cpp

$(OBJDIR)/sx_l.o:     sx.l 
			$(LEX) $(LFLAGS) sx.l | ./yytopp $(YYFLAGS) > sx_l.cpp
			$(CC) $(CFLAGS) -I$(OBJDIR) $(PPFLAGS) -c -o $(OBJDIR)/sx_l.o sx_l.cpp

build:  	 $(OBJS)
			
clean:;		@rm -fr .sb y.output lex.yy.c sx_y.h sx_y.cpp sx_l.cpp yytopp

#################
# dependencies
#################
yytopp:   yytopp.l
