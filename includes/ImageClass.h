// ImageClass.h

#ifndef _IMAGE_CLASS
#define _IMAGE_CLASS

#include "TerrainMgr.h"
#include "ImageMgr.h"

class ExprMgr;
class TerrainSym;
class TNode;
class TerrainData;
class TNarg;
class Image;
class Texture;
class Object3D;

//~~~~~~~~~~~~~~~~~ unary nodes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//************************************************************
// Class TNinode
//************************************************************
class TNinode : public TNvector
{
protected:
public:
	int opts;
	Image *image;
	char  *name;
	TNinode(char *, int l, TNode *r);
	virtual ~TNinode();
	void setName(char*);
	static void optionString(char *, int);
	virtual void optionString(char *);
	virtual void valueString(char *);
	virtual void save(FILE *f);
	virtual void eval();
	virtual int itype()        { return 0;}
	virtual int t1d()          { return opts&T1D;}
	virtual bool texActive();
	virtual bool bumpActive();
	virtual bool hmapActive();
	virtual void setTexActive(bool);
	virtual void setBumpActive(bool);
	virtual void setHmapActive(bool);
	virtual char *nodeName()  { return name;}

};

//************************************************************
// Class TNtexture
//************************************************************
class TNtexture : public TNinode
{
public:
	Texture  *texture;
	bool tex_noise;
	TNtexture(char *, int l, TNode *d);
   ~TNtexture();
	void init();
	void eval();
	void save(FILE*);
	int typeValue()			{ return ID_TEXTURE;}
	const char *typeName ()	{ return "texture";}
	const char *symbol()	{ return "Texture";}
	void valueString(char *);
	void setExpr(char *c);
	void applyExpr();
	void setTexActive(bool);
	void setBumpActive(bool);
	bool initProgram();
	int exprString(TNarg *,char *nstr);

};

//************************************************************
// Class TNimage
//************************************************************
class TNimage : public TNinode
{
protected:
public:

	TNimage(char *s, int l, TNode *r);
	int typeValue()			    { return ID_IMAGE;}
	void save(FILE *f);
	void init();
    const char *typeName()	    { return "image";}
	const char *symbol()	    { return "image";}
	int itype()					{ return IMAGE;}
};

//************************************************************
// Class TNbands
//************************************************************
class TNbands :public TNinode
{
public:

	TNbands(char *s, int l, TNode *r);
	int typeValue()			    { return ID_BANDS;}
    const char *typeName()	    { return "bands";}
	const char *symbol()	    { return "bands";}
	void save(FILE *f);
	void init();
    void valueString(char *);
	int itype()					{ return BANDS;}
	int t1d()					{ return 1;}
};

#endif

