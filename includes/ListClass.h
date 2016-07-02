
/* =========================================================================
 *  MODULE:     ListClass.h
 * =========================================================================
 * PURPOSE: c++ list class template definitions
 * COMMENT: all class method are "inline" because microsoft visual c++ does
 *          not support templates properly.
 * =========================================================================*/

#ifndef LISTCLASS_H
#define LISTCLASS_H

#include "defs.h"

/*====================== Constructor/Destructor debugging =================*/

enum{
	debug_objs  	= 0x00000001,
	debug_vars  	= 0x00000002,
	debug_lists 	= 0x00000004,
	debug_all_mem   = 0x0000000f
};

extern void debug_malloc(const char*, const char*, int);
extern void debug_malloc(const char*, int, int);
extern void debug_free(const char*, const char*, int);
extern void debug_free(const char*, int, int);

extern void set_mem_debug(int n);
extern int mem_debug;

/*======================  ObjectSym =========================================*/
class ObjectSym {
public:
	const char			   *text;
	char *name()		{ return (char*)text;}
	int  cmp(char *s)	{ return strcmp(text,s);}
	ObjectSym(const char *s)	{ text=s;}
};

/*======================  LongSym =========================================*/
class LongSym {
public:
	const char			   *text;
	int				value;
	char *name()		{ 	return (char*)text;}
	int  cmp(char *s)	{	return strcmp(text,s);}
};

/*======================  DoubleSym =======================================*/
class DoubleSym {
public:
	const char			   *text;
	double					value;
	char *name()		    { 	return (char*)text;}
	int  cmp(char *s)		{	return strcmp(text,s);}
};

/*======================  template objs ====================================*/

#define ATMARKER(p)		((p==&start) || (p==&end))
#define ATSTART(p)		(p==&start)
#define ATEND(p)		(p==&end)

/*======================  Node =============================================*/

template<class T> class Node {
public:
	Node<T>		*next;
	Node<T>		*prev;
	T			obj;
	Node(T val)						{	next=prev=0;obj=val;}

	Node(void)						{	next=prev=0;obj=0;}
	Node *insert(Node *b)			{	// insert node b after "this"
										Node<T> *c = next;
										b->next=c;
										b->prev=this;
										next=b;
										if(c)
											c->prev=b;
										return b;
									}
	Node *remove(void)				{	// remove links to "this"
										next->prev=prev;
										prev->next=next;
										return this;
									}
};

/*======================  LinkedList =======================================*/

