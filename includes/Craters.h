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
	double  noise_bias;
	double  noise_bias_min;
		
	Placement *make(Point4DL&,int);

	~CraterMgr();
	CraterMgr(int);
	void init();
	void eval();
	void reset();
};


#endif


