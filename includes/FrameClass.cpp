// MovieClass.cpp

#include "FrameClass.h"
#include "NodeIF.h"
#include "ListClass.h"
#include "ObjectClass.h"

extern char tabs[];
extern void inc_tabs();
extern void dec_tabs();


//************************************************************
// ViewFrame class
//************************************************************
ViewFrame::ViewFrame(View *v)
{
	viewtype=v->viewtype;
	if(viewtype==GLOBAL)
	    point=v->spoint;
	else{
	    point.x=v->phi;
		point.y=v->theta;
		point.z=v->height;
	}
	heading=v->heading;
	pitch=v->pitch;
	view_angle=v->view_angle;
	view_tilt=v->view_tilt;
	view_skew=v->view_skew;
	gndlvl=v->gndlvl;
	time=v->time;
	delt=v->delt;
	gstride=v->gstride;
	fov=v->fov;
	vobj=v->vobj;

	hstride=v->hstride;
	vstride=v->vstride;
	zoom=v->zoom;
	minh=v->minh;
	minr=v->minr;
	astride=v->astride;
	radius=v->radius;

	msg=0;
}

ViewFrame::ViewFrame(ViewFrame *v)
{
	viewtype=v->viewtype;
	point=v->point;
	heading=v->heading;
	pitch=v->pitch;
	view_angle=v->view_angle;
	view_tilt=v->view_tilt;
	view_skew=v->view_skew;
	gndlvl=v->gndlvl;
	time=v->time;
	delt=v->delt;
	gstride=v->gstride;
	fov=v->fov;
	vobj=v->vobj;

	hstride=v->hstride;
	vstride=v->vstride;
	zoom=v->zoom;
	minh=v->minh;
	minr=v->minr;
	astride=v->astride;
	radius=v->radius;

	msg=0;
}
ViewFrame::ViewFrame(ViewFrame *v1, ViewFrame *v2, double f2)
{
    if(f2<0)
        f2=0;
    if(f2>1)
        f2=1;
	double f1=1-f2;
	viewtype=v1->viewtype;
	vobj=v1->vobj;

	point=(v1->point*f1)+(v2->point*f2);
	heading=f1*v1->heading+f2*v2->heading;
	pitch=f1*v1->pitch+f2*v2->pitch;
	view_angle=f1*v1->view_angle+f2*v2->view_angle;
	view_tilt=f1*v1->view_tilt+f2*v2->view_tilt;
	view_skew=f1*v1->view_skew+f2*v2->view_skew;
	gndlvl=f1*v1->gndlvl+f2*v2->gndlvl;
	time=f1*v1->time+f2*v2->time;
	gstride=f1*v1->gstride+f2*v2->gstride;
	fov=f1*v1->fov+f2*v2->fov;
	msg=0;
}

ViewFrame::~ViewFrame()
{
	FREE(msg);
}

//-------------------------------------------------------------
// ViewFrame::log_message() set log message
//-------------------------------------------------------------
void ViewFrame::log_message(char *v)
{
	FREE(msg);
	MALLOC(strlen(v)+1,char,msg);
	strcpy(msg,v);
}

//-------------------------------------------------------------
// ViewFrame::save() set a frame from a view
//-------------------------------------------------------------
void ViewFrame::save(View *v)
{
	viewtype=v->viewtype;
	if(v->viewtype==GLOBAL)
		point=v->spoint;
	else{
	    point.x=v->phi;
		point.y=v->theta;
		point.z=v->height;
	}
	heading=v->heading;
	pitch=v->pitch;
	view_angle=v->view_angle;
	view_tilt=v->view_tilt;
	view_skew=v->view_skew;
	gndlvl=v->gndlvl;
	time=v->time;
	delt=v->delt;
	gstride=v->gstride;

	hstride=v->hstride;
	vstride=v->vstride;
	zoom=v->zoom;
	minh=v->minh;
	minr=v->minr;
	astride=v->astride;
	radius=v->radius;

	fov=v->fov;
	vobj=v->vobj;
}

//-------------------------------------------------------------
// ViewFrame::restore() restore a view from a frame
//-------------------------------------------------------------
void ViewFrame::restore(View *v)
{
	v->viewtype=viewtype;
	if(viewtype==GLOBAL)
		v->spoint=point;
	else{
	    v->phi=point.x;
		v->theta=point.y;
		v->height=point.z;
		v->set_sidestep(viewtype==SURFACE?0:1);
	}
	v->heading=heading;
	v->pitch=pitch;
	v->view_angle=view_angle;
	v->view_tilt=view_tilt;
	v->view_skew=view_skew;
	v->gndlvl=gndlvl;
	v->time=time;
	v->delt=delt;
	v->fov=fov;
	v->vobj=vobj;

	v->gstride=gstride;
	v->hstride=hstride;
	v->vstride=vstride;
	v->zoom=zoom;
	v->minh=minh;
	v->minr=minr;
	v->astride=astride;
	v->radius=radius;
}

