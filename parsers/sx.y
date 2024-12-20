%{
/*==========================================================================
 *  MODULE:     sx.y
 * PURPOSE:     sx yacc expression parser
 * Revision Control Info:
 * 		$Id$
 *=========================================================================*/
#include <iostream>

using namespace std;

#include "SceneClass.h"
#include "TerrainClass.h"
#include "Rocks.h"
#include "Craters.h"
#include "Sprites.h"
#include "Plants.h"
#include "States.h"
#include "Fractal.h"
#include "Erode.h"
#include "Layers.h"
#include "ImageClass.h"

static Orbital *OrbitalStack[256]={0};
static int orbi=0;
static TNode *args[256];
static int argi=0;
static int expr_mode=0;

int yylex();
//int yyerror(const char *msg);

void sx_error(char *msg,...);
#define yyerror(a)  sx_error(a)

#define ARG0  args[argi]
#define APUSH \
				ARG0=lastarg; \
				argi++; \
				if(argi>255) \
					sx_error("argument stack overflow"); \

#define APOP  	argi--; \
				if(argi<0) \
					sx_error("argument stack underflow"); \
				lastarg=ARG0


#define ORB0  OrbitalStack[orbi-1]
#define ORB1  (Orbital*)(orbi>1?OrbitalStack[orbi-2]:0)
#define OPUSH(x) \
				orbi++; \
				if(orbi>255) \
					sx_error("orbital stack overflow"); \
				ORB0=x;

#define OPOP  	orbi--; \
				if(orbi<0) \
					sx_error("orbital stack underflow");

static double m[10];
static int    mcnt=0;
static TNode  *lastarg=0;
static NodeIF *last_node=0;

NodeIF *get_node(){
    return last_node;
}
void set_node(NodeIF *node){
    last_node=node;
    lastarg=0;
    argi=0;
}

void clrorbs(){
	orbi=0;
	OrbitalStack[0]=0;
	lastarg=0;
}

void reset_parser(){
	set_node(0);
	clrorbs();
}

void set_orbital(Orbital *orb){
    if(orb){
		orbi=1;
		OrbitalStack[0]=orb;
	}
	else{
		orbi=0;
	}
	lastarg=0;
}

%}

%union{
	double              d;
	float               f;
	long                l;
	char               *s;
	class Color        *col;
	class LongSym 	   *ls;
	class Orbital 	   *orb;
	class TNode 	   *n;
}

/******************************  YACC SECTION ****************************/
%token YY_SCENE
%token YY_UNIVERSE YY_GALAXY YY_NEBULA YY_SYSTEM YY_STAR YY_PLANET YY_MOON
%token YY_CLOUD_LAYER YY_SHELL YY_RING YY_SKY YY_CORONA YY_HALO
%token YY_VIEWOBJ YY_DEBUG YY_INCLUDE YY_EXCLUDE
%token YY_COLOR YY_DENSITY YY_POINT YY_NOISE YY_VIEW  YY_VEXPR YY_SURFACE
%token YY_TERRAIN YY_IMAGE YY_BANDS
%token YY_MIX YY_WATER YY_CRATERS YY_MAP YY_TEXTURE YY_FRACTAL YY_FOG YY_SPRITE
%token YY_ERODE YY_HARDNESS YY_GLOSS YY_SNOW YY_ROCKS YY_COLORS YY_BASE YY_CLOUDS
%token YY_ROOT YY_BRANCH YY_LEAF YY_STEM
%token YY_OCEAN YY_LIQUID YY_SOLID
%token OR AND EQ NE GE LE

%token <s> NAME STRING
%token <d> VALUE UNIT
%token <l> NTYPE VTYPE LTYPE GTYPE ITYPE OTYPE PTYPE CTYPE ID