template<class T> class LinkedList {
	Node<T>      end;      			// end element marker node
	Node<T>      start;     		// start element marker node
public:
	Node<T>     *ptr;      			// ptr to active node
	int          size;	        	// number of objs in list

	LinkedList(void)				{	if(mem_debug & 4)
											debug_malloc("LinkedList","",1);
										end.prev=&start;
										start.next=&end;
										ptr=&end;
										size=0;
									}
	~LinkedList(void)				{ 	if(mem_debug & 4)
											debug_free("~LinkedList","",1);
										if(size<=0)
											return;
										ss();
										while(pop()){}
									}
	int setptr(T val)				{	// check if val is in list
										ptr=start.next;
										while(!ATEND(ptr)){
											if(val==ptr->obj)
												return 1;
											ptr=ptr->next;
										}
										return 0;
									}
	T add(T val)					{	// add new node to end of list
										end.prev->insert(new Node<T>(val));
										size++;
										return end.prev->obj;
									}
	T add(T a,T b)				    {	// add new node b after a in list
										if(setptr(a)){
											ptr->insert(new Node<T>(b));
											size++;
											return ptr->next->obj;
										}
										else
											return add(b);
									}
	void push(T val)				{	// push new node at ptr
										ptr->prev->insert(new Node<T>(val));
										ptr=ptr->prev;
										size++;
									}
	T  pop()						{	// pop stack
										if(ATMARKER(ptr))
											return 0;

										Node<T> *node = ptr;
										T obj = node->obj;

										node->remove();
										ptr=node->next;
										delete node;
										size--;
										return obj;
									}
	T  ss()							{ 	// set ptr to first node
										ptr=start.next;
										return ptr->obj;
									}
	T  se()							{ 	// set ptr to last node
										ptr=end.prev;
										return ptr->obj;
									}
	T  at()							{ 	// return obj at ptr;
										return ptr->obj;
									}
	T  first()						{ 	// return obj at ptr;
										return start.next->obj;
									}
	T  last()						{ 	// return obj at ptr;
										return end.prev->obj;
									}
	int atstart()				    {   return ptr==start.next?1:0;}
	int atend()				    	{   return ptr==end.prev?1:0;}
	T  operator++(int)				{	// post inc ptr
										if(ATEND(ptr))
											return 0;
										ptr=ptr->next;
										return ptr->prev->obj;
									}
	T  operator--(int)				{	// post decr ptr
										if(ATSTART(ptr))
											return 0;
										ptr=ptr->prev;
										return ptr->next->obj;
									}
	T  operator[](int i)			{	// indexed offset
										if(i<0 || i >=size)
											return 0;

										Node<T>     *tmp=start.next;
										int j=0;

										while(j<i && !ATMARKER(tmp)){
											j++;
											tmp=tmp->next;
										}
										return tmp->obj;
									}
	int index()						{	// return index of ptr
	                                    Node<T> *tmp=start.next;
	                                    int i=0;
										while(!ATEND(tmp)){
											if(tmp==ptr)
												break;
											tmp=tmp->next;
											i++;
										}
										return i;
									}
	T next()						{	// return next ptr
	                                    if(ptr==end.prev)
	                                        return ptr->obj;
										return ptr->next->obj;
									}
	T prev()						{	// return next ptr
	                                    if(ptr==start.next)
	                                        return ptr->obj;
										return ptr->prev->obj;
									}
	int inlist(T val)				{	// check if val is in list
	                                    Node<T> *tmp=start.next;
										while(!ATEND(tmp)){
											if(val==tmp->obj)
												return 1;
											tmp=tmp->next;
										}
										return 0;
									}
	T remove(T val)					{	// remove val if in list
										if(setptr(val)){
											Node<T> *tmp=ptr;
											ptr->remove();
											ptr=ptr->next;
											delete tmp;
											size--;
											return ptr->obj;
										}
										return 0;
									}
	T replace(T a,T b)					{	// replace val if in list
										if(setptr(a)){
											ptr->obj=b;
											return a;
										}
										return 0;
									}
	T free(T val)					{	// remove val if in list
										if(setptr(val)){
											Node<T> *tmp=ptr;
											ptr->remove();
											ptr=ptr->next;
											delete tmp;
											size--;
											delete val;
											return ptr->obj;
										}
										return 0;
									}
	void reset()					{ 	// remove all nodes
										if(size<=0)
											return;
										ss();
										while(pop()){}
										end.prev=&start;
										start.next=&end;
										ptr=&end;
										size=0;
									}
	void free()						{
										if(mem_debug & 4)
										debug_free("LinkedList","free",0);
										if(size>0){
											T tmp;
											ss();
											while((tmp=pop())>0)
											delete tmp;
										}
										end.prev=&start;
										start.next=&end;
										ptr=&end;
										size=0;
									}
};


/*======================  Array =========================================*/

