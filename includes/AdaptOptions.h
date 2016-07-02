// AdaptOptions.h

#ifndef _ADAPT_OPTIONS
#define _ADAPT_OPTIONS

#include "defs.h"
#include "ColorClass.h"

typedef struct adaptdata {
	uint  tests		: 16;	// tests
	uint  tlvl_sharp	: 3;	// tlvl option
	uint  tlvl_normal	: 3;	// tlvl option
	uint  tlvl_smooth	: 3;	// tlvl option
	uint  tlvl_hidden	: 3;	// tlvl option
} adaptdata;

class AdaptOptions
{
enum{
	BACKTEST		= 0x0001,
	CLIPTEST		= 0x0002,
	OCCLTEST		= 0x0004,
	CURVETEST		= 0x0008,
	ALWAYS		    = 0x0010,
	NEVER		    = 0x0020,
	UNIFORM			= 0x0040,
	SMOOTH_STEP		= 0x0080,
	EDGES			= 0x0100,
	SHOW_EDGES		= 0x0200,
	LOD		        = 0x0400,
	RECALC		    = 0x0800,
	SHOW_STEPS		= 0x1000,
	MINDCNT			= 0x2000,
	MORE_CYCLES		= 0x4000,

	DEFAULTS        = LOD|RECALC|OCCLTEST|CURVETEST|CLIPTEST|BACKTEST|SMOOTH_STEP|EDGES
	};
	adaptdata	flags;
	int max_grid;
	int  _maxcells;		// max cells
	int  _maxcycles;	// max cycles
	double  _conv;	    // convergence threshold
	double  _minext;
	double  _mindz;

	static double ttable[32];
	static Color  hctbl[];
    static int hcmax;

public:
	AdaptOptions();
    void set_defaults();
	void init();
	int  smooth_step()			{ return BIT_TST(flags.tests,SMOOTH_STEP);}
	void set_smooth_step(int f)	{ BIT_SET(flags.tests,SMOOTH_STEP,f);}

	int  lod()					{ return BIT_TST(flags.tests,LOD);}
	void set_lod(int f)			{ BIT_SET(flags.tests,LOD,f);}

	int  mindcnt()				{ return BIT_TST(flags.tests,MINDCNT);}
	void set_mindcnt(int f)		{ BIT_SET(flags.tests,MINDCNT,f);}

	int  recalc()				{ return BIT_TST(flags.tests,RECALC);}
	void set_recalc(int f)		{ BIT_SET(flags.tests,RECALC,f);}

	int  always()				{ return BIT_TST(flags.tests,ALWAYS);}
	void set_always(int f)		{ BIT_SET(flags.tests,ALWAYS,f);}

	int  never()				{ return BIT_TST(flags.tests,NEVER);}
	void set_never(int f)		{ BIT_SET(flags.tests,NEVER,f);}

	int  show_edges()			{ return BIT_TST(flags.tests,SHOW_EDGES);}
	void set_show_edges(int f)	{ BIT_SET(flags.tests,SHOW_EDGES,f);}

	int  edges()				{ return BIT_TST(flags.tests,EDGES);}
	void set_edges(int f)		{ BIT_SET(flags.tests,EDGES,f);}

	int  show_steps()			{ return BIT_TST(flags.tests,SHOW_STEPS);}
	void set_show_steps(int f)  { BIT_SET(flags.tests,SHOW_STEPS,f);}

	int  more_cycles()			{ return BIT_TST(flags.tests,MORE_CYCLES);}
	void set_more_cycles(int f) { BIT_SET(flags.tests,MORE_CYCLES,f);}

	int  uniform()				{ return BIT_TST(flags.tests, UNIFORM);}
	void set_uniform(int f)		{ BIT_SET(flags.tests,UNIFORM,f);}

	int  clip_test()			{ return BIT_TST(flags.tests,CLIPTEST);}
	void set_clip_test(int f)	{ BIT_SET(flags.tests,CLIPTEST,f);}

	int  back_test()			{ return BIT_TST(flags.tests,BACKTEST);}
	void set_back_test(int f)	{ BIT_SET(flags.tests,BACKTEST,f);}

	int  overlap_test();
	void set_overlap_test(int f){ BIT_SET(flags.tests,OCCLTEST,f);}

	int  curve_test()			{ return BIT_TST(flags.tests,CURVETEST);}
	void set_curve_test(int f)	{ BIT_SET(flags.tests,CURVETEST,f);}

	uint grid_depth()			{ return max_grid;}
	void set_grid_depth(uint f){ max_grid=f;}

	int maxcells()				{ return _maxcells;}
	void set_maxcells(int v)	{ _maxcells=v;}

	int maxcycles()				{ return _maxcycles;}
	void set_maxcycles(int v)	{ _maxcycles=v;}

	double conv()				{ return _conv;}
	void set_conv(double c)		{ _conv=c;}

	double minext()				{ return _minext;}
	void set_minext(double c)	{ _minext=c;}

	double mindz()				{ return _mindz;}
	void set_mindz(double c)	{ _mindz=c;}

	double sharp_scale()		{ return ttable[flags.tlvl_sharp];}
	uint sharp()				{ return flags.tlvl_sharp;}
	void set_sharp(uint v)		{ flags.tlvl_sharp=v;}

	double normal_scale()		{ return ttable[flags.tlvl_normal];}
	uint normal()				{ return flags.tlvl_normal;}
	void set_normal(uint v)	{ flags.tlvl_normal=v;}

	double smooth_scale()		{ return ttable[flags.tlvl_smooth];}
	uint smooth()				{ return flags.tlvl_smooth;}
	void set_smooth(uint v)	{ flags.tlvl_smooth=v;}

	double hidden_scale()		{ return ttable[flags.tlvl_hidden];}
	uint hidden()				{ return flags.tlvl_hidden;}
	void set_hidden(uint v)	{ flags.tlvl_hidden=v;}

	double tlevel(int i)		{ return i>0?ttable[i]:1.0;}
	Color tcolor(int i);

	void set(AdaptOptions &f)	{	*this=f; }
	void get(AdaptOptions &f)	{	f=*this;}
};

extern	AdaptOptions    Adapt;

#endif
