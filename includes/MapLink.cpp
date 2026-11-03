#include "MapLink.h"
#include "SceneClass.h"

//************************************************************
// MapLink class
//************************************************************
//-------------------------------------------------------------
// MapLink::MapLink()	constructor
//-------------------------------------------------------------
MapLink::MapLink(MapNode *d, MapNode *c, MapNode *r)
{
	rt=r;dn=d;ctr=c;lwr=0;
	TheMap->links++;
}

//-------------------------------------------------------------
// MapLink::~MapLink()	destructor
//-------------------------------------------------------------
MapLink::~MapLink()
{
	TheMap->links--;
}

//-------------------------------------------------------------
// MapLink::lowest()	return 1 if at lowest tree level
//-------------------------------------------------------------
int MapLink::lowest()
{
	if(empty())
		return 0;

	if(lwr) return 0;
	if(dn && dn->link) return 0;
	if(rt && rt->link) return 0;
	if(ctr && ctr->link) return 0;
	return 1;
}

//-------------------------------------------------------------
// MapLink::empty()	return 1 if no link or node pointers
//-------------------------------------------------------------
int MapLink::empty()
{
	if(lwr) return 0;
	if(dn) return 0;
	if(rt) return 0;
	if(ctr) return 0;
	return 1;
}

//-------------------------------------------------------------
// MapLink::visit()	visit all child corner nodes
//-------------------------------------------------------------
void MapLink::visit(void (MapNode::*func)())
{
	if(lwr) lwr->visit(func);
	if(rt)  rt->visit(func);
	if(dn)  dn->visit(func);
	if(ctr) ctr->visit(func);
}
void MapLink::visit(void (*func)(MapNode*))
{
	if(lwr) lwr->visit(func);
	if(rt)  rt->visit(func);
	if(dn)  dn->visit(func);
	if(ctr) ctr->visit(func);
}

//-------------------------------------------------------------
// MapLink::visit_all()	visit all child nodes
//-------------------------------------------------------------
void MapLink::visit_all(void (MapNode::*func)())
{
	if(lwr) lwr->visit_all(func);
	if(rt)  rt->visit_all(func);
	if(dn)  dn->visit_all(func);
	if(ctr) ctr->visit_all(func);
}
void MapLink::visit_all(void (*func)(MapNode*))
{
	if(lwr) lwr->visit_all(func);
	if(rt)  rt->visit_all(func);
	if(dn)  dn->visit_all(func);
	if(ctr) ctr->visit_all(func);
}

//-------------------------------------------------------------
// MapLink::render()	render all children
//-------------------------------------------------------------
void MapLink::render()
{
	if(lwr)	lwr->render();
	if(rt)  rt->render();
	if(dn)  dn->render();
	if(ctr) ctr->render();
}

//-------------------------------------------------------------
// MapLink::render_vertex()	render all children
//-------------------------------------------------------------
void MapLink::render_vertex()
{
	if(lwr)	lwr->render_vertex();
	if(rt)  rt->render_vertex();
	if(dn)  dn->render_vertex();
	if(ctr) ctr->render_vertex();
}

//-------------------------------------------------------------
// MapLink::render_ids()	render all children
//-------------------------------------------------------------
void MapLink::render_ids()
{
	if(lwr)	lwr->render_ids();
	if(rt)  rt->render_ids();
	if(dn)  dn->render_ids();
	if(ctr) ctr->render_ids();
}

//-------------------------------------------------------------
// MapLink::cycle()		apply an input function to all child nodes
//						in a cw pattern starting at center node, then
//						the left-top corner and then around the
//						outside edge. (used for lowest links surface
//						rendering)
//-------------------------------------------------------------
void MapLink::cycle(void  (MapNode::*func)())
{
	MapNode *n;

	(ctr->*func)();

	n=TheMap->current;
	(n->*func)();

	n=n->CWright();
	(n->*func)();

	n=n->CWright();
	(n->*func)();

	n=n->CWdown();
	(n->*func)();

	n=n->CWdown();
	(n->*func)();

	n=n->CWleft();
	(n->*func)();

	n=n->CWleft();
	(n->*func)();

	n=n->CWup();
	(n->*func)();

	n=n->CWup();
	(n->*func)();

}

//-------------------------------------------------------------
// MapLink::adapt()		adaptive resizing function
//-------------------------------------------------------------
void MapLink::adapt()
{
	MapNode *last=TheMap->current;

	if(lwr){
		lwr->adapt();
		if(lwr->empty()){
			delete lwr;
			lwr=0;
		}
	}
	else if(last){
		TheMap->current=last;
		if (last->stest()){
			if(last->cdata)
				lwr=last->split();
			else
				last->split();
		}
	}
	if(rt)	rt->adapt();
	if(dn)	dn->adapt();
	if(ctr)	ctr->adapt();
	if(lowest()){
		TheMap->current=last;
		if(!last->ctest())
			return;
		if(last->cdata){
			last->clr_nflags();
			delete last->cdata;
			last->cdata=0;
			TheMap->mdeleted++;
			if(TheMap)
				TheMap->size--;
			return;
		}
		if(rt && !rt->ctest())
			return;
		if(dn && !dn->ctest())
			return;
		if(ctr && !ctr->ctest())
			return;
		combine();
		last->bounds.reset();
		last->dec_alevel();
		last->clr_nflags();
		last->clr_cchecks();
		last->clr_bchecked();
	}
}

//-------------------------------------------------------------
// MapLink::combine()	delete all child nodes at lowest level
//-------------------------------------------------------------
void MapLink::combine()
{
	MapNode *node;

	if(rt){
		rt->dnode=0;
		if(rt->unode==0){
			rt->remove();
			delete rt;
		}
		rt=0;
	}
	if(dn){
		dn->rnode=0;
		if(dn->lnode==0){
			dn->remove();
			delete dn;
		}
		dn=0;
	}
	if(ctr) {
		ctr->unode=ctr->lnode=0;
		node=ctr->rnode;
		if(node){
			node->lnode=0;
			if(node->rnode==0){
				node->remove();
				delete node;
			}
		}
		node=ctr->dnode;
		if(node){
			node->unode=0;
			if(node->dnode==0){
				node->remove();
				delete node;
			}
		}
		delete ctr;
		ctr=0;
	}
	lwr=0;
}

//-------------------------------------------------------------
// MapLink::free()		delete all child nodes
//-------------------------------------------------------------
void MapLink::free()
{
	MapNode *node;
	if(lwr) {
		lwr->free();
		delete lwr;
		lwr=0;
	}
	if(rt){
		rt->free();
		delete rt;
		rt=0;
	}
	if(dn){
		dn->free();
		delete dn;
		dn=0;
	}
	if(ctr) {
		ctr->free();
		node=ctr->rnode;
		if(node && node->rnode==0){
			node->remove();
			delete node;
		}
		node=ctr->dnode;
		if(node && node->dnode==0){
			node->remove();
			delete node;
		}
		delete ctr;
		ctr=0;
	}
}
