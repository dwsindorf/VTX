#ifndef _UTILS_H
#define _UTILS_H
// ########## begin utils.h #########################
float lerp(float x, float xmin, float xmax, float ymin, float ymax){
	x=x<xmin?xmin:x;
	x=x>xmax?xmax:x;
	float a=(ymax-ymin)/(xmax-xmin);
	float b=ymin-a*xmin;
	float y=a*x+b;
	return y;
}
// ########## end  utils.h #########################
#endif
