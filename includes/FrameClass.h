
//  FrameClass.h

#ifndef _FRAMECLASS
#define _FRAMECLASS

#include "ViewClass.h"
class FrameMgr;

class ViewFrame
{
protected:
public:
	int   viewtype;
	double heading;
	double pitch;
	double view_angle;
	double view_tilt;
	double view_skew;
	double gndlvl;
	double time;
	double delt;
	double fov;
	double speed;
	Point  point;
	char   *msg;
	ObjectNode  *vobj;
	
	int changedView(View &);
	
	ViewFrame(View *);
	ViewFrame(ViewFrame *);
	ViewFrame(ViewFrame *,ViewFrame *,double);
	~ViewFrame();
	void restore(View *v);
	void save(View *v);
	void save(FILE *fp,FrameMgr*);
	void save(FILE *fp,View *v,FrameMgr*);
	void print_position();
	void log_message(char *s);
	char *message() { return msg;}
	int changedView(View *);
};

class FrameMgr
{
protected:
    ObjectMgr &objects;
	ObjectNode *childobj(LinkedList<ObjectNode*>&children, char *name, int cnt);
	int type_index(ObjectNode*,ObjectNode*);
    void getChildren(ObjectNode *,LinkedList<ObjectNode*>&);
    LinkedList<ViewFrame*> frames;
    void getObject(ViewFrame *);
public:
    
    FrameMgr(ObjectMgr &mgr);
    ~FrameMgr();
    void free();
    void clip();
    void restart();
    ViewFrame  *add(View *v);
    ViewFrame  *add(ViewFrame *v)	{ return frames.add(v);}
    int setFrame(ViewFrame *v)		{ return frames.setptr(v);}
    int atend()						{ return frames.atend();}
    int atstart()					{ return frames.atstart();}
    ViewFrame *at()           		{ return frames.at();}
    ViewFrame *first()				{ return frames.first();}
    ViewFrame *last()				{ return frames.last();}
    ViewFrame *next()				{ return frames.next();}
    ViewFrame *ss()   				{ return frames.ss();}
    ViewFrame *se()   				{ return frames.se();}
    ViewFrame *incr() 				{ return frames++;}
    ViewFrame *decr()				{ return frames--;}
    ViewFrame *pop()				{ return frames.pop();}
    int index()						{ return frames.index();}
    ViewFrame *lerp(ViewFrame *f1, ViewFrame *f2, double f);
	void save(FILE *fp,ViewFrame*);
	void save(FILE *fp,View *v,ViewFrame*);
	void getObjects();
    char *getViewExpr(ObjectNode *obj);
    int size() { return frames.size;}
	int changedView(View *,ViewFrame*);
};

#endif
