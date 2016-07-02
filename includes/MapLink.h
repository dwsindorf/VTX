// MapLink.h

#ifndef _MAPLINK
#define _MAPLINK

#include "MapNode.h"

//************************************************************
// MapLink class
//************************************************************
class MapLink
{
public:
	MapNode *rt;	
	MapNode *dn;	
	MapNode *ctr;
	MapLink *lwr;

	int		lowest();
	int		empty();
	void	adapt();

	void	visit(void  (MapNode::*func)());
	void	visit(void (*func)(MapNode*));
	void	visit_all(void  (MapNode::*func)());
	void	visit_all(void (*func)(MapNode*));
	void	cycle(void (MapNode::*func)());
	void	CWcycle(void (MapNode::*func)());
	void	render();
	void	render_vertex();
	void	render_ids();
	void	combine();

	void	free();

	MapLink() : rt(0),dn(0),ctr(0),lwr(0) {}
	MapLink(MapNode *, MapNode *, MapNode *);
	~MapLink();
};


#endif

