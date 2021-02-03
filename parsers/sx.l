%{
/*==========================================================================
 *  MODULE: sx.l
 * PURPOSE:	hex file or char* parser
 * COMMENT:	coupled	with hex.y hex.h
 * Revision Control Info:
 * 		$Id$
 *=========================================================================*/
 
#define LASTCHAR (yytext[yyleng-1])
#define FIRSTCHAR (yytext[0])
#define BEGINSTATE ((int)(yybgin-yysvec-1))

#include "SceneClass.h"

#include "sx_y.h"
#include "defs.h"

#define MAXSXLINE 256

static void init_parser();

class LexSym {
public:	
	const char			*text;
	YYSTYPE				value;
	int 				rtype;  // return type
	LexSym(const char *t,int r){
	    text=t;
	    rtype=r;
	    value.d=0;
	    value.l=r;
	} 
	LexSym(const char *t,int r, int l){
	    text=t;
	    rtype=r;
	    value.d=0;
	    value.l=l;
	} 
	LexSym(const char *t,int r, double d){
	    text=t;
	    rtype=r;
	    value.d=d;
	} 
	char *name()		{ 	return (char*)text;}
	int  cmp(char *s)	{	return strcmp(text,s);}
};

int				SX_errors;
static int initialized=0;
static NameList<LexSym*> LexSymbols(200);

/*---------------------  yacc_hex.y function prototypes --------------------*/

extern int 			yyparse();

/*---------------------  extern parameters ---------------------------------*/

#define isatty(x) (1)
#define INITIAL 0

extern YYSTYPE      	yylval;

/*---------------------  static function prototypes ------------------------*/

static void 			newstring(char *);
static void 			setfile(char *);
static void 			setline(char *);

/*---------------------  static parameters ---------------------------------*/

static char            *S_yyin;
static int             S_last_line;
static char            *S_last_file;
static int             state_stack[32];
static int              old_state=INITIAL;
static char             lastfile[256];
static char             lastline[MAXSXLINE+1];

static LongSym keytbl[]={ 
	{"Scene",		YY_SCENE},
	{"System",		YY_SYSTEM},
	{"Star",		YY_STAR},
	{"Corona",		YY_CORONA},
	{"Planet",		YY_PLANET},
	{"Moon",		YY_MOON},
	{"Clouds",		YY_CLOUD_LAYER},
	{"Sky",			YY_SKY},
	{"Nebula",		YY_NEBULA},
	{"Universe",	YY_UNIVERSE},
	{"Galaxy",		YY_GALAXY},
	{"Surface",		YY_SURFACE},
	{"Volume",		YY_SURFACE},
	{"Ring",		YY_RING},
	{"surface",		YY_TERRAIN},	
    {"terrain",     YY_TERRAIN},
    {"voxel",       YY_TERRAIN},
	
	{"Color",		YY_COLOR},	
	{"Colors",		YY_COLORS},	
	{"Point",		YY_POINT},
	{"Z",           YY_POINT},
	{"Texture",		YY_TEXTURE},
	{"Density",		YY_DENSITY},	
		
	{"debug",		YY_DEBUG},	

	{"noise",       YY_NOISE},
	{"water",       YY_WATER},
	{"fractal",     YY_FRACTAL},
	{"erode",      	YY_ERODE},
	{"hardness",    YY_HARDNESS},
	{"gloss",       YY_GLOSS},
	{"base",        YY_BASE},
	{"snow",        YY_SNOW},
	{"craters",     YY_CRATERS},
	{"rocks",       YY_ROCKS},
	{"clouds",      YY_CLOUDS},
	{"map",         YY_MAP},
	{"fog",         YY_FOG},
    {"layer",       YY_MIX},
    {"image",       YY_IMAGE},
    {"bands",       YY_BANDS},
	
	{"view",		YY_VEXPR},
	{"View",		YY_VIEW},
	{"viewobj",		YY_VIEWOBJ},
	{"include",		YY_INCLUDE},
	{"exclude",		YY_EXCLUDE}
};
NameList<LongSym*> keywrds(keytbl,sizeof(keytbl)/sizeof(LongSym));

extern NameList<LongSym*>   POpts;
extern NameList<LongSym*>   NTypes;
extern NameList<LongSym*>   NOpts;
extern NameList<LongSym*>   IOpts;
extern NameList<LongSym*>   ITypes;
extern NameList<LongSym*>   TOpts;
extern NameList<LongSym*>   TXTypes;
extern NameList<LongSym*>   LTypes;
extern NameList<LongSym*>   LOpts;
extern NameList<LongSym*>   NGlobals;
extern NameList<LongSym*>   Vtypes;
extern NameList<DoubleSym*> Units;

/******************************  LEX SECTION *******************************/	
%}

%START STD
%START STR
%START COMM


