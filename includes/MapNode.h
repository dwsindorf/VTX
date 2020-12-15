// MapNode.h

#ifndef _NODECLASS
#define _NODECLASS

#include "NodeData.h"

#define TLMAX     14

#define AVE_TRIANGLES

class MapData;
class MapNode;
class Map;
class MapLink;

class Triangle {
public:
	Triangle(MapData *a,MapData *b,MapData *c);
	double distance;
	int type;
	bool backfacing;
	bool visible;

	double value(){
		return distance;
	}
	
	double size();
	double extent();
	double height();
	double depth();
	double density();
	Point vertex();
	Point center();
	Point normal();
	FColor color();
	void init();

	MapData  *d1;
	MapData  *d2;
	MapData  *d3;
};


//************************************************************
// MapNode class
//************************************************************
typedef struct mdata {
    unsigned int  ccheck   : 1; // curvature test flag
    unsigned int  clipped  : 1; // clipped by frame
    unsigned int  aflags   : 2; // adapt flags
    unsigned int  mflags   : 6; // visibility flags
    unsigned int  tlvl     : 4; // target level for subdivision
    unsigned int  flvl     : 5; // fractal subdivision level
    unsigned int  alvl     : 5; // actual subdivision level
    unsigned int  cdata    : 1; // use cdata flag
    unsigned int  recalc   : 1; // need recal flag
    unsigned int  nodraw   : 1; // no render flag (completely invisible)
    unsigned int  edge     : 1; // edge flag
    unsigned int  margin   : 1; // margin flag
    unsigned int  hidden   : 1; // invisible flag
    unsigned int  test1    : 1; // test flag
    unsigned int  test2    : 1; // test flag
} mdata;

typedef void (MapNode::*MNV)( void );
typedef void (MapNode::*MND)( MapData* );

class MapNode
{
private:
    enum {
        MCHECK      = 0x01, // set by visibility check
        BCHECK      = 0x02, // set by visibility check
        MASKED      = 0x04, // set if masked
        PARTVIS     = 0x08, // set if masked & neighbor visible
        DBF         = 0x10, // set if data polygon backfacing
        CBF         = 0x20, // set if cdata polygon backfacing
        BACKFACING  = DBF|CBF, // backfacing mask
        VCHKD       = MCHECK|BCHECK,
        NOTVIS      = MASKED|BACKFACING
    };

    enum {  // aflags (split/combine) cleared on each adapt pass
    CFLAG       =   0x0001, // combine flag (set in adapt if cell is too small)
    SFLAG       =   0x0002  // split flag (set in adapt if cell is too large)
    };

    enum{
        LEFT,RIGHT,UP,DOWN,UPLT,UPRT,DNLT,DNRT
    };

    friend class Map;
    friend class PreviewMap;
    friend class MapLink;
    friend class MapData;

    mdata   info;    // misc. flags
public:

    static MNV Dmid;
    static MNV Dstart;
    static MND Cmid;
    static MND Cstart;
    static MapNode *TheNode;
    MapNode *lnode;
    MapNode *rnode;
    MapNode *unode;
    MapNode *dnode;

    void vertexN()              { vertexN(&data);}
    void vertexC()              { Vcolor(&data);vertex(&data);}
    void vertexCN()             { vertexCN(&data);}
    void vertex()               { vertex(&data);}
    void vertexP()              { Point p=data.point();
                                  glVertex3dv((double*)(&p));
                                }
    void vertexP(MapData*d)     { Point p=d->point();
                                  glVertex3dv((double*)(&p));
                                }
    void IDvertex()             { IDvertex(&data);}
    void IDvertex(MapData*d);
    Color Tcolor(MapData*d);
    void Vcolor()               { Vcolor(&data);}
    void vertexZ()              { vertexZ(&data);}
    void vertexZ(MapData*d);
    void Rvertex();//              { info.cdata=0;Raster.vertex(this);}
    void Rvertex(MapData*d);//     { info.cdata=1;Raster.vertex(this);}

    void Vcolor(MapData*);
    void vertex(MapData*);
    void vertexC(MapData*n)     { Vcolor(n);vertex(n);}
    void vertexN(MapData*);
    void vertexCN(MapData*n);
//    void vertexS()              { info.cdata=0;Svertex();}
//    void vertexS(MapData*)      { info.cdata=1;Svertex();}
    void Svertex()				 { Svertex(&data);}
    void Svertex(MapData*);
    void remove();