%type <d> value positive negative
%type <n> expr water_expr craters_expr color_expr density_expr layer_expr fog_expr
%type <n> ocean_expr liquid_expr solid_expr
%type <n> erosion_expr hardness_expr gloss_expr snow_expr rocks_expr clouds_expr base_expr
%type <n> group_expr map_expr const_expr string_expr var var_def fractal_expr
%type <n> noise_expr global_expr point_expr arg_list subr_expr texture_expr sprite_expr
%type <n> root_expr branch_expr leaf_expr
%type <n> inode_expr image image_expr bands_expr
%type <n> expr_item var_expr set_surface colors_expr
%type <l> ntype ltype itype ptype
%type <s> item_name

/* precedence order */

%right	'='
%left   '?' ':'
%left 	OR AND
%left	EQ NE '<' '>' GE LE
%right  ','
%right	'(' ')' '[' ']'
%left	'+' '-' '|'
%left	'*' '/'

%nonassoc UMINUS
%%

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~ ALL STATEMENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
program
	:  {clrorbs();}             scene
	|  {set_node(0);}           orbital
	|  {clrorbs();set_node(0);} expr_node

/*------------- orbital rules ---------------------------------------*/
orbital
    : orbital_item { set_node(OrbitalStack[orbi]);}
orbital_item
    : system
    | universe
    | galaxy
    | nebula
    | star
    | corona
    | halo
    | planet
    | moon
    | ring
    | clouds
    | sky

/*------------- expr_node rules ---------------------------------------*/
expr_node
    : expr_item { set_node($1);}
expr_item
    : expr
    | set_surface
    | inode_expr

/*------------- Scene level rules ---------------------------------------*/
scene
    : YY_SCENE '(' ')' '{' scene_body '}'
    						{ if(orbi!=0)
     							sx_error("orbital stack unbalanced");
     							if(OrbitalStack[orbi])
     								TheScene->add_orbital(OrbitalStack[orbi]);
     						}
scene_body
   : scene_item
   | scene_body scene_item

scene_item
    : system
    | universe
    | galaxy
    | view
    | NAME '=' expr	 ';'			{ TheScene->add_expr($1,$3);}

view
   : YY_VIEW '{' view_body '}'		{ TheScene->eval_view();}
   | YY_VIEW '{' '}'		        { }

view_body
   : view_item
   | view_body view_item

view_item
	: NAME '=' expr	 ';'			{ TheScene->view_expr($1,$3);}
	| YY_VEXPR '=' VTYPE ';'		{ TheScene->set_view($3);}
    | YY_VIEWOBJ '=' NAME ';'		{ TheScene->set_viewobj($3);}

view_object
    : YY_VIEWOBJ ';'				{ TheScene->set_viewobj(ORB0);}

/*------------- function rules -----------------------------------------*/

subr_expr
    : NAME '(' arg_list ')' 		{ $$=TheScene->subr_expr($1,$3);APOP;}

arg_list
    : 				        		{ APUSH;$$=0;lastarg=0;}
    | expr							{ APUSH;$$=lastarg=new TNarg($1,0); }
    | arg_list ',' expr				{ lastarg=new TNarg($3,lastarg);$$=$1;}


/*------------- universe rules ----------------------------------------------*/

universe
    : universe_def '{' universe_body '}' { OPOP;}
universe_def
    : YY_UNIVERSE  		 			{ OPUSH(new Universe(0));}
universe_body
	:
	| universe_exprs
universe_exprs
   	: universe_expr
   	| universe_exprs universe_expr
universe_expr
    : var_expr
    | galaxy

/*------------- galaxy rules ----------------------------------------------*/

galaxy
    : galaxy_def '{' galaxy_body '}' { OPOP;}
galaxy_def
    : YY_GALAXY '(' expr ')' 		 { OPUSH(new Galaxy(ORB1,TNvalue($3)));}
galaxy_body
	:
	| galaxy_exprs
galaxy_exprs
   	: galaxy_expr
   	| galaxy_exprs galaxy_expr
galaxy_expr
    : var_expr
    | surface
    | set_surface
    | inode_expr
    | system
    | nebula
    | corona
    | view_object

/*------------- nebula rules ----------------------------------------------*/

nebula
    : nebula_def '{' nebula_body '}' { OPOP;}
nebula_def
    : YY_NEBULA '(' expr ')' 		 {  OPUSH(new Nebula(ORB1,TNvalue($3))); }
