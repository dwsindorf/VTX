// CraterClass.h

#ifndef _CRATERCLASS
#define _CRATERCLASS

#include "Placements.h"

class CraterMgr;

class Crater : public Placement
{
protected:
public:
	Crater(PlacementMgr&, Point4DL&,int);
	bool set_terrain(PlacementMgr &mgr);
};

class CraterMgr : public PlacementMgr 
{
protected:
public:
    static TNode *default_rnoise;
    static TNode *default_vnoise;
	double  noise_radial;
	double  noise_vertical;
	
	TNode   *rnoise;
	TNode   *vnoise;
  
    double  impact; 
	double  ampl;
	double  rim;
	double  flr;
	double  ctr;
	double  rise;
	double  drop;
	double  offset;
		
	Placement *make(Point4DL&,int);

	~CraterMgr();
	CraterMgr(int);
	void init();
	void eval();
	void reset();
};

//************************************************************
// Class TNcraters
//************************************************************
class TNcraters : public TNplacements
{
protected:
	int join(TNode *);
	int joined(TNode *);
	TNode *next();
public:
	TNcraters(int t, TNode *l, TNode *r, TNode *b);
	~TNcraters();
	void eval();
	void init();
	void applyExpr();	   
	int typeValue()			{ return ID_CRATERS;}
	const char *typeName()	{ return "craters";}
	void addToken(LinkedList<TNode*>&l);
	void valueString(char *);
};


#endif


