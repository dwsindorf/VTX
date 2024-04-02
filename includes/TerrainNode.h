// TerrainNode.h

#ifndef _TERRAIN_NODE
#define _TERRAIN_NODE
#include "TerrainData.h"
#include "NodeIF.h"

class ExprMgr;
class TerrainMgr;
class TerrainData;
class TerrainSym;
class TNode;
class TNarg;
class Object3D;
class Orbital;
class TNsubr;
class TNvar;

extern double TNvalue(TNode *n);

enum {
	ID_EVAL         = 0x00000010,
	ID_BINARY     	= 0x00000020,
	ID_FUNC  		= 0x00000040,
	ID_BASE  	    = 0x00000080,
	ID_EXPR  	    = ID_BINARY | ID_EVAL,
	ID_ARG     		= ID_TERRAIN | ID_BINARY | 0x0001,
	ID_VAR  	    = ID_TERRAIN | ID_BINARY | 0x0002,
	ID_ROOT   	    = ID_TERRAIN | ID_BINARY | 0x0003,
	ID_VOXEL   	    = ID_TERRAIN | ID_BINARY | 0x0004,

	ID_CONST  	    = ID_TERRAIN | ID_EVAL | 0x0001,
	ID_GLOBAL  		= ID_TERRAIN | ID_EVAL | 0x0002,
	ID_STRING  	    = ID_TERRAIN | ID_EVAL | 0x0003,
	ID_ZBASE	 	= ID_TERRAIN | ID_EVAL | 0x0004,
	ID_NOISE     	= ID_TERRAIN | ID_EVAL | 0x0009,
	ID_INVRT   	    = ID_TERRAIN | ID_EVAL | 0x000A,
	ID_SUBR   	    = ID_TERRAIN | ID_EVAL | 0x000B,
	ID_IMAGE   	    = ID_TERRAIN | ID_EVAL | 0x000C,
	ID_BANDS   	    = ID_TERRAIN | ID_EVAL | 0x000D,
	ID_GROUP   	    = ID_TERRAIN | ID_EVAL | 0x000E,
	
	ID_ADD     		= ID_TERRAIN | ID_EXPR | 0x0002,
	ID_SUBTRACT     = ID_TERRAIN | ID_EXPR | 0x0003,
	ID_MULTIPLY     = ID_TERRAIN | ID_EXPR | 0x0004,
	ID_DIVIDE       = ID_TERRAIN | ID_EXPR | 0x0005,
	ID_GT           = ID_TERRAIN | ID_EXPR | 0x0006,
	ID_LT           = ID_TERRAIN | ID_EXPR | 0x0007,
	ID_GE           = ID_TERRAIN | ID_EXPR | 0x0008,
	ID_LE           = ID_TERRAIN | ID_EXPR | 0x0009,
	ID_EQ           = ID_TERRAIN | ID_EXPR | 0x000A,
	ID_NE           = ID_TERRAIN | ID_EXPR | 0x000B,
	ID_AND      	= ID_TERRAIN | ID_EXPR | 0x000C,
	ID_OR      	    = ID_TERRAIN | ID_EXPR | 0x000D,
	ID_COND     	= ID_TERRAIN | ID_EXPR | 0x000E,
	ID_CLIST     	= ID_TERRAIN | ID_EXPR | 0x000F,


	ID_TEXTURE  	= ID_LEVEL8|ID_TERRAIN | ID_EVAL | 0x0001,
	ID_SPRITE  	    = ID_LEVEL8|ID_TERRAIN | ID_EVAL | 0x0002,
	ID_PLANT  	    = ID_LEVEL8|ID_TERRAIN | ID_EVAL | 0x0003,
	ID_STEM  	    = ID_LEVEL8|ID_TERRAIN | ID_EVAL | 0x0004,
	ID_BRANCH  	    = ID_LEVEL8|ID_TERRAIN | ID_EVAL | 0x0005,
	ID_LEAF  	    = ID_LEVEL8|ID_TERRAIN | ID_EVAL | 0x0006,

	ID_COLOR 	    = ID_LEVEL7|ID_TERRAIN | ID_EVAL | 0x0007,
	ID_GLOSS 	    = ID_LEVEL7|ID_TERRAIN | ID_EVAL | 0x0008,