template<class T> class Array 	{
	int				memsize;
public:
	T 			   *base;
	int				addsize;
	int				size;
	int				index;

	Array()							{	if(mem_debug & 4)
											debug_malloc("Array()","",1);
										memsize = 20;
										MALLOC(memsize, T, base);
										addsize = memsize ;
										index=size=0;
									}

	Array(int n)					{	if(mem_debug & 4)
											debug_malloc("Array(int)","",1);
										memsize = n ? n : 1;
										MALLOC(memsize, T, base);
										addsize = memsize ;
										index=size=0;
									}
	Array(int n, int chunk)			{
										memsize = n ? n : 1;
										if(mem_debug & 4)
											debug_malloc("Array(int,int)","",1);
										MALLOC(memsize, T, base);
										addsize = chunk > 1 ? chunk : 1;
										index=size=0;
									}
	Array(T list, int n) 			{	if(mem_debug & 4)
											debug_malloc("Array(T,int)","",1);
										memsize = n;
										addsize = 1;
										MALLOC(memsize, T, base);
										index=size=0;
										for(int i=0;i<n;i++)
										    base[size++]=list+i;
									}

	Array(LinkedList<T> &l)			{	T tmp;
										memsize = l.size;
										if(mem_debug & 4)
											debug_malloc("Array(List&)","",1);
										addsize=1;
										MALLOC(memsize, T, base);
										index=size=0;

										l.ss();
										while((tmp=l++)>0)
											add(tmp);
										l.ss();
									}
	~Array()						{	if(mem_debug & 4)
											debug_free("~Array","",1);
										if(base)
											::free((char*)base);
										base=0;
									}
	void set(LinkedList<T> &l)		{	T tmp;
										if(base)
											::free((char*)base);
										base=0;
										memsize = l.size;
										addsize=1;
										MALLOC(memsize, T, base);
										index=size=0;
										l.ss();
										while((tmp=l++)>0)
											add(tmp);
										l.ss();
									}
	void reset()					{	size=index=0;}
	void insert(T val, int pos)		{	// insert into middle
										if(size+1>=memsize){
											memsize+=addsize;
											REALLOC(memsize, T, base);
										}
										int tmp=size;
										while(tmp>pos){
											base[tmp]=base[tmp-1];
											tmp--;
										}
										base[pos]=val;
										size++;
									}
	T add(T val)					{	// add to end
										if(size+1>=memsize){
											memsize+=addsize;
											REALLOC(memsize, T, base);
										}
										base[size++]=val;
										return size>=1 ? base[size-1] : 0 ;
									}
	void remove(T val)				{	// remove element
										int i,j;
										T *oldbase=base;
										MALLOC(size, T, base);
										for(i=0,j=0;i<size;i++){
										    if(oldbase[i]==val)
											    delete oldbase[i];
											else
											   	base[j++]=oldbase[i];
										}
										size=j;
										index=0;
										::free(oldbase);
									}
	void pack()						{	// free excess from top of array
 										memsize=size;
 										REALLOC(memsize, T, base);
									}
	void grow(int n)				{	// increase memsize by n
										memsize=size+n;
										REALLOC(memsize, T, base);
									}
	void resize(int n)				{	// resize to n
										REALLOC(n, T, base);
										memsize=n;
									}
	void  ss()						{ 	// set ptr to first element
										index=0;
									}
	void  se()						{ 	// set ptr to last element
										index=size ? size-1 : 0;
									}
	T  at()							{ 	// return obj at ptr;
										if(index>=size || index <0)
											return 0;
										return base[index];
									}
	T  first()						{ 	// return obj at ptr;
										return size>0?base[0]:0;
									}
	T  last()						{ 	// return obj at ptr;
										return size>0?base[size-1]:0;
									}
	T  operator++(int)				{	// post inc ptr
										if(index>=size)
											return 0;
										index++;
										return base[index-1];
									}
	T  operator--(int)				{	// post inc ptr
										if(index<0)
											return 0;
										index--;
										return base[index+1];
									}
	T  operator[](int i)			{	// indexed offset
										if(i<0 || i >=size)
											return 0;
										return base[i];
									}
	T  operator=(T val)				{	// indexed offset assignment
										return val;
									}
	void free()						{	if(mem_debug & 4)
										debug_free("Array","free",0);
										for(int i=0;i<size;i++)
											delete base[i];
										size=index=0;
									}
	};

