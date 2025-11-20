// TerrainMgr.h

#ifndef _TERRAINMGR
#define _TERRAINMGR
#include "NoiseClass.h"
#include "PointClass.h"
#include "ColorClass.h"
#include "TextureClass.h"
#include "TerrainNode.h"
#include "NodeIF.h"

class ExprMgr;
class TerrainMgr;
class TerrainData;
class TerrainSym;
class TNode;
class TNarg;
class Object3D;
class TNroot;
class TNsubr;
class TNvar;
class TNinode;


#define RANDID 1.237e-66

enum scope_codes{
	ID_MGR  	    = 0x00001000,
	ID_TMGR  	    = 0x00002000,
	ID_EXPMGR  	    = ID_SCOPE | ID_MGR,
	ID_TNMGR  		= ID_LEVEL9|ID_SCOPE | ID_MGR  | ID_TMGR,
	ID_VOLMGR  		= ID_SCOPE | ID_MGR  | ID_TMGR | 0x00000001
};


//************************************************************
// Class TNvar
//************************************************************
class TNvar : public TNunary
{
enum {
	SHOW_FLAG  = 0x01,
	EVAL_FLAG  = 0x02,
	CONST_FLAG = 0x04,
	REVAL_FLAG = 0x08,
	DIRTY_FLAG = 0x10,
	HT_FLAG    = 0x20
};

protected:
	int flags;
public:
	~TNvar();
	TNode  *expr;
	char  *_name;
	TNvar(char *n);
	TNvar(char *n, TNode *e);
	char *name()                             { return _name;}
	int  cmp(char *s)						{ return strcmp(_name,s);}

	void set_show(int i)					{ BIT_SET(flags,SHOW_FLAG,i);}
	int  do_show()		    				{ return flags&SHOW_FLAG;}

	void set_eval(int i)					{ BIT_SET(flags,EVAL_FLAG,i);}
	int  do_eval()		    				{ return flags&EVAL_FLAG;}

	void set_reval(int i)					{ BIT_SET(flags,REVAL_FLAG,i);}
	int  reval()		    				{ return flags&REVAL_FLAG;}

	void set_constant(int i)				{ BIT_SET(flags,CONST_FLAG,i);}
	int  constant()		    				{ return flags&CONST_FLAG;}

	void set_dirty(int i)					{ BIT_SET(flags,DIRTY_FLAG,i);}
	int  dirty()		    				{ return flags&DIRTY_FLAG;}

	void set_ht(int i)						{ BIT_SET(flags,HT_FLAG,i);}
	int  ht()		    					{ return flags&HT_FLAG;}

	virtual void dump()						{ printf("var: %s\n",_name);}
	virtual void eval();
	virtual void init();
	virtual void save(FILE *f);
	virtual void saveNode(FILE *f);
	virtual void valueString(char *s);
	virtual int typeValue()					{ return ID_VAR;}
	virtual const char *typeName()			{ return "var";}
	virtual const char *symbol()			{ return (const char *)_name;}
	virtual char *nodeName()				{ return _name; }
	virtual NodeIF *getValue()				{ return right;}
	virtual void setValue(NodeIF *n)		{ replaceChild(right,n);}
	virtual NodeIF *setValueString(char *s);
	virtual NodeIF *removeChild(NodeIF *c);
	virtual NodeIF *addChild(NodeIF *);
	virtual void addToken(LinkedList<TNode*>&l);
	virtual void applyExpr();
	virtual void clearExpr();
	virtual void setExpr(char *c);
	virtual TNode *getExprNode();
	virtual int isExprNode()                 { return 1;}
};

//************************************************************
// Class TNroot
//************************************************************
class TNroot : public TNvar
{
public:
	TNroot(TNode *r);
	TNroot(char*, TNode *r);
	~TNroot();
	virtual int typeValue()				{ return ID_ROOT;}
	virtual void save(FILE*);
	virtual const char *typeName()		{ return "terrain";}
	virtual char *nodeName()			{ return (char*)"";}
	virtual NodeIF *addChild(NodeIF *);
	virtual NodeIF *removeChild(NodeIF *);
	virtual NodeIF *addAfter(NodeIF *a,NodeIF *n);
};

//************************************************************
// Class Scope
//************************************************************