    MapLink *split();
    void    sizechk();
    int     curvechk();
    int     gradchk();
    void    vischk();
    void    pvischk();
    double  extent();
    void    balance();
    void    set_tests();
    int     clipchk(Point p);
    MapLink *link;
    MapData *cdata;
    MapData data;    // point, color data
    MapNode(MapNode *, uint, uint);
    MapNode(MapNode *, MapData *);
    ~MapNode();

    MapNode *CCWup()        {  MapNode *n=unode;
                                while(n && n->lnode==0)
                                    n=n->unode;
                                return n;
                            }
    MapNode *CCWdown()      {   MapNode *n;
                                n=dnode;
                                while(n && n->rnode==0)
                                    n=n->dnode;
                                return n;
                            }
    MapNode *CCWright()     {   MapNode *n;
                                n=rnode;
                                if(unode)
                                while(n && n->unode==0)
                                    n=n->rnode;
                                return n;
                            }
    MapNode *CCWleft()      {   MapNode *n;
                                n=lnode;
                                if(dnode)
                                while(n && n->dnode==0)
                                    n=n->lnode;
                                return n;
                            }
    MapNode *CWup()         {  MapNode *n=unode;
                                while(n && n->rnode==0)
                                    n=n->unode;
                                return n;
                            }
    MapNode *CWdown()       {   MapNode *n;
                                n=dnode;
                                while(n && n->lnode==0)
                                    n=n->dnode;
                                return n;
                            }
    MapNode *CWright()      {   MapNode *n;
                                n=rnode;
                                if(dnode){
                                while(n && n->dnode==0)
                                    n=n->rnode;
                                }
                                return n;
                            }
    MapNode *CWleft()       {   MapNode *n;
                                n=lnode;
                                while(n->unode==0)
                                    n=n->lnode;
                                return n;
                            }

    void Vgroup(MapData**p);
    void Dgroup(MapData**p);
    void Lgroup(MapData**p);
    void Cgroup(MapData**p);
    void Pgroup(MapData**p);

    bool TanGroup(MapData**v);

    // visibility flags
    void clr_backfacing()   { BIT_OFF(info.mflags,BACKFACING);}
    void set_backfacing(int i)  {info.mflags|=(i&BACKFACING);}
    int backfacing()        { return info.mflags & BACKFACING;}
    void test_backfacing();
    void clr_mchecked()     { BIT_OFF(info.mflags,MCHECK);}
    void set_mchecked()     { BIT_ON(info.mflags,MCHECK);}
    int mchecked()          { return info.mflags & MCHECK;}
    void clr_bchecked()     { BIT_OFF(info.mflags,BCHECK);}
    void set_bchecked()     { BIT_ON(info.mflags,BCHECK);}
    int bchecked()          { return info.mflags & BCHECK;}
    void set_hidden(int i)  { info.hidden=i;}
    int hidden()            { return info.hidden;}
    int invisible()         { return (info.mflags & NOTVIS);}
    int visible()           { return invisible()?0:info.mflags&VCHKD;}

    void clr_masked()       { BIT_OFF(info.mflags,MASKED);}
    void set_masked()       { BIT_ON(info.mflags,MASKED);}
    int masked()            { return info.mflags & MASKED;}
    void set_partvis()      { BIT_ON(info.mflags,PARTVIS);}
    void clr_partvis()      { BIT_OFF(info.mflags,PARTVIS);}
    int partvis()           { return info.mflags & PARTVIS;}
    int  clipped()          { return info.clipped;}
    void set_clipped()      { info.clipped=1;}
    void clr_clipped()      { info.clipped=0;}

    void set_margin(int i)  { info.margin=i;}
    int margin()            { return info.margin;}

    void set_edge(int i)    { info.edge=i;}
    int edge()              { return info.edge;}

    void set_test1(int i)   { info.test1=i;}
    int test1()             { return info.test1;}
    void set_test2(int i)   { info.test2=i;}
    int test2()             { return info.test2;}

    void set_need_recalc(int i) { info.recalc=i;}
    int  need_recalc()      { return info.recalc;}
    void set_nodraw(int i)  { info.nodraw=i;}
    int  nodraw()           { return info.nodraw;}
    void set_cdata(int i)   { info.cdata=i;}
    MapData *ndata()        { return (info.cdata && cdata) ?cdata:&data;}
    static MapData *surface_data(MapData *d);

    // curvature test and status flags

    int  cchecked()         { return info.ccheck;}
    void set_ccheck()       { info.ccheck=1;}
    void clr_ccheck()       { info.ccheck=0;}