/*======================  NameList =========================================*/
template<class T> class NameList : public Array<T>
{
public:
	NameList() : Array<T>(20)				{}
	NameList(int n)	: Array<T>(n)			{}
	NameList(int n,int m) : Array<T>(n,m)	{}
	NameList(T l, int n) : Array<T>(l,n)		{ sort();}
	NameList(LinkedList<T>&l) : Array<T>(l) 	{ sort();}
	char *substr(T v, char *s)	{	return strstr(v->name(),s);}
	int cmp(T val1, T val2)		{	return strcmp(val1->name(), val2->name());}
	int cmp(char *s, T val2)	{	return strcmp(s, val2->name());}

	// merge

	void merge(NameList<T> &b)  {
		int                 test;
		T                  avar, bvar;

		b.ss();
		this->ss();

		if(this->size==0){
			while(bvar=b++)
				add(bvar);
			this->pack();
			return;
		}
		NameList<T> c(this->size+b.size+1);
		while(this->index<this->size && b.index<b.size){
			avar=this->at();
			bvar=b.at();
			test=avar->cmp(bvar->name());
			if(test<=0){
				c.add(avar);
				(*this)++;
				if(test==0)
					b++;
			}
			else{
				c.add(bvar);
				b++;
			}
		}
		while(avar=(*this)++)
			c.add(avar);

		while(bvar=b++)
			c.add(bvar);
		grow(c.size-this->size);
		c.ss();

		for(int i=0;i<c.size;i++){
			this->base[i]=c.base[i];
		}
		this->size=c.size;
	}

	// inlist

	T inlist(char *str) {
		register int        ju, jm, jl, test;
		register T		   *ra;

		if (this->size<=0)
			return 0;

		ra=this->base-1; jl = 0; ju = this->size+1;

		while (ju - jl > 1) {
			jm = (ju + jl) >> 1;
			test = cmp(str, ra[jm]);
			if(!test) {
				this->index=jm+1;
				return ra[jm];
			}
			if(test>0)
				jl=jm;
			else ju=jm;
		}
		return 0;
	}

	void sort() {
		int                 l, j, ir, i = 1;
		T           	   *ra,rra;
		if (this->size<=1)
			return;

		ra=this->base-1;
		ir = this->size;
		l = (ir >> 1) + 1;
		for (;;) {
			if (l > 1)
				rra = ra[--l];
			else {
				rra = ra[ir];
				ra[ir] = ra[1];
				if (--ir == 1) {
					ra[1] = rra;
					return;
				}
			}
			i = l;
			j = l << 1;
			while (j <= ir) {
				if (j < ir && cmp(ra[j], ra[j+1]) < 0)
					j++;
				if (cmp(rra, ra[j]) < 0) {
					ra[i] = ra[j];
					j += (i = j);
				}
				else
					j = ir + 1;
			}
			ra[i] = rra;
		}
		this->ss();
	}
};

/*====================== ValueList =========================================*/
template<class T> class ValueList : public Array<T> {
public:
	ValueList() : Array<T>(20)					{}
	ValueList(int n) : Array<T>(n)				{}
	ValueList(int n,int m) : Array<T>(n,m)		{}
	ValueList(T l, int n) : Array<T>(l,n)		{}
	ValueList(LinkedList<T>&l) : Array<T>(l) 	{sort();}
	double cmp(T val1, T val2)		   {	return val1->value()-val2->value();}

	T inlist(double n)
	{
		register int        ju, jm, jl;
		register double     test;
		register T		   *ra;

		if (this->size<=0)
			return 0;

	    ra=this->base-1; jl = 0; ju = this->size+1;

		while (ju - jl > 1) {
			jm = (ju + jl) >> 1;
			test = cmp(n, ra[jm]);
			if(!test) return ra[jm]; if(test>0) jl=jm; else ju=jm;
		}
		return 0;
	}
	void sort() {
		int                 l, j, ir, i = 1;
		T           	   *ra,rra;

		if (this->size<=1)
			return;
		if(this->size==2){
		    if(cmp(this->base[0], this->base[1])<=0)
		        return;
		    rra=this->base[0];
		    this->base[0]=this->base[1];
		    this->base[1]=rra;
			return;
		}

		ra=this->base-1;
		ir = this->size;
		l = (ir >> 1) + 1;
		for (;;) {
			if (l > 1)
				rra = ra[--l];
			else {
				rra = ra[ir];
				ra[ir] = ra[1];
				if (--ir == 1) {
					ra[1] = rra;
					return;
				}
			}
			i = l;
			j = l << 1;
			while (j <= ir) {
				if (j < ir && cmp(ra[j], ra[j+1]) < 0)
					j++;
				if (cmp(rra, ra[j]) < 0) {
					ra[i] = ra[j];
					j += (i = j);
				}
				else
					j = ir + 1;
			}
			ra[i] = rra;
		}
	}
};
#endif