//-------------------------------------------------------------
// ViewFrame::print_position() return true if not same position
//-------------------------------------------------------------
void ViewFrame::print_position()
{
    if(viewtype==GLOBAL)
    	printf("origin(%g,%g,%g)\n",point.x/LY,point.y/LY,point.z/LY);
    else{
        if(point.z>1000*MILES)
     		printf("t %-10g p %-10g ht %g\n",point.x,point.y,point.z);
       	else if(point.z>10*MILES)
    		printf("t %-10g p %-10g ht %-10g glvl %g miles\n",point.x,point.y,point.z/MILES,gndlvl/MILES);
        else
    		printf("t %-10g p %-10g ht %-10g glvl %g ft\n",point.x,point.y,point.z/FEET,gndlvl/FEET);
    }
}

//-------------------------------------------------------------
// ViewFrame::save() save a view
//-------------------------------------------------------------
void ViewFrame::save(FILE *fp,FrameMgr *mgr)
{
	fprintf(fp,"%sView{\n",tabs);
	inc_tabs();
	switch(viewtype){
	case GLOBAL:
		fprintf(fp,"%sview=GLOBAL;\n",tabs);
		if(point.length()>LY)
			fprintf(fp,"%sorigin=Point(%g ly,%g ly, %g ly);\n",
				    tabs,point.x/LY, point.y/LY, point.z/LY);
		else 
			fprintf(fp,"%sorigin=Point(%g ,%g,%g);\n",
				    tabs,point.x, point.y, point.z);
		break;
	case ORBITAL:
	case SURFACE:
	    if(viewtype==ORBITAL)
			fprintf(fp,"%sview=ORBITAL;\n",tabs);
		else		
			fprintf(fp,"%sview=SURFACE;\n",tabs);	
		if(point.x)
			fprintf(fp,"%sphi=%g;\n",tabs,point.x);
		if(point.y)
			fprintf(fp,"%stheta=%g;\n",tabs,point.y);
		if(point.z){
		    if(point.z>LY)
				fprintf(fp,"%sheight=%g ly;\n",tabs,point.z/LY);
			else if(point.z>1000*MILES)
				fprintf(fp,"%sheight=%g;\n",tabs,point.z);
			else if(point.z>10*MILES)
				fprintf(fp,"%sheight=%g miles;\n",tabs,point.z/MILES);
			else
				fprintf(fp,"%sheight=%g ft;\n",tabs,point.z/FEET);
		}
		if(gndlvl)
			fprintf(fp,"%sgndlvl=%g ft;\n",tabs,gndlvl/FEET);
		break;		
	}
	fprintf(fp,"%stime=%g;\n",tabs,time);

	if(vstride)
		fprintf(fp,"%svstride=%g;\n",tabs,vstride);
	if(hstride)
		fprintf(fp,"%shstride=%g;\n",tabs,hstride);
	if(astride)
		fprintf(fp,"%sastride=%g;\n",tabs,astride);
	if(zoom)
		fprintf(fp,"%szoom=%g;\n",tabs,zoom);
	if(minh)
		fprintf(fp,"%sminh=%g;\n",tabs,minh);
	if(minr)
		fprintf(fp,"%sminr=%g;\n",tabs,minr);

	if(heading)
		fprintf(fp,"%sheading=%g;\n",tabs,heading);
	if(pitch)
		fprintf(fp,"%spitch=%g;\n",tabs,pitch);
	if(view_tilt)
		fprintf(fp,"%stilt=%g;\n",tabs,view_tilt);
	if(view_angle)
		fprintf(fp,"%sangle=%g;\n",tabs,view_angle);
	if(fov != 45.0)
		fprintf(fp,"%sfov=%g;\n",tabs,fov);
	if(gstride){
		if(gstride>LY)
			fprintf(fp,"%sspeed=%g ly;\n",tabs,gstride/LY);
		else
			fprintf(fp,"%sspeed=%g;\n",tabs,gstride);
	}
	if(msg){
		fprintf(fp,"%smessage=\"%s\";\n",tabs,msg);
	}
	if(vobj){
		char *buff=mgr->getViewExpr(vobj);
		if(buff){
			fprintf(fp,"%sviewobj=%s;\n",tabs,buff);
			::free(buff);
		}
	}
	dec_tabs();
	fprintf(fp,"%s}\n",tabs);
}

