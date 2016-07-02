#include "AdaptOptions.h"
#include <math.h>

AdaptOptions  Adapt;

// colors to render tlevels

Color AdaptOptions::hctbl[]={
	Color(1,0,0),	// red
	Color(1,0,1),	// violet
	Color(1,1,1),	// white
	Color(1,1,0),	// yellow
	Color(0,1,0),	// green
	Color(0,1,1),	// turquose
	Color(0,0,1),	// blue
	Color(0,0,0.5),	// dark blue
    Color(0.5,0.5,0.5), // dark blue
    Color(0.5,0.5,0.5), // orange
    Color(0.2,0.2,0.2), // dark blue
	Color(0,0,0)	// black
};
double AdaptOptions::ttable[32];
int init_flag=0;
int AdaptOptions::hcmax=0;
Color AdaptOptions::tcolor(int i)
{
	i=i%hcmax;
    i=i<0?0:i;
    i=i>hcmax?hcmax:i;
    return hctbl[i];
}

//-------------------------------------------------------------
// AdaptOptions()	constructor
//-------------------------------------------------------------
AdaptOptions::AdaptOptions()
{
    set_defaults();
    hcmax=sizeof(hctbl)/sizeof(Color)-1;
	init();
}
//-------------------------------------------------------------
// AdaptOptions::set_defaults()   reset to default conditions
//-------------------------------------------------------------
void AdaptOptions::set_defaults()
{
    flags.tests=DEFAULTS;
    set_maxcells(1000);
    set_maxcycles(59);
    set_grid_depth(4);
    set_sharp(0);
    set_normal(1);
    set_smooth(3);
    set_hidden(5);
    set_conv(0.04);
    set_minext(2.50);
    set_mindz(0.5);
    set_mindcnt(1);
    set_back_test(0); // TODO: need to retest backfacing functions
   // set_overlap_test(0);
}

int  AdaptOptions::overlap_test()
{
	extern int color_depth();
	if(color_depth()<24)
		return 0;
	return BIT_TST(flags.tests,OCCLTEST);
}

//-------------------------------------------------------------
// void update()	make static arrays
//-------------------------------------------------------------
void AdaptOptions::init()
{
	if(init_flag)
		return;
	init_flag=1;

	double t=1;

	for(int i=0;i<32;i++){
		ttable[i]=t;
		t*=2;
	}
}