digit									[0-9]
ws										[ \t]
word									[a-zA-Z_][a-zA-Z_0-9\.]*
id										ID[0-9]+
string									[\"][^\"\n]*[\"]
delimiter								[\:\(\)\{\}\[\]\;\,\=\+\-\>\<\|\*\/\?]
sign									[\-\+]
e_exponent								[eE]
dot										[\.]
lineinfo								[0-9]+:[0-9]+.*\n
fileinfo								"file:"[0-9]+[^\n]+[\n]
fileline								"file:"[0-9]+:[0-9]+.*\n
%%
%{
%}
^{fileline}								;
^{fileinfo}							{	LASTCHAR=0;setfile(yytext);}
^{lineinfo}							{	LASTCHAR=0;setline(yytext);}
<COMM>.								;
"*/"							    {   BEGIN STD;}
\n                                  {   S_last_line++;}
"/*"								{   BEGIN COMM; }
<STD>"//".*							;
<STD>"!".*							;
<STD>{ws}*                          ;
<STD>{string}						{   LASTCHAR=0;
										newstring(yytext+1);
										return STRING;
									}
<STD>">="							{	return GE;}
<STD>"<="							{	return LE;}
<STD>"=="							{	return EQ;}
<STD>"<>"							{	return NE;}
<STD>"||"							{	return OR;}
<STD>"&&"							{	return AND;}
<STD>{delimiter}					{	return FIRSTCHAR;}
<STD>{id}							{ 	yylval.l=atol(yytext+2);
										return ID;
									}
<STD>{word}                         {
										LexSym *lexsym;
										if(lexsym=LexSymbols.inlist(yytext)){
										    yylval=lexsym->value;
										    return lexsym->rtype;
										}
										else{
											newstring(yytext);
											return NAME;
										}
									}
<STD>{digit}+	|
<STD>{digit}+{dot}?{digit}*{e_exponent}{sign}?{digit}+ 	|
<STD>{digit}*{dot}{digit}+			{
										yylval.d=atof(yytext);
										return VALUE;
									}
.									;
%%


/****************************** SUBR SECTION *******************************/

static void add_symbols(NameList<LongSym*> &list)
{
    LexSym *sym;
    for(int i=0;i<list.size;i++){
        sym=new LexSym(list.base[i]->text,list.base[i]->value);
        LexSymbols.add(sym);
    }
}
static void add_symbols(NameList<LongSym*> &list, int rtype)
{
    LexSym *sym;
    for(int i=0;i<list.size;i++){
        sym=new LexSym(list.base[i]->text,rtype,list.base[i]->value);
        LexSymbols.add(sym);
    }
}
static void add_symbols(NameList<DoubleSym*> &list, int rtype)
{
    LexSym *sym;
    for(int i=0;i<list.size;i++){
        sym=new LexSym(list.base[i]->text,rtype,list.base[i]->value);
        LexSymbols.add(sym);
    }
}
 
/* =========================================================================
 * TITLE:   init_parser()
 * PURPOSE: initialize parser
 * =========================================================================*/
static void init_parser()
{
    add_symbols(keywrds);
    add_symbols(POpts,PTYPE);
    add_symbols(NTypes,NTYPE);
    add_symbols(NOpts,NTYPE);
    add_symbols(IOpts,ITYPE);
    add_symbols(ITypes,ITYPE);
    add_symbols(LTypes,LTYPE);
    add_symbols(LOpts,LTYPE);
    add_symbols(NGlobals,GTYPE);
    add_symbols(Vtypes,VTYPE);
    add_symbols(Units,UNIT);
    LexSymbols.pack();
    LexSymbols.sort();
    //printf("lexsyms=%d\n",LexSymbols.size);    
	initialized=1;	
}

/* =========================================================================
 * TITLE:   destroy_parser()
 * PURPOSE: free parser resources
 * =========================================================================*/
void destroy_parser()
{
    printf("destroy_parser() %d\n",LexSymbols.size);
    LexSymbols.free();
}

/* =========================================================================
 * TITLE:   setfile()
 * PURPOSE: set last file name for error reporting
 * =========================================================================*/
void setfile(char *name)
{	
	strcpy(lastfile,name);
}

/* =========================================================================
 * TITLE:   setline()
 * PURPOSE: save a line string
 * =========================================================================*/
static void setline(char *text)
{
	strncpy(lastline,text,MAXSXLINE);
}

/* =========================================================================
 * TITLE:   sx_error()
 * PURPOSE: yacc error handler (explicit errors)
 * =========================================================================*/
void sx_error(char *msg,...)
{
	extern void DisplayErrorMessage(const char *format,...);
    extern void reset_parser();
	

	va_list             xp;
	char				ermsg[512];

	ermsg[0]=0;
	
	va_start(xp, msg);
	sprintf(ermsg,"** ");
	vsprintf(ermsg+3,msg, xp);
	
	va_end(xp);

	DisplayErrorMessage("%s %s\n",ermsg,yytext);
	
	SX_errors++;
	reset_parser();
	YY_FLUSH_BUFFER;
}

/* =========================================================================
 * TITLE:   newstring()
 * PURPOSE: save a new string
 * =========================================================================*/
static void newstring(char *start)
{	
	MALLOC(strlen(start)+1,char,yylval.s);
	strcpy(yylval.s,start);
}

/* =========================================================================
 * TITLE:  	parse_file()
 * PURPOSE:	read and parse a file(name)
 * =========================================================================*/
int parse_file(char *name)
{
	FILE   	*the_file;
	char 	*ppout=name;
	int 	 n=0;
	if(!initialized)
		init_parser();
	//printf("parsing %s\n",name);
	S_last_file=name;

	if (!(the_file = fopen(ppout, "r"))) {
		printf("source file: %s not found\n", ppout);
		return 0;
	}
	yyin = the_file;
	S_yyin=0;
	S_last_line=0;
		
	SX_errors=0;	
	
	BEGIN STD;
	lastfile[0]=0;
	lastline[0]=0;
	
	yyparse();
	
	fclose(the_file);

	if(SX_errors)
		return 0;
	return 1;
}

/* =========================================================================
 * TITLE:   yywrap()
 * PURPOSE: do end-of-stream processing
 * =========================================================================*/
 yywrap()
 {
	return (1);
 }