nebula_body
	:
	| nebula_exprs
nebula_exprs
   	: nebula_expr
   	| nebula_exprs nebula_expr
nebula_expr
    : var_expr
    | surface
    | set_surface
    | inode_expr
    | corona
    | view_object

/*------------- System rules -------------------------------------*/
system
    : system_def  '{' system_body '}' { OPOP;}

system_def
    : YY_SYSTEM '(' value ')' 		  { OPUSH(new System(ORB1,$3)); }
system_body
   	:
   	| system_exprs
system_exprs
   	: system_expr
   	| system_exprs system_expr
system_expr
    : star
    | planet
    | var_expr
    | view_object

/*------------- Surface rules ----------------------------------------------*/

surface
    : surface_def '{' surface_body '}'	{ expr_mode=0;}
surface_def
	: YY_SURFACE						{ expr_mode=1;}
surface_body
   	:
   	| surface_exprs
surface_exprs
   	: surface_expr
   	| surface_exprs surface_expr
surface_expr
    : var_expr
    | set_surface

/*------------- Shell rules ----------------------------------------------*/

shell_body
   	:
   	| shell_exprs
shell_exprs
   	: shell_expr
   	| shell_exprs shell_expr
shell_expr
   	: orb_expr
   	| surface

/*------------- Sky rules ----------------------------------------------*/

sky
    : sky_def '{' shell_body '}'	{ OPOP;}

sky_def
    : YY_SKY '(' expr ')' 			{ OPUSH(new Sky(ORB1,TNvalue($3)));}

/*------------- CloudLayer rules ------------------------------------------*/

clouds
    : clouds_def '{' shell_body '}'	{ OPOP;}

clouds_def
    : YY_CLOUD_LAYER '(' expr ')' 		{ OPUSH(new CloudLayer(ORB1,TNvalue($3)));}


/*------------- Star rules ----------------------------------------------*/

star
    : star_def '{' star_body '}'	{ OPOP;}

star_def
    : YY_STAR '(' expr')'    		{ OPUSH(new Star(ORB1,TNvalue($3),0));}
    | YY_STAR '(' expr ',' expr ')'
    		{ OPUSH(new Star(ORB1,TNvalue($3),TNvalue($5)));}

star_body
   	:
   	| star_exprs
star_exprs
   	: star_expr
   	| star_exprs star_expr
star_expr
   	: orb_expr
   	| surface
   	| corona
   	| halo
 

/*------------- Corona rules ----------------------------------------------*/

corona
    : corona_def '{' shell_body '}'	{ OPOP;}

corona_def
    : YY_CORONA '(' expr ')' 	  { OPUSH(new Corona(ORB1,TNvalue($3)));}
    
/*------------- Halo rules ----------------------------------------------*/

halo
    : halo_def '{' shell_body '}'	{ OPOP;}

halo_def
    : YY_HALO '(' expr ')' 	  { OPUSH(new Halo(ORB1,TNvalue($3)));}

/*------------- Ring rules ----------------------------------------------*/

ring
    : ring_def '{' shell_body '}'	{ OPOP;}

ring_def
    : YY_RING '(' expr ',' expr ')'
    	{ OPUSH(new Ring((Planet*)ORB1,TNvalue($3),TNvalue($5)));}
    
/*------------- Planet rules --------------------------------------------*/

planet
    : planet_def '{' planet_body '}'{ OPOP;}

planet_def
	: YY_PLANET '(' expr ',' expr ')'
    	{ OPUSH(new Planet(ORB1,TNvalue($3),TNvalue($5)));}
planet_body
   	:
   	| planet_exprs
planet_exprs
   	: planet_expr
   	| planet_exprs planet_expr
planet_expr
    : shell_expr
    | moon
    | sky
    | clouds
    | ring

/*------------- Moon rules ----------------------------------------------*/

moon
    : moon_def '{' moon_body '}'{ OPOP;}

moon_def
    : YY_MOON '(' expr ',' expr ')'
    					{ OPUSH(new Moon((Planet*)ORB1,TNvalue($3),TNvalue($5)));}