	ID_WATER  		= ID_LEVEL2|ID_TERRAIN | ID_FUNC | 0x0001,
	ID_FOG    	    = ID_LEVEL2|ID_TERRAIN | ID_FUNC | 0x0002,
	ID_ERODE      	= ID_TERRAIN | ID_FUNC | 0x0003,
	ID_HARDNESS     = ID_TERRAIN | ID_FUNC | 0x0004,
	ID_DENSITY      = ID_TERRAIN | ID_FUNC | 0x0005,
	ID_SNOW         = ID_LEVEL4|ID_TERRAIN | ID_FUNC | 0x0006,
	ID_CLOUD  	    = ID_LEVEL4|ID_TERRAIN | ID_FUNC | 0x0007,
	ID_MAP  		= ID_LEVEL5|ID_TERRAIN | ID_FUNC | 0x0008,
	ID_FCHNL  		= ID_TERRAIN | ID_FUNC | 0x0009,
	ID_PLACED  		= ID_TERRAIN | ID_FUNC | ID_BASE | 0x0000,
	ID_LAYER  		= ID_TERRAIN | ID_FUNC | ID_BASE | 0x0001,
	ID_CRATERS  	= ID_TERRAIN | ID_FUNC | ID_BASE | 0x0002,
	ID_ROCKS  	    = ID_LEVEL6|ID_TERRAIN | ID_FUNC | ID_BASE | 0x0003,
	ID_POINT	 	= ID_LEVEL7|ID_TERRAIN | ID_EVAL | 0x0005,

};

//************************************************************
// Class TNode
//************************************************************
class TNode : public NodeIF
{
protected:
	NodeIF *parent;
public:
	int start;
	int end;
	int middle;

	TNode()	{parent=0;start=end=middle=0;}
	static Orbital *getOrbital(NodeIF *obj);
	virtual ~TNode();
	virtual void dump();
	virtual void eval();
	virtual void init();
	virtual void save(FILE*);
	virtual const char *symbol()		{ return typeName();}
    virtual void evalNode(TerrainData &);
	virtual void visit(void (*f) (TNode *));
	virtual ExprMgr *getMgr();
	virtual ExprMgr *getExprMgr();
	virtual TNode *getExprNode()		{ return 0;}
	virtual TerrainMgr *getTerrainMgr();
	virtual void setToken(char *);
	virtual char *getToken();
	virtual void addToken(LinkedList<TNode*>&);
	virtual void setStart(char *);
	virtual void setMiddle(char *);
	virtual void setEnd(char *);
	virtual int getStart();
	virtual int getMiddle();
	virtual int getEnd();
	virtual Object3D *getObject();

	// NodeIF methods

	virtual NodeIF *setPropertyString(char *n);
	virtual NodeIF *setValueString(char *n);
	virtual void invalidate();
	virtual NodeIF *addChild(NodeIF *);
	virtual void saveNode(FILE *f);
	virtual void setParent(NodeIF *p)   { parent=p;}
	virtual NodeIF *getParent()			{ return parent;}
};

//************************************************************
// Class TNunary
//************************************************************
class TNunary : public TNode
{
public:
	TNode *right;
	TNunary(TNode *r);
	virtual ~TNunary();
	virtual void eval();
	virtual void init();
	virtual void save(FILE*);
	virtual void visit(void (*f) (TNode *));
	virtual bool hasChildren();
	virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual NodeIF *removeChild(NodeIF *c);
	virtual NodeIF *replaceChild(NodeIF *c, NodeIF *n);
	virtual void addToken(LinkedList<TNode*>&l);
	virtual NodeIF *getValue()		{ return this;}
	virtual void applyExpr();
	virtual void clearExpr();
};

//************************************************************
// Class TNinvert
//************************************************************
class TNinvert : public TNunary
{
public:
	TNinvert(TNode *r) :  TNunary(r) {}
	void eval();
	void save(FILE *f) 		{ fprintf(f,"-");right->save(f);}
	int typeValue()			{ return ID_INVRT;}
	const char *typeName ()	{ return "invert";}
	void valueString(char *);
	void addToken(LinkedList<TNode*>&);
};

//************************************************************
// Class TNgroup
//************************************************************
class TNgroup : public TNunary
{
public:
	TNgroup(TNode *r) : TNunary(r) {}
	void save(FILE *f) 		{
		fprintf(f,"(");
		if(right)
			right->save(f);
		fprintf(f,")");
	}
	int typeValue()			{ return ID_GROUP;}
	const char *typeName ()	{ return "group";}
	int linkable()      	{ return 1;}
	bool isLeaf()			{ return false;}

	void valueString(char *);
	void addToken(LinkedList<TNode*>&);
};