#define NEQ(test,value) fabs(value-test)>1e-6*fabs(value+test)

//-------------------------------------------------------------
// ViewFrame::save()  save a delta view
//-------------------------------------------------------------
void ViewFrame::save(FILE *fp,View *v,FrameMgr *mgr)
{
	char tmp[1024];
	tmp[0]=0;

	//sprintf(tmp,"%sView{\n",tabs);
	inc_tabs();
	if(v->viewtype != viewtype){
		switch(viewtype){
		case GLOBAL:
			sprintf(tmp+strlen(tmp),"%sview=GLOBAL;\n",tabs);
			break;
		case ORBITAL:
		case SURFACE:
		    if(viewtype==ORBITAL)
				sprintf(tmp+strlen(tmp),"%sview=ORBITAL;\n",tabs);
			else		
				sprintf(tmp+strlen(tmp),"%sview=SURFACE;\n",tabs);
		}
	}
	switch(viewtype){
	case GLOBAL:
	    if(point != v->spoint){
		if(point.length()>LY)
			sprintf(tmp+strlen(tmp),"%sorigin=Point(%g ly,%g ly, %g ly);\n",
				    tabs,point.x/LY, point.y/LY, point.z/LY);
		else 
			sprintf(tmp+strlen(tmp),"%sorigin=Point(%g ,%g,%g);\n",
				    tabs,point.x, point.y, point.z);
	    }
		break;
	case ORBITAL:
	case SURFACE:
		if(NEQ(point.x,v->phi))
			sprintf(tmp+strlen(tmp),"%sphi=%g;\n",tabs,point.x);
		if(NEQ(point.y,v->theta))
			sprintf(tmp+strlen(tmp),"%stheta=%g;\n",tabs,point.y);
		if(NEQ(point.z,v->height)){
		    if(point.z>LY)
				sprintf(tmp+strlen(tmp),"%sheight=%g ly;\n",tabs,point.z/LY);
			else if(point.z>1000*MILES)
				sprintf(tmp+strlen(tmp),"%sheight=%g;\n",tabs,point.z);
			else if(point.z>10*MILES)
				sprintf(tmp+strlen(tmp),"%sheight=%g miles;\n",tabs,point.z/MILES);
			else
				sprintf(tmp+strlen(tmp),"%sheight=%g ft;\n",tabs,point.z/FEET);
		}
		if(NEQ(gndlvl,v->gndlvl))
			sprintf(tmp+strlen(tmp),"%sgndlvl=%g ft;\n",tabs,gndlvl/FEET);
		break;		
	}
	if(NEQ(time,v->time))
		sprintf(tmp+strlen(tmp),"%stime=%g;\n",tabs,time);
	if(NEQ(delt , v->delt))
		sprintf(tmp+strlen(tmp),"%sdelt=%g;\n",tabs,delt);
	if(NEQ(heading , v->heading))
		sprintf(tmp+strlen(tmp),"%sheading=%g;\n",tabs,heading);
	if(NEQ(pitch , v->pitch))
		sprintf(tmp+strlen(tmp),"%spitch=%g;\n",tabs,pitch);
	if(NEQ(view_tilt , v->view_tilt))
		sprintf(tmp+strlen(tmp),"%stilt=%g;\n",tabs,view_tilt);
	if(NEQ(view_angle , v->view_angle))
		sprintf(tmp+strlen(tmp),"%sangle=%g;\n",tabs,view_angle);
	if(NEQ(view_skew , v->view_skew))
		sprintf(tmp+strlen(tmp),"%sskew=%g;\n",tabs,view_skew);
	if(NEQ(fov , v->fov))
		sprintf(tmp+strlen(tmp),"%sfov=%g;\n",tabs,fov);
	if(NEQ(gstride , v->speed)){
		if(gstride>LY)
			sprintf(tmp+strlen(tmp),"%sspeed=%g ly;\n",tabs,gstride/LY);
		else
			sprintf(tmp+strlen(tmp),"%sspeed=%g;\n",tabs,gstride);
	}
	if(NEQ(vstride , v->vstride))
		sprintf(tmp+strlen(tmp),"%svstride=%g;\n",tabs,vstride);
	if(NEQ(hstride , v->hstride))
		sprintf(tmp+strlen(tmp),"%shstride=%g;\n",tabs,hstride);
	if(NEQ(astride , v->astride))
		sprintf(tmp+strlen(tmp),"%sastride=%g;\n",tabs,astride);
	if(NEQ(zoom , v->zoom))
		sprintf(tmp+strlen(tmp),"%szoom=%g;\n",tabs,zoom);
	if(NEQ(minh , v->minh))
		sprintf(tmp+strlen(tmp),"%sminh=%g;\n",tabs,minh);
	if(NEQ(minr , v->minr))
		sprintf(tmp+strlen(tmp),"%sminr=%g;\n",tabs,minr);
	if(NEQ(radius , v->radius))
		sprintf(tmp+strlen(tmp),"%sradius=%g;\n",tabs,radius);

	if(msg){
		sprintf(tmp+strlen(tmp),"%smessage=\"%s\";\n",tabs,msg);
	}

	if(vobj && vobj!=v->vobj){
		char *buff=mgr->getViewExpr(vobj);
		if(buff){
			sprintf(tmp+strlen(tmp),"%sviewobj=%s;\n",tabs,buff);
			::free(buff);
		}
	}
	    
	dec_tabs();
	//sprintf(tmp,"%s}\n",tabs);
	if(strlen(tmp)>0)
		fprintf(fp,"%sView{\n%s%s}\n",tabs,tmp,tabs);
}