moon_body
   	:
   	| moon_exprs
moon_exprs
    : moon_expr
    | moon_exprs moon_expr
moon_expr
    : shell_expr
    | sky
    | clouds

/*------------- Orbital level rules ------------------------------------*/

var_expr
	: NAME '=' expr	';'			{ if(orbi)
									 $$=ORB0->add_expr(expr_mode,$1,$3);
								  else
								     $$=new TNvar($1,$3);
								}
set_surface
    : YY_TERRAIN '=' expr  ';'	{ if(orbi)
									 $$=ORB0->set_terrain($3);
								  else
								     $$=new TNroot($3);
    							}

debug_expr
	: YY_DEBUG '(' positive ')' ';'	{ if(orbi) ORB0->set_debug((int)$3);}

orb_expr
    :  ';'
    | view_object
    | var_expr
    | inode_expr
    | debug_expr
    | set_surface
    | YY_INCLUDE ';'			{ ORB0->fg_include();}
    | YY_EXCLUDE ';'			{ ORB0->fg_exclude();}

/*------------- Variable rules -----------------------------------------*/
positive
    : VALUE                	{ $$=$1;}
    | VALUE UNIT			{ $$=$1*$2;}
negative
    : '-' VALUE            	{ $$=-$2;}
value
    : positive             	{ $$=$1;}
    | negative             	{ $$=$1;}

/*-------------   image rules --------------------------------*/
inode_expr
    : image ';'				{ if(orbi)
								 $$=ORB0->add_image($1);
							  else
							     $$=$1;
							}
image
    : bands_expr            { $$=$1;}
    | image_expr            { $$=$1;}

bands_expr
    : YY_BANDS '('  item_name ',' itype ',' arg_list ')'
    						{ $$=new TNbands($3,$5,$7);APOP;}
    | YY_BANDS '('  item_name ',' arg_list ')'
    						{ $$=new TNbands($3,0,$5);APOP;}
image_expr
    : YY_IMAGE '('  item_name ',' itype ',' arg_list ')'
    						{ $$=new TNimage($3,$5,$7);APOP;
    						}
    | YY_IMAGE '('  item_name ',' arg_list ')'
    						{ $$=new TNimage($3,0,$5);APOP;}
texture_expr
	: YY_TEXTURE '(' item_name ',' itype ',' arg_list ')'
    						{ $$=new TNtexture($3,$5,$7);APOP;}
	| YY_TEXTURE '(' item_name ',' arg_list ')'
    						{ $$=new TNtexture($3,0,$5);APOP;}
sprite_expr
	: YY_SPRITE '(' item_name ','  ptype ',' arg_list ')' expr
    						{ $$=new TNsprite($3, $5, $7, $9);APOP;}
    | YY_SPRITE '(' item_name ','  ptype ',' arg_list ')'
  						{ $$=new TNsprite($3, $5, $7,0);APOP;}
leaf_expr
    : YY_LEAF '(' arg_list ')' 
    						{$$=new TNLeaf($3, 0, 0);APOP;}
    | YY_LEAF '(' arg_list ')' '[' arg_list ']'
    						{ $$=new TNLeaf($3, 0, $6);APOP;}
    | YY_LEAF '(' arg_list ')' '[' arg_list ']' expr
    						{ $$=new TNLeaf($3, $8, $6);APOP;}	
    | YY_LEAF '(' arg_list ')' expr
    						{ $$=new TNLeaf($3, $5, 0);APOP;}	

branch_expr
    : YY_BRANCH '(' arg_list ')'
    						{ $$=new TNBranch($3, 0, 0);APOP;}
    | YY_BRANCH '(' arg_list ')' '[' arg_list ']'
    						{ $$=new TNBranch($3, 0, $6);APOP;}
    | YY_BRANCH '(' arg_list ')' '[' arg_list ']' expr
    						{ $$=new TNBranch($3, $8, $6);APOP;}	
    | YY_BRANCH '(' arg_list ')' expr
    						{ $$=new TNBranch($3, $5, 0);APOP;}	