//************************************************************
// Class TNvector
//************************************************************
class TNvector : public TNunary
{
protected:
    char token[MAXSTR];
    TNvector *expr;
public:
	TNvector(TNode *r) : TNunary(r) {token[0]=0;expr=0;}
	virtual ~TNvector();
	virtual bool hasChildren()						{ return false; }
	virtual int getChildren(LinkedList<NodeIF*>&l)  { return 0; }
	virtual void valueString(char *);
	virtual NodeIF* getValue()			{ return this;}
	virtual void setToken(char *t)      { strcpy(token,t);}
	virtual char *getToken()			{ return token;}
	virtual void applyExpr();
	virtual void clearExpr();
	virtual void setExpr(char *c);
	virtual TNode *getExprNode()        { return expr;}
	virtual int isExprNode()            { return 1;}
	virtual void save(FILE*);
};

//************************************************************
// Class TNsubr
//************************************************************
class TNsubr : public TNvector
{
public:
    char   *name;
    ~TNsubr();
	TNsubr (char *s, TNode *r) : TNvector(r) { name=s;}
	virtual void eval();
	virtual void save(FILE *f);
	virtual int typeValue()			{ return ID_SUBR;}
    virtual const char *typeName()	{ return (const char*)name;}
	virtual const char *symbol()	{ return typeName();}
	virtual void addToken(LinkedList<TNode*>&l);

	//virtual char *nodeName()		        { return name;}
};

//************************************************************
// Class TNbinary
//************************************************************
class TNbinary : public TNunary
{
public:
	TNode *left;
	TNbinary(TNode *l, TNode *r) :  TNunary(r) {
		left=l;
		if(left)
			left->setParent(this);
	}
	TNbinary(TNode *r) : TNunary(r) { left=0;}
	virtual ~TNbinary();
	virtual void init();
	virtual void eval();
	virtual void save(FILE*);
	virtual void visit(void (*f) (TNode *));
	virtual int typeValue()		{ return ID_BINARY;}
	virtual bool hasChildren();
	virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual int isRightNode(NodeIF *t) { return (t && t==right)?1:0;}
	virtual int isLeftNode(NodeIF *t)  { return (t && t==left)?1:0;}
	virtual NodeIF *removeChild(NodeIF *c);
	virtual NodeIF *replaceChild(NodeIF *c, NodeIF *n);
	virtual NodeIF *addChild(NodeIF *c);
	virtual void valueString(char *);
	virtual void addToken(LinkedList<TNode*>&);
	virtual void applyExpr();
	virtual void clearExpr();
};

//************************************************************
// Class TNarg
//************************************************************
class TNarg : public TNbinary
{
public:
	TNarg(TNode *l, TNode *r);
	void dump();
	void eval();
	void save(FILE*);
	TNarg *next() 			{ return (TNarg*)right;}
	const char *typeName ()	{ return "arg";}
	int typeValue()			{ return ID_ARG;}
	void valueString(char *s);

	TNode *operator[](int i);
	TNarg *index(int i);
	void addToken(LinkedList<TNode*>&);
};

//************************************************************
// Class TNcond
//************************************************************
class TNcond : public TNbinary
{
	TNode *cond;
public:
	TNcond(TNode *l, TNode *r, TNode *c);
	~TNcond();
	void eval();
	void init();
	void save(FILE*);
	int typeValue()			{ return ID_COND;}
};

//************************************************************
// Class TNexpr
//************************************************************
class TNexpr : public TNbinary
{
public:
	TNexpr(TNode *l, TNode *r);
	virtual void valueString(char *);
	virtual const char *typeName ()	{ return "expr";}
	virtual int typeValue()		    { return ID_EXPR;}
	virtual void addToken(LinkedList<TNode*>&);
	void save(FILE *f);
	NodeIF* getValue()			{ return this;}
};