#define TSTDIF(x) if(NEQ(v->x,x)) return 1
//-------------------------------------------------------------
// ViewFrame::changedView()  test for different view
//-------------------------------------------------------------
int ViewFrame::changedView(View *v)
{
    TSTDIF(viewtype);
    TSTDIF(time);
    TSTDIF(delt);
    TSTDIF(heading);
    TSTDIF(pitch);
    TSTDIF(view_tilt);
    TSTDIF(view_angle);
    TSTDIF(view_skew);
    TSTDIF(fov);
    if(vobj!=v->vobj)
    	return 1;
    if(viewtype==GLOBAL){
        if(point.x != v->spoint.x) return 1;
        if(point.y != v->spoint.y) return 1;
        if(point.z != v->spoint.z) return 1;
    }
    else{
        if(point.x != v->phi)      return 1;
        if(point.y != v->theta)    return 1;
        if(point.z != v->height)   return 1;
    }
    return 0;
}

//************************************************************
// FrameMgr class
//************************************************************
FrameMgr::FrameMgr(ObjectMgr &mgr)  : objects(mgr)
{
}

FrameMgr::~FrameMgr()
{
    free();
}

//-------------------------------------------------------------
// ViewFrame::free() release storage
//-------------------------------------------------------------
void FrameMgr::free()
{
	frames.free();
}

//-------------------------------------------------------------
// ViewFrame::restart() clear views before current
//-------------------------------------------------------------
void FrameMgr::restart()
{
	if(frames.size<=1 || frames.atstart())
		return;
	ViewFrame *v;
	frames--;
	ViewFrame *f=frames.at();
	frames.ss();
	while((v=frames.pop())>0){
		if(v==frames.last() || v==f)
			break;
		delete v;
	}
	frames.ss();
}
//-------------------------------------------------------------
// ViewFrame::clip()  clear views after current
//-------------------------------------------------------------
void FrameMgr::clip()
{
	if(frames.size<1 || frames.atend())
		return;
	ViewFrame *v;
	ViewFrame *f=frames.at();
	//frames.se();
	frames++;
	while((v=frames.pop())>0){
		if(v==frames.first() || v==f)
			break;
		delete v;
	}
	frames.se();
}

//-------------------------------------------------------------
// FrameMgr::save()  save a delta view
//-------------------------------------------------------------
void FrameMgr::save(FILE *fp,View *v,ViewFrame *frame)
{
    frame->save(fp,v,this);
}

//-------------------------------------------------------------
// FrameMgr::save()  save a delta view
//-------------------------------------------------------------
void FrameMgr::save(FILE *fp,ViewFrame *frame)
{
    frame->save(fp,this);
}

//-------------------------------------------------------------
// FrameMgr::changedView()  test for different view
//-------------------------------------------------------------
int FrameMgr::changedView(View *v,ViewFrame *f)
{
     return f->changedView(v);
}

//-------------------------------------------------------------
// FrameMgr::add() add a new frame
//-------------------------------------------------------------
ViewFrame *FrameMgr::add(View *v) 
{
 	if(!frames.atend())
		clip();
	ViewFrame *frame;	
    if(frames.size>0){
		frame=frames.at();
		if(!frame->changedView(v))
			return 0;
	}

    frame=new ViewFrame(v);
    frames.add(frame);
    frames.se();
    return frame;
}