typedef struct scopedata {
	unsigned int  limits   : 1;	// scope mode
	unsigned int  tokens   : 1;	// scope mode
	unsigned int  shader   : 1;	// scope mode
	unsigned int  texture  : 1;	// scope mode
	unsigned int  init	   : 1;	// scope mode
	unsigned int  eval	   : 1;	// scope mode
	unsigned int  first    : 1;	// scope mode
	unsigned int  last     : 1;	// scope mode
	unsigned int  reval    : 1;	// scope mode
	unsigned int  preview  : 1;	// scope mode
	unsigned int  changed  : 1;	// scope mode
	unsigned int  twopass  : 1;	// height pass needed
	unsigned int  htpass   : 3;	// height pass
} scopedata;


class Scope
{
protected:
	scopedata flags;
public:
	NameList<TerrainSym*> vars;
	Scope  *parent;
	Scope()					{ parent=0;
							  set_init_mode(1);
							  set_eval_mode(0);
							  set_shader(0);
							  set_texture(0);
							  set_first(0);
							  set_preview(0);
							  set_zcpass();
							  set_twopass(0);
							  set_keep_limits(0);
							  set_tokens(0);
							  set_changed(0);
							}
	~Scope();

	void set_twopass(int c)	    { flags.twopass=c;}
	int twopass()				{ return flags.twopass;}

	void set_zcpass()	        { flags.htpass=0;}
	void set_zpass()	        { flags.htpass=1;}
	void set_cpass()	        { flags.htpass=2;}
	void set_rpass()	        { flags.htpass=3;}
	void set_tpass()	        { flags.htpass=4;}
	void set_spass()	        { flags.htpass=5;}

	int passmode()				{ return flags.htpass;}
	void set_passmode(int m)     { flags.htpass=m;}

	int zpass()				    { return flags.htpass==1;}
	int cpass()				    { return flags.htpass==2;}
	int rpass()				    { return flags.htpass==3;}
	int tpass()				    { return flags.htpass==4;}
	int spass()				    { return flags.htpass==5;}


	void set_init_mode(int c)	{ flags.init=c;}
	int init_mode()			    { return flags.init;}
	void set_eval_mode(int c)	{ flags.eval=c;}
	int eval_mode()			    { return flags.eval;}
	void set_first(int c)	    { flags.first=c;}
	int first()			        { return flags.first;}
	void set_reval(int c)	    { flags.reval=c;}
	int reval()			        { return flags.reval;}
	void set_preview(int c)	    { flags.preview=c;}
	int preview()			    { return flags.preview;}
	void set_tokens(int c)	    { flags.tokens=c;}
	int tokens()			    { return flags.tokens;}
	void set_shader(int c)	    { flags.shader=c;}
	int shader()			    { return flags.shader;}
	void set_texture(int c)	    { flags.texture=c;}
	int texture()			    { return flags.texture;}
	void set_keep_limits(int c)	{ flags.limits=c;}
	int keep_limits()			{ return flags.limits;}
	int changed()			    { return flags.changed;}
	void set_changed(int c)	    { flags.changed=c;}

	int find_var(char *s)	    { return vars.inlist(s)?1:0;}

	virtual TerrainSym *set_units(const char*,const char*);
	virtual TerrainSym *set_var(const char*,char*);
	virtual TerrainSym *set_var(const char*,double);
	virtual TerrainSym *set_var(const char*,Point);
	virtual TerrainSym *set_var(const char*,Color);

	virtual TerrainSym *hide_var(const char*);
	virtual TerrainSym *set_var(const char*,char*,int);
	virtual TerrainSym *set_var(const char*,double,int);
	virtual TerrainSym *set_var(const char*,Point,int);
	virtual TerrainSym *set_var(const char*,Color,int);
	virtual TerrainSym *set_var(char*,TerrainData&);
	virtual TerrainSym *get_var(char*);
	virtual TerrainSym *get_var(char*,TerrainData&);
	virtual TerrainSym *get_local(const char*,TerrainData&);
	virtual TerrainSym *get_local(const char*);
	virtual void set_show(char*, int);
	virtual void set_eval(char*, int);

	virtual void free_var(char *s);
	virtual void save(FILE*);
	virtual void eval_node(TNode*);
	virtual void free();
	virtual void revaluate();
	virtual TNvar *get_expr(char*);
	virtual void applyVarExprs();
	virtual void clearVarExprs();
	virtual void setPreviewMode();
	virtual void setStandardMode();
	virtual void setAdaptMode();
};