root_expr
    : YY_ROOT '(' arg_list ')'
    						{ $$=new TNplant($3, 0);APOP;}
 
    | YY_ROOT '(' arg_list ')' expr
    						{ $$=new TNplant($3, $5);APOP;}

item_name
    : NAME                  { $$=$1;}
    | STRING                { $$=$1;}

/*-------------   expression rules --------------------------------*/

expr
    : water_expr        	{ $$=$1;}
    | ocean_expr            { $$=$1;}
    | erosion_expr          { $$=$1;}
    | hardness_expr         { $$=$1;}
    | gloss_expr            { $$=$1;}
    | base_expr             { $$=$1;}
    | snow_expr             { $$=$1;}
    | craters_expr          { $$=$1;}
    | rocks_expr            { $$=$1;}
    | clouds_expr           { $$=$1;}
    | fog_expr         		{ $$=$1;}
    | texture_expr          { $$=$1;}
    | sprite_expr           { $$=$1;}
    | root_expr             { $$=$1;}
    | branch_expr           { $$=$1;}
    | leaf_expr             { $$=$1;}
    | map_expr              { $$=$1;}
    | color_expr            { $$=$1;}
    | density_expr          { $$=$1;}  
    | colors_expr           { $$=$1;}
    | const_expr            { $$=$1;}
    | string_expr           { $$=$1;}
    | layer_expr            { $$=$1;}
    | fractal_expr          { $$=$1;}
    | noise_expr			{ $$=$1;}
    | point_expr			{ $$=$1;}
    | subr_expr             { $$=$1;}
    | global_expr			{ $$=$1;}
	| group_expr			{ $$=$1;}
	| var				    { $$=$1;}
	| var_def				{ $$=$1;}
    | expr '+' expr         { $$=new TNadd($1,$3);}
    | expr '-' expr         { $$=new TNsub($1,$3);}
    | expr '*' expr         { $$=new TNmul($1,$3);}
    | expr '/' expr         { $$=new TNdiv($1,$3);}
    | expr '<' expr         { $$=new TNlt($1,$3);}
    | expr '>' expr         { $$=new TNgt($1,$3);}
    | expr GE expr          { $$=new TNge($1,$3);}
    | expr LE expr          { $$=new TNle($1,$3);}
    | expr EQ expr          { $$=new TNeq($1,$3);}
    | expr NE expr          { $$=new TNne($1,$3);}
    | expr OR expr  		{ $$=new TNor($1,$3);}
    | expr AND expr 		{ $$=new TNand($1,$3);}
	| '-' expr %prec UMINUS	{ $$=new TNinvert($2);}
    | expr '?' expr ':' expr { $$=new TNcond($3,$5,$1);}

 group_expr
	: '(' expr ')' 			{ $$=new TNgroup($2);}
	| '(' ')' 			    { $$=new TNgroup(0);}

 var_def
	: NAME '=' expr			{ $$=new TNvar($1,$3);}
 var
	: NAME 					{ $$=new TNvar($1);}

 fog_expr
    : YY_FOG '(' arg_list ')' expr
    						{ $$=new TNfog($3,$5);APOP;}
    | YY_FOG '(' arg_list ')'
    						{ $$=new TNfog($3,0);APOP;}
 hardness_expr
    : YY_HARDNESS '(' arg_list ')' expr
    						{ $$=new TNhardness($3,$5);APOP;}
    | YY_HARDNESS '(' arg_list ')'
    						{ $$=new TNhardness($3,0);APOP;}
 snow_expr
    : YY_SNOW '(' arg_list ')' expr
    						{ $$=new TNsnow($3,$5);APOP;}
    | YY_SNOW '(' arg_list ')'
    						{ $$=new TNsnow($3,0);APOP;}
 water_expr
    : YY_WATER '(' arg_list ')' expr
    						{ $$=new TNwater($3,$5);APOP;}
    | YY_WATER '(' arg_list ')'
    						{ $$=new TNwater($3,0);APOP;}