    void set_split()        { BIT_ON(info.aflags,SFLAG);}
    void clr_split()        { BIT_OFF(info.aflags,SFLAG);}
    void set_combine()      { BIT_ON(info.aflags,CFLAG);}
    void clr_combine()      { BIT_OFF(info.aflags,CFLAG);}

    int  stest()            { return info.aflags & SFLAG;}
    int  ctest()            { return info.aflags & CFLAG;}

    void set_stest()        { BIT_ON(info.aflags,SFLAG);}
    void clr_stest()        { BIT_OFF(info.aflags,SFLAG);}

    // flags reset and clear functions

    void init_flags()       { info.aflags=0;
                              info.mflags=MASKED;
                              info.nodraw=0;
                              info.ccheck=0;
                              info.clipped=0;
                              info.recalc=0;
                              info.edge=0;
                              info.margin=0;
                              info.hidden=0;
                              info.cdata=0;
                            }
    void clr_flags()        { info.aflags=0;
                              info.mflags=MASKED;
                              info.nodraw=0;
                            }
    void clr_nflags()       { info.aflags=0;
                              info.mflags=MASKED;
                              info.nodraw=0;
                              info.ccheck=0;
                              data.invalidate_normal();
                              info.hidden=0;
                              if(cdata)
                            	  cdata->invalidate_normal();
                            }
    void clr_aflags()       { info.aflags=0; }
    void clr_cchecks();
    void find_limits();

    // cellsize level functions
#define NMOVE(r,u) \
	while(n && n->r==0){ \
		(*func)(&n->data,n); \
		n=n->u; \
	}

    unsigned int tlevel()           { return info.tlvl;}
    void set_tlevel(unsigned int t) { info.tlvl=t;}

    unsigned int flevel()           { return info.flvl<<1;}
    unsigned int elevel()           { return cdata?(info.alvl<<1)+1:(info.alvl<<1);}
    unsigned int alevel()           { return info.alvl<<1;}
    unsigned int alvl()             { return info.alvl;}
    unsigned int flvl()             { return info.flvl;}
    void inc_alevel()               { info.alvl++;}
    void dec_alevel()               { info.alvl--;}

    // visit functions

    void    CWcycle(void (MapNode::*func)());
    void    CWcycle(void (*func)(MapNode*));
    void    visit(void  (MapNode::*func)());
    void    visit(void (*func)(MapNode*));
    void    visit_all(void  (MapNode::*func)());
    void    visit_all(void (*func)(MapNode*));
    void    visit_cycle(void (*func)(MapData*));
    void    visit_cycle(void (*func)(MapNode*));

    void    free();

    double max_height()             { return data.max_height();}
    double height()                 { return data.height();}
    double theta()                  { return data.theta();}
    double phi()                    { return data.phi();}

    double delht()                  { return data.Z();}
    double delt()                   { return data.X();}
    double delp()                   { return data.Y();}

    double density()                { return data.density();}
    double sediment()               { return data.sediment();}
    double rock()                   { return data.rock();}

    int type()                      { return data.type(); }
    int fog()                       { return data.has_density()||(cdata && cdata->has_density());}
    int dual_terrain()              { return data.two_pass()|| (cdata && cdata->two_pass());}
    int surface_water()             { return data.water() || (cdata && cdata->water());}
    int subsurface_water()          { return surface_water()?0:data.two_surface()||(cdata && cdata->two_surface());}
    int has_water()                 { return data.two_surface()||(cdata && cdata->two_surface());}

    // mapping dimensions & fractal ht

    MapNode *locate(double, double);
    MapNode *grid_walk(double, double);
    void    find_neighbors(int);
    int     neighbors();
    void    recalc1();
    void    recalc2();

    Point *pnt_normal(MapData*);
    Point *ave_normal(MapData*);
    Point *normal(MapData*);
    Point  ave_point();

    double cell_size();

    MapNode *down()                 { return dnode;}
    MapNode *up()                   { return unode;}
    MapNode *left()                 { return lnode;}
    MapNode *right()                { return rnode;}
    Point point()                   { return data.point();}
    Point tangent(MapData *a);
    Color color()                   { return data.color();}
    void setColor(Color c)          { data.setColor(c);
                                      if(cdata)
                                         cdata->setColor(c);
                                    }
    virtual void init_render();
    virtual void render();
    virtual void render_cycle();
    virtual void render_vertex();
    virtual void render_ids();
    virtual void adapt();
    virtual void init_map_data(MapData*);
};

#endif