//************************************************************
// Class ExprMgr
//************************************************************
class ExprMgr : public Scope, public NodeIF
{
	friend class MapNode;
protected:
	NodeIF *object;
	int     dflag;
public:

	LinkedList<TNode*>      exprs;
	LinkedList<TNode*>      inodes;
	ExprMgr() : Scope()			{ dflag=0;}
	~ExprMgr();

	void set_debug(int d)		{ dflag=d;}
	int  debug()				{ return dflag;}

	virtual void free();
	virtual void init();
	virtual void eval();
	virtual void save(FILE*);
	virtual void revaluate();

	void save_vars(FILE*f);
	void save_selected(FILE*f);
	TNode  *add_expr(char *, TNode *r);
	TNvar  *setVar(char *s, char *);
	TNvar  *removeVar(char *);
	TNvar  *removeExprVar(char *);
	TNvar  *getVar(char *);

	TNode    *add_image(TNode *r);
	TNinode  *get_image(char *r);
	TNinode  *replace_image(TNinode *i, TNinode *r);

	// NodeIF methods

	void setParent(NodeIF *p)   		{ object=p;}
	NodeIF *getParent()					{ return object;}
	virtual int typeValue()				{ return ID_EXPMGR;}
	virtual const char *typeName()		{ return "ExprMgr";}
	virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual NodeIF *addChild(NodeIF *);
	virtual NodeIF *addAfter(NodeIF *a,NodeIF *n);
	virtual NodeIF *removeChild(NodeIF *);
	virtual NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	virtual void invalidate();
	virtual TNvar *get_expr(char*);
	virtual TNvar *getExprVal(char*, TerrainData &f);
	virtual TNvar *setExprVal(char*, TerrainData &f);
	virtual void applyVarExprs();
	virtual void clearVarExprs();
	virtual void applyVarExpr(char *);
	virtual void clearVarExpr(char *);
	virtual int setVarExpr(char *n,char *v);
	virtual int getVarExpr(char *n,char *v);
	virtual void setPreviewMode();
	virtual void setStandardMode();
};

//************************************************************
// Class TerrainMgr
//************************************************************
class TerrainMgr : public ExprMgr
{
	friend class MapNode;
	int var_index;
    char name_str[64];
public:
	TNroot  *root;
	TerrainMgr();
	~TerrainMgr();
	virtual TNode *set_root(TNode *r);
	virtual TNode *get_root();
	virtual void clr_root(){root=0;}
	virtual void set_surface(TerrainData&);
	virtual void init();
	virtual void eval();
	virtual void save(FILE*);
	virtual bool hasChildren();
	virtual TNode *getChild(int );
	virtual bool hasChild(int );
	virtual bool hasChild(int t, bool enabled);
	virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual int typeValue()				{ return ID_TNMGR;}
	virtual const char *typeName()		{ return "Surface";}
	virtual void printNode(char *s)     {  strcpy(s,"terrain");}
	virtual void setName(char *s) { strncpy(name_str,s,63);}
	virtual char *nodeName()  { return name_str;}

	void setVarIndex(int i){
		if(i<exprs.size)
			var_index=i;
		else
			var_index=0;
	}
	int getVarIndex(){
		if(var_index<exprs.size)
			return var_index;
		return 0;
	}
	void valueString(char *s);
	int valueValid();
	NodeIF* getValue();
	void setValue(NodeIF *n);
	NodeIF *addChild(NodeIF *);
	NodeIF *addAfter(NodeIF *a,NodeIF *n);
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	NodeIF *removeChild(NodeIF *);
	NodeIF *replaceNode(NodeIF *c);

	bool isLeaf()						{ return false;}

	void applyVarExprs();
	void clearVarExprs();
	void setPreviewMode();
	void setStandardMode();
	void setAdaptMode();
	void applyExpr();
	void clearExpr();
	void reset();
	void validateTextures();
	void validateVariables();
	void init_render();
	void saveNode(FILE *f);
	bool addTextureImage(char *name);

};


extern Scope   *CurrentScope;
extern int getargs(TNode *args, double *array, int max);
extern void push_scope(Scope*);
extern Scope* pop_scope();
#endif

