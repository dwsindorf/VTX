%{
/*==========================================================================
 *  MODULE:     yytopp.l
 *==========================================================================
 * PURPOSE:     multiple lex or yacc output file filter 
 *
 * Revision Control Info:
 * 		$Id$
 * 		$Author$
 * 		$Date$
 * 		$Locker$
 *=========================================================================*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define	BIT_ON(a,b)		a |= (b)
#define	BIT_OFF(a,b)	a &= ~(b)


static long status=0;

static char 		*ext="pp";
static char 		*outfile=0;
static char 		*infile=0;

#undef LINUX
#undef WINDOWS

enum {
	NOLINES    		= 1,
	WINDOWS    		= 2,
	LINUX    		= 4,
	SPARC    		= 8,
	YYTOPP    		= 0x10
};
	
/* =========================================================================
 *   TITLE: main() 
 * PURPOSE: main entry point 
 * =========================================================================*/
int    main(int argc, char **argv)
{	
	int	    i;

	yyout=stdout;
	yyin=stdin;
	
	for (i = 1; i < argc; i++) {
		if (argv[i][0]=='-'){
			switch (argv[i][1]){
			case 'N':
				BIT_ON(status,NOLINES);
				break;

			case 'L':
				BIT_ON(status,LINUX);
				break;
				
			case 'W':
				BIT_ON(status,WINDOWS);
				break;

			case 'S':
				BIT_ON(status,SPARC);
				break;
				
			case 'X':
				BIT_ON(status,YYTOPP);
				ext=argv[i]+2;
				break;

			case 'O':
				outfile=argv[i]+2;
				break;
			}
		}
		else{
			infile=argv[i];
			break;
		}			
	}
	if(outfile){
		yyout=fopen(outfile,"w");
		if(!yyout){
			printf("could not open file %s for output\n",outfile);
			return 1;
		}
	}
	if(infile){
		yyin=fopen(infile,"r");
		if(!yyin){
			printf("could not open file %s for input\n",infile);
			return 1;
		}
	}
	
	while(yylex()){
	}
	
	if(yyout!=stdout)
		fclose(yyout);
	if(yyin!=stdin)
		fclose(yyin);
	return 0;
}

int yywrap()
{
	return 1;
}

/******************************  LEX SECTION *******************************/
%}

%%
"yy_pop_state"				{
							    fprintf(yyout,"yy_pop_state");
							}							
"yy"						{	if(status & YYTOPP) 
									fprintf(yyout,"%s",ext);
								else
									fprintf(yyout,"%s",yytext);
							}
"uintptr_t"					{   if(!(status & WINDOWS)) 
									fprintf(yyout,"%s",yytext);
								else
									fprintf(yyout,"int");
							}
^"#pragma".*$				{	if(!(status & WINDOWS))
									fprintf(yyout,"%s",yytext);
                            }
^"#include <inttypes.h>"	{	if(!(status & WINDOWS))
									fprintf(yyout,"%s",yytext);
							}							
^"#include <values.h>"		{	if(!(status & WINDOWS))
									fprintf(yyout,"%s",yytext);
                            }
^"#include <unistd.h>"		{	if(!(status & WINDOWS))
									fprintf(yyout,"%s",yytext);
                            }
^"#include \"y.tab.h\""		{
								fprintf(yyout,"#include \"sx_y.h\"");
							}
^"	void exit(int);"		{	if(!(status & WINDOWS))
									fprintf(yyout,"%s",yytext);
                            }
^"int yyerror(const char *msg)"	{	if(status & SPARC) 
									fprintf(yyout,"void yyerror(YYCONST char *msg)");
								else
									fprintf(yyout,"%s",yytext);
							}				

^" yywrap()"				{	if(status & YYTOPP) 
									fprintf(yyout,"int %swrap()",ext);
								else
									fprintf(yyout,"int %s",yytext);
							}
%%