//-------------------------------------------------------------
// FrameMgr::viewobj() return viewobj by parsing objexp
//-------------------------------------------------------------
ObjectNode *FrameMgr::childobj(LinkedList<ObjectNode*>&children, char *name, int cnt)
{
    ObjectNode *node;
    int i=0;
    children.ss();
    while((node=children++)>0){
        if(strcmp(name,node->name())==0){
            i++;
            if(i==cnt)
                return node;
        }
    }
    return 0;
}

//-------------------------------------------------------------
// FrameMgr::viewobj() return viewobj by parsing objexp
//-------------------------------------------------------------
void FrameMgr::getChildren(ObjectNode *obj,LinkedList<ObjectNode*>&children)
{
    ObjectNode *node;
    obj->children.ss();
    children.reset();
    while((node=(ObjectNode*)obj->children++)>0){
        children.add(node);
    }
}

static char *vlast;
//-------------------------------------------------------------
// ViewFrame::getViewObj() find objects from object exprs 
//-------------------------------------------------------------
void FrameMgr::getObjects()
{
    vlast=0;
    frames.ss();
    ViewFrame *frame;
    while((frame=frames++)>0){
        getObject(frame);
    }
    frame=frames.last();
    if(vlast && frame->vobj != (ObjectNode*)vlast){
    	FREE(vlast); // free text malloced in sx.y or getViewExpr
    }
    vlast=0;
}

//-------------------------------------------------------------
// ViewFrame::getObject() get object by parsing expr string
//-------------------------------------------------------------
void FrameMgr::getObject(ViewFrame *v)
{
    if(!v || !v->vobj){
        printf("no object\n");
        return;
    }
    char  vstr[256];
    
    char  *objs[32];
    int   objn[32];
    int   objcnt=0;
    
    char *vexpr=(char*)v->vobj;

    strcpy(vstr,vexpr);
    char *s=strtok(vstr,".");
    if(s){
        objs[objcnt]=s;
        s=strtok(0,".");
        sscanf(s,"%d",objn+objcnt);
        s=strtok(0,".");
        objcnt++;
    }
    while(s){
        objs[objcnt]=s;
        s=strtok(0,".");
        sscanf(s,"%d",objn+objcnt);
        s=strtok(0,".");
        objcnt++;
    }
    LinkedList<ObjectNode*>children;
    ObjectNode *obj=(ObjectNode*)objects.ss();
    if(strcmp(obj->name(),objs[0])==0){
        while((obj=(ObjectNode*)objects.next())>0)
            children.add(obj);
    }
    else
        getChildren(obj,children);
    obj=0;    
    for(int i=0;i<objcnt;i++){
        obj=childobj(children,objs[i], objn[i]);
        if(!obj)
            break;
        getChildren(obj,children);
    }
    if(vlast && vexpr !=vlast){
    	FREE(vlast); // free text malloced in sx.y or getViewExpr
    }
    vlast=vexpr;
    v->vobj=obj;
    
}

//-------------------------------------------------------------
// ViewFrame::type_index() return index of child object
//-------------------------------------------------------------
int FrameMgr::type_index(ObjectNode *obj,ObjectNode*child)
{
    int i=0;
    obj->children.ss();
    ObjectNode *node;
    while((node=(ObjectNode*)obj->children++)>0){
        if(strcmp(node->name(),child->name()))
            continue;
        i++;
        if(node==child)
            break;
    }
    return i;
}

//-------------------------------------------------------------
// ViewFrame::getViewExpr() return view expr for object
//-------------------------------------------------------------
char *FrameMgr::getViewExpr(ObjectNode *vobj)
{
	if((vobj->typeClass()& ID_OBJECT)==0)
		return 0;
	LinkedList<ObjectNode*>objpath;
    char buff[256];
    buff[0]=0;
    
    ObjectNode *obj=vobj;
    
    while(obj&&(obj->typeClass()& ID_OBJECT)){
		objpath.push(obj);
		obj=obj->parent;
	}
    obj=objpath.ss();
    if(obj)
    	sprintf(buff,"%s.1",obj->name());
    while((obj=objpath++)>0){
        ObjectNode *nextobj=objpath.at();
         if(!nextobj)
             break;
        int i=type_index(obj,nextobj);
        if(i>0)
             sprintf(buff+strlen(buff),".%s.%d",nextobj->name(),i);
    }
    char *vexpr;
    MALLOC(strlen(buff)+1,char,vexpr);
    strcpy(vexpr,buff);
    return vexpr;
}

//-------------------------------------------------------------
// ViewFrame::lerp() return interpolaterd view
//-------------------------------------------------------------
ViewFrame *FrameMgr::lerp(ViewFrame *v1, ViewFrame *v2, double f)
{
    return new ViewFrame(v1,v2,f);
}