//************************************************************
// Class TNadd
//************************************************************
class TNadd : public TNexpr
{
public:
	TNadd(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	void valueString(char *);
	const char *symbol()    { return "+";}
	int typeValue()			{ return ID_ADD;}
};

//************************************************************
// Class TNsub
//************************************************************
class TNsub : public TNexpr
{
public:
	TNsub(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	void valueString(char *);
	const char *symbol()	{ return "-";}
	int typeValue()			{ return ID_SUBTRACT;}
};

//************************************************************
// Class TNmul
//************************************************************
class TNmul : public TNexpr
{
public:
	TNmul(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "*";}
	int typeValue()			{ return ID_MULTIPLY;}
};

//************************************************************
// Class TNdiv
//************************************************************
class TNdiv : public TNexpr
{
public:
	TNdiv(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "/";}
	int typeValue()			{ return ID_DIVIDE;}
};

//************************************************************
// Class TNgt
//************************************************************
class TNgt : public TNexpr
{
public:
	TNgt(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return ">";}
	int typeValue()			{ return ID_GT;}
};

//************************************************************
// Class TNlt
//************************************************************
class TNlt : public TNexpr
{
public:
	TNlt(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "<";}
	int typeValue()			{ return ID_LT;}
};

//************************************************************
// Class TNle
//************************************************************
class TNle : public TNexpr
{
public:
	TNle(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "<=";}
	int typeValue()			{ return ID_LE;}
};

//************************************************************
// Class TNge
//************************************************************
class TNge : public TNexpr
{
public:
	TNge(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return ">=";}
	int typeValue()			{ return ID_GE;}
};

//************************************************************
// Class TNeq
//************************************************************
class TNeq : public TNexpr
{
public:
	TNeq(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "==";}
	int typeValue()			{ return ID_EQ;}
};

//************************************************************
// Class TNne
//************************************************************
class TNne : public TNexpr
{
public:
	TNne(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "<>";}
	int typeValue()			{ return ID_NE;}
};

//************************************************************
// Class TNor
//************************************************************
class TNor : public TNexpr
{
public:
	TNor(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "||";}
	int typeValue()			{ return ID_OR;}
};

//************************************************************
// Class TNand
//************************************************************
class TNand : public TNexpr
{
public:
	TNand(TNode *l, TNode *r) :  TNexpr(l,r) {}
	void eval();
	const char *symbol()	{ return "&&";}
	int typeValue()			{ return ID_AND;}
};

//************************************************************
// Class TNfunc
//************************************************************
class TNfunc : public TNbinary
{
protected:
    char name_str[64];
    char token[64];
	TNfunc  *expr;
public:
	TNfunc(TNode *l, TNode *r) :  TNbinary(l,r) {token[0]=0;expr=0;name_str[0]=0;}
	~TNfunc();
	virtual bool hasChildren();
	virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual void save(FILE *);
	virtual void saveNode(FILE *);

	virtual void propertyString(char *s);
	virtual void valueString(char *s);
	virtual NodeIF* getValue()			    {return this;}
	virtual NodeIF* getProperties()			{return left;}
	virtual void setProperties(NodeIF *n)	{replaceChild(left,n);}
	virtual int typeValue()					{ return ID_FUNC;}
	virtual NodeIF *removeChild(NodeIF *c);
	virtual NodeIF *removeNode();
	virtual int valueValid()				{ return 0;}
	virtual int propertyValid()				{ return 1;}
	virtual NodeIF *addChild(NodeIF *c);
	virtual int linkable()      		    { return 1;}
	virtual void addToken(LinkedList<TNode*>&l);
	virtual void setToken(char *t)          { strcpy(token,t);}
	virtual char *getToken()			    { return token;}
	virtual void applyExpr();
	virtual void clearExpr();
	virtual void setExpr(char *c);
	virtual TNode *getExprNode()            { return expr;}
	virtual int isExprNode()                { return 1;}
	virtual void setName(char *s) { if(s)strncpy(name_str,s,63);else name_str[0]=0;}
	virtual char *nodeName()  { return name_str;}

};

//************************************************************
// Class TNbase
//************************************************************
class TNbase : public TNfunc
{
public:
    TNode *base;
    int type;
	TNbase(int t, TNode *l, TNode *r, TNode *b);
	~TNbase();
	virtual void visit(void (*f) (TNode *));
	virtual void valueString(char *s);
	virtual int typeValue()					{ return ID_BASE;}
	virtual bool hasChildren();
    virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual NodeIF *removeChild(NodeIF *c);
	virtual NodeIF *replaceChild(NodeIF *c, NodeIF *n);
	virtual void save(FILE *f);
	virtual void saveNode(FILE *f);
	virtual int optionString(char *);
	virtual void propertyString(char *s);
	virtual int branching()      		    { return 1;}
	virtual NodeIF *addChild(NodeIF *c);
	virtual void applyExpr();
	virtual void clearExpr();
	virtual void init();
	virtual void setValue(NodeIF *n)		{ replaceChild(base,n);}
};

extern int getargs(TNode *args, double *array, int max);
extern int getargs(TNode *args, TerrainData *array, int max);
extern int numargs(TNode *args);

#endif