liquid_expr
    : YY_LIQUID '(' arg_list ')'
    						{ $$=new LiquidState($3);APOP;}
solid_expr
    : YY_SOLID '(' arg_list ')'
    						{ $$=new SolidState($3);APOP;}

ocean_expr
    : YY_OCEAN '(' arg_list ')' '[' liquid_expr ',' solid_expr ']'
    						{ $$=new OceanState($3,$6,$8);APOP;}
 
clouds_expr
    : YY_CLOUDS '(' arg_list ')' expr
    						{ $$=new TNclouds($3,$5);APOP;}
    | YY_CLOUDS '(' arg_list ')'
							{ $$=new TNclouds($3,0);APOP;}

 rocks_expr
    : YY_ROCKS '(' arg_list ')' expr
    						{ $$=new TNrocks(0,$3,$5,0);APOP;}
    | YY_ROCKS '(' arg_list ')'
							{ $$=new TNrocks(0,$3,0,0);APOP;}
    | YY_ROCKS '(' arg_list ')' '[' expr ']'
    						{ $$=new TNrocks(0,$3,0,$6);APOP;}
    | YY_ROCKS '(' arg_list ')' '[' expr ']' expr
    						{ $$=new TNrocks(0,$3,$8,$6);APOP;}
    | YY_ROCKS '(' ptype ',' arg_list ')'
							{ $$=new TNrocks($3,$5,0,0);APOP;}
    | YY_ROCKS '(' ptype ',' arg_list ')' expr
    						{ $$=new TNrocks($3,$5,$7,0);APOP;}
    | YY_ROCKS '(' ptype ',' arg_list ')' '[' expr ']'
    						{ $$=new TNrocks($3,$5,0,$8);APOP;}
    | YY_ROCKS '(' ptype ',' arg_list ')' '[' expr ']' expr
    						{ $$=new TNrocks($3,$5,$10,$8);APOP;}
 craters_expr
    : YY_CRATERS '(' arg_list ')' expr
    						{ $$=new TNcraters(0,$3,$5,0);APOP;}
    | YY_CRATERS '(' arg_list ')'
							{ $$=new TNcraters(0,$3,0,0);APOP;}
    | YY_CRATERS '(' arg_list ')' '[' expr ']'
    						{ $$=new TNcraters(0,$3,0,$6);APOP;}
    | YY_CRATERS '(' arg_list ')' '[' expr ']' expr
    						{ $$=new TNcraters(0,$3,$8,$6);APOP;}
    | YY_CRATERS '(' ptype ',' arg_list ')' expr
    						{ $$=new TNcraters($3,$5,$7,0);APOP;}
    | YY_CRATERS '(' ptype ',' arg_list ')'
							{ $$=new TNcraters($3,$5,0,0);APOP;}
    | YY_CRATERS '(' ptype ',' arg_list ')' '[' expr ']'
    						{ $$=new TNcraters($3,$5,0,$8);APOP;}
    | YY_CRATERS '(' ptype ',' arg_list ')' '[' expr ']' expr
    						{ $$=new TNcraters($3,$5,$10,$8);APOP;}
 ptype
	: PTYPE					{ $$=$1;}
	| ID					{ $$=$1;}
	| ptype '|' PTYPE		{ $$=$1|$3;}

 fractal_expr
    : YY_FRACTAL '(' arg_list ')'
    						{ $$=new TNfractal(0,$3,0);APOP;}
    | YY_FRACTAL '(' ntype ',' arg_list ')'
    						{ $$=new TNfractal($3,$5,0);APOP;}
    | YY_FRACTAL '(' arg_list ')' expr
    						{ $$=new TNfractal(0,$3,$5);APOP;}
    | YY_FRACTAL '(' ntype ',' arg_list ')' expr
    						{ $$=new TNfractal($3,$5,$7);APOP;}

 erosion_expr
    : YY_ERODE '(' arg_list ')'
    						{ $$=new TNerode(0,$3,0);APOP;}
    | YY_ERODE '(' ntype ',' arg_list ')'
    						{ $$=new TNerode($3,$5,0);APOP;}
    | YY_ERODE '(' arg_list ')' expr
    						{ $$=new TNerode(0,$3,$5);APOP;}
    | YY_ERODE '(' ntype ',' arg_list ')' expr
    						{ $$=new TNerode($3,$5,$7);APOP;}

 map_expr
    : YY_MAP '(' arg_list ')' expr
    						{ $$=new TNmap($3,$5);APOP;}
    | YY_MAP '(' arg_list ')'
    						{ $$=new TNmap($3,0);APOP;}
 layer_expr
    : YY_MIX '(' ltype ',' arg_list ')' '[' expr ']'
    						{ $$=new TNlayer(0, $3,$5,0,$8);APOP;}
    | YY_MIX '(' ltype ',' arg_list ')' '[' expr ']' expr
    						{ $$=new TNlayer(0, $3,$5,$10,$8);APOP;}
    | YY_MIX '(' ltype ',' arg_list ')'
    						{ $$=new TNlayer(0, $3,$5,0,0);APOP;}
    | YY_MIX '(' ltype ')'
    						{ $$=new TNlayer(0, $3,0,0,0);}
    | YY_MIX '(' ltype ')'  '[' expr ']'
    						{ $$=new TNlayer(0, $3,0,0,$6);}		
	| YY_MIX '(' item_name ',' ltype ',' arg_list ')' '[' expr ']'
    						{ $$=new TNlayer($3, $5,$7,0,$10);APOP;}
    | YY_MIX '(' item_name ',' ltype ',' arg_list ')' '[' expr ']' expr
    						{ $$=new TNlayer($3, $5,$7,$12,$10);APOP;}
    | YY_MIX '(' item_name ',' ltype ',' arg_list ')'
    						{ $$=new TNlayer($3, $5, $7,0,0);APOP;}
    | YY_MIX '(' item_name ',' ltype ')'
    						{ $$=new TNlayer($3, $5,0,0,0);}
    | YY_MIX '(' item_name ',' ltype ')'  '[' expr ']'
    						{ $$=new TNlayer($3, $5,0,0,$8);}
 ltype
	: LTYPE					{ $$=$1;}
	| ltype '|' LTYPE		{ $$=$1|$3;}

 itype
	: ITYPE					{ $$=$1;}
	| itype '|' ITYPE		{ $$=$1|$3;}

 /*~~~~~~~~~~~~~~~~~~~~~ terminal nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

 const_expr
    : positive              { $$=new TNconst($1);}

 string_expr
    : STRING                { $$=new TNstring($1);}

 global_expr
    : GTYPE                 { $$=new TNglobal($1);}

 color_expr
    : YY_COLOR '(' arg_list ')'
                            { $$=new TNcolor($3);APOP;}
 density_expr
    : YY_DENSITY '(' arg_list ')'
                            { $$=new TNdensity($3);APOP;}
 colors_expr
    : YY_COLORS '(' arg_list ')'
                            { $$=new TNclist($3);APOP;}
 point_expr
    : YY_POINT '(' arg_list ')'
    						{ $$=new TNpoint($3);APOP;}
 gloss_expr
    : YY_GLOSS '(' arg_list ')'
    						{ $$=new TNgloss($3);APOP;}
 base_expr
    : YY_BASE '(' arg_list ')'
    						{ $$=new TNzbase($3);APOP;}
 noise_expr
    : YY_NOISE '(' ntype ',' arg_list ')'
    						{ $$=new TNnoise($3,$5);APOP;}
    | YY_NOISE '(' arg_list ')'
    						{ $$=new TNnoise(0,$3);APOP;}
 ntype
	: NTYPE					{ $$=$1;}
	| ntype '|' NTYPE		{ $$=$1|$3;}
	| ntype '|' NAME		{ $$=$1;sx_error("illegal Noise symbol:",$3);}

%%


/* ========================================================================
 * TITLE:   yyerror()
 * PURPOSE: yacc error handler (default syntax errors)
 * =======================================================================*/
//int yyerror(const char *msg)
//{
//	sx_error("%s\n",msg);
//}
