#include "TerrainData.h"
#include "TerrainClass.h"
#include "ImageClass.h"
#include "TextureClass.h"
#include "AdaptOptions.h"
#include "SceneClass.h"
#include "RenderOptions.h"
#include "GLSLMgr.h"
#include "Layers.h"

#define DEBUG_IMAGES
//#define DEBUG_TEXS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern double Rand();
extern double zslope();
extern double Phi;
extern void inc_tabs();
extern void dec_tabs();

extern char   tabs[];

extern int addtabs;

//------- TNimage symbols --------------------------------------

static LongSym iopts[]={
	{"INVT",	INVT},
	{"GRAY",	GRAY},
	{"ACHNL",	ACHNL},
	{"NORM",	NORM},
	{"REPEAT",  REPEAT},
	{"BUMP",	BUMP},
	{"HMAP",	HMAP},
	{"TEX",		TEX},
	{"TBIAS",	TBIAS},
	{"DECAL",	DECAL},
	{"LINEAR",	INTERP},
	{"NEAREST",	NEAREST},
	{"CLAMP",	CLAMP},
	{"REFLECT",	REFLECT},
	{"BLEND",	BLEND},
	{"REPLACE",	REPLACE},
	{"BORDER",	BORDER},
	{"S",		SEXPR},
	{"A",		AEXPR},
	{"TMP",	    TMP},
	{"RANDOMIZE",	RANDOMIZE},

};
NameList<LongSym*> IOpts(iopts,sizeof(iopts)/sizeof(LongSym));

static LongSym itypes[]={
	{"TILE",		TILE},
	{"PMAP",		PMAP},
	{"SMAP",		SMAP},
	{"CMAP",		CMAP}
};
NameList<LongSym*> ITypes(itypes,sizeof(itypes)/sizeof(LongSym));

extern int hits,visits,misses;

static TerrainData Td;

static bool getLayer(TNode *p) {
	while (p && (p->typeValue() != ID_LAYER)) {
		p=p->getParent();
	}
	if(p && p->typeValue() == ID_LAYER)
		return true;
	else
		return false;
}
bool TNinode::inLayer=false;

//************************************************************
// Class TNinode
//************************************************************
TNinode::TNinode(char *s, int l, TNode *r) : TNvector(r)
{
	opts=l;
	image=0;
	name=0;
	gname=0;

	setName(s);
	FREE(s);
}

TNinode::~TNinode()
{
	FREE(name);
	FREE(gname);
	DFREE(expr);
	DFREE(right);
}

//-------------------------------------------------------------
// TNinode::texActive() return enable color in texture
//-------------------------------------------------------------
bool TNinode::texActive(){
	return (opts &TEX)?true:false;
}
//-------------------------------------------------------------
// TNinode::bumpActive() return enable bump in texture
//-------------------------------------------------------------
bool TNinode::bumpActive(){
	return (opts &BUMP)?true:false;
}
//-------------------------------------------------------------
// TNinode::hmapActive() return enable hmap in texture
//-------------------------------------------------------------
bool TNinode::hmapActive(){
	return (opts &HMAP)?true:false;
}

//-------------------------------------------------------------
// TNinode::setTexActive() enable color in texture
//-------------------------------------------------------------
void TNinode::setTexActive(bool b){
	if(!b)
		BIT_OFF(opts,TEX);
	else
		BIT_ON(opts,TEX);
}
//-------------------------------------------------------------
// TNinode::setBumpActive() enable bumpmap in texture
//-------------------------------------------------------------
void TNinode::setBumpActive(bool b){
	if(!b)
		BIT_OFF(opts,BUMP);
	else
		BIT_ON(opts,BUMP);
}
//-------------------------------------------------------------
// TNinode::setHmapActive() enable hmap in texture
//-------------------------------------------------------------
void TNinode::setHmapActive(bool b){
	if(!b)
		BIT_OFF(opts,HMAP);
	else
		BIT_ON(opts,HMAP);
}

//-------------------------------------------------------------
// TNinode::valueString() node value substring
//-------------------------------------------------------------
void TNinode::valueString(char *s)
{
	setStart(s);
	char tmp[256];
	tmp[0]=0;
    optionString(tmp);
    if(tmp[0]==0)
	    sprintf(s+strlen(s),"%s(\"%s\",",symbol(),name);
	else
	    sprintf(s+strlen(s),"%s(\"%s\",%s",symbol(),name,tmp);
	TNarg *arg=(TNarg*)right;
	while(arg){
		arg->valueString(s);
		arg=arg->next();
		if(arg)
			strcat(s,",");
	}
	if(inLayer)
		strcat(s,")");
	else
		strcat(s,");\n");
	setEnd(s);
}

//-------------------------------------------------------------
// TNinode::save() set name
//-------------------------------------------------------------
void TNinode::save(FILE *f)
{
	TNvector::save(f);
}

//-------------------------------------------------------------
// TNinode::setName() set name
//-------------------------------------------------------------
void TNinode::setName(char *s)
{
	FREE(name);
	if(s){
	 	MALLOC(strlen(s)+1,char,name);
		strcpy(name,s);
	}
}
//-------------------------------------------------------------
// TNinode::setName() set name
//-------------------------------------------------------------
void TNinode::setGName(char *s)
{
	FREE(gname);
	if(s){
	 	MALLOC(strlen(s)+1,char,gname);
		strcpy(gname,s);
	}
}

//-------------------------------------------------------------
// TNinode::eval() build image
//-------------------------------------------------------------
void TNinode::eval() {
	if(!image)
		init();
	SINIT;
	if(image){
		S0.image=image;
 		S0.set_ivalid();
 	}
}

//-------------------------------------------------------------
// TNinode::optionString() get option string
//-------------------------------------------------------------
void TNinode::optionString(char *c, int opts_val)
{
	int i,j=0;
	if((opts_val&IOPTS) == 0)
	    return;
	int itype=opts_val & IMAP;
	if(itype){
		for(i=0;i<ITypes.size;i++){
		    if(ITypes[i]->value==itype){
		        strcat(c,ITypes[i]->name());
		        j++;
		        break;
		    }
		}
	}
	for(i=0;i<IOpts.size;i++){
    	if(opts_val & IOpts[i]->value){
    	    if(j>0)
   				strcat(c,"|");
   			strcat(c,IOpts[i]->name());
   			j++;
   		}
	}
	strcat(c,",");
}

//-------------------------------------------------------------
// TNinode::optionString() get option string
//-------------------------------------------------------------
void TNinode::optionString(char *c)
{
	optionString(c,opts);
}

//************************************************************
// Class TNbands
//************************************************************
bool TNbands::show_tmps=false;
TNbands::TNbands(char *s, int l, TNode *r) : TNinode(s,l,r)
{
	opts|=BANDS|T1D|BMP|SPX;
}

//-------------------------------------------------------------
// TNbands::valueString() node value substring
//-------------------------------------------------------------
void TNbands::valueString(char *s)
{
	setStart(s);
	sprintf(s+strlen(s),"%s(\"%s\",",symbol(),name);
	optionString(s+strlen(s));
	TNarg *arg=(TNarg*)right;
	int n=0;
	while(arg){
		arg->valueString(s);
		arg=arg->next();
		if(arg){
			strcat(s,",");
			n++;
			if(n>4){
				if(inLayer)
					strcat(s,"\n\t");
				else
					strcat(s,"\n");
				n=0;
			}
		}
	}
	if(inLayer)
		strcat(s,")");
	else		
		strcat(s,");\n");
	setEnd(s);
}

//-------------------------------------------------------------
// TNbands::save() save the node
//-------------------------------------------------------------
void TNbands::save(FILE *f)
{
    char buff[256];
    buff[0]=0;
   // bool inlayer=getLayer(this);
    optionString(buff);
    if(buff[0]==0)
	    fprintf(f,"%s%s(\"%s\",",tabs,symbol(),name);
	else
	    fprintf(f,"%s%s(\"%s\",%s",tabs,symbol(),name,buff);

	int n=0;
	TNarg *arg=(TNarg*)right;
	while(arg){
		if(arg->left)
			arg->left->save(f);
		arg=arg->next();
		if(arg){
			fprintf(f,",");
			n++;
			if(n>4){
			    if(n==5)
			        inc_tabs();
				fprintf(f,"\n%s",tabs);
			}
		}
	}
	if(n>4)
	    dec_tabs();
//	if(inLayer)
//		fprintf(f,")+\n");
//	else		
	fprintf(f,");\n");

}

//-------------------------------------------------------------
// TNbands::init() build image
//
// syntax options:
//
// bands(Color,Color,..)      	      explicit color list
// bands(h, Color,Color,..)           color spline or gradient
// bands(h, delta, Color,..)          random intensity modulation
// bands(h, delta, mix, cb, Color,..) random intensity & color mod.
// bands(h, delta, Color, mix, cb)    alternate form of above.
//-------------------------------------------------------------
void TNbands::init()
{
	if(image)
		return;

	TNarg *arg=(TNarg*)right;
	Color *colors=0;
	int h=0;
	int i=0,j=0;
	Color cols[256];
	int ncols=0;

	INIT;
	if(arg)
		arg->eval();

	if(S0.svalid())
	    h=(int)S0.s;
	
	if(opts & TMP)
		TheScene->tmp_files++;

	Image *im=images.load(name,this);
	if(im){
#ifdef DEBUG_IMAGES
		printf("%-20s LOADING SAVED 1 X %d X %d IMAGE %s\n","TNbands",h,im->comps(), name);
#endif
		image=im;
		return;
	}
	if(arg)
		arg=arg->next();

	int mflag=0;
	int dflag=0;
	int rmix=0;
	bool aflag=(opts&ACHNL);
	Color ca=Color(0.5,0.5,0.5);
	Color cb=ca;
	Color *knots=0;
	Color randcol;
	double delta=0;
	double mix=0;
	bool reflect=opts&REFLECT;
	int r=reflect?2:1;
	int last_rand=lastn;
	
	// evaluate remaining arguments

	while(arg && ncols<255){
		arg->eval();
		if(S0.cvalid()){
			if(S0.c.hasAlpha())
			   aflag=true;
			if(mflag && j){
			    cb=S0.c;
			    j=0;
			}
			else
			    cols[ncols++]=S0.c;
		}
		else if(S0.s==RANDID){
		   // if(j){
		        rmix=1;
		    //    j=0;
		    //}
		    //else
		    //    cols[ncols++]=Color(Rand(),Rand(),Rand(),Rand());
		}
		else if(i==0){
			delta=S0.s;
			dflag=1;
		}
		else if(i==1){
		    mix=S0.s;
		    mflag=1;
		    j=1;
		}
		arg=arg->next();
		i++;
	}

	if(reflect){
		for(int i=0;i<ncols;i++){
			cols[ncols*2-i-1]=cols[i];
		}
		ncols*=2;
	}
	//cout<<"ncols="<<ncols<<" h:"<<h<<endl;

	// malloc colors and assign variables
	int b=1;

	if(h==0){
	    h=ncols;
	    while(b<h){
	        b=b*2;
	    }
		MALLOC(b,Color,colors);
		for(i=0;i<ncols;i++)
		    colors[i]=cols[i];
		for(i=ncols;i<b;i++)
		    colors[i]=cols[ncols-1];
		h=b;
	}
	else{
		
		MALLOC(h,Color,colors);
		if(!dflag && !mflag)
		    knots=cols;
		else {
		    if(!mflag)
		        knots=cols;
		    else if(ncols==1)
		        ca=cols[0];
		    else
		        knots=cols;
		}

		// assign colors

		for(i=0;i<h;i++){
		    if(knots){
		    	if((opts & INTRP_MASK) == NEAREST){
		    		int m = i*ncols/h;
		    		colors[i]=knots[m];
		    	}
		    	else{
		    		double f=i/(h-1.0);
					if(opts & CLAMP){
						if(i==h-1){
							colors[i]=knots[ncols-1];
							break;
						}
						else
							colors[i]=cspline(f,ncols-1,knots);
					}
					else{
						colors[i]=cspline(f,ncols,knots);
					}
		    	}
		    }
		    else
			   colors[i]=ca;
//			if(delta){
//			    double f=Rand();
//			    if(f>0.5)
//					colors[i]=colors[i].lighten(delta*(f-0.5));
//			    else
//					colors[i]=colors[i].darken(delta*(0.5-f));
//			}
			if(mflag){
				double a=colors[i].alpha();
			    colors[i]=colors[i].blend(cb,mix*LRAND);
			    colors[i].set_alpha(a);
			}
		    if(dflag)
		    	colors[i]=colors[i].blend(Color(LRAND,LRAND,LRAND,LRAND),delta*LRAND);			    			    
			if(!aflag)
			    colors[i].set_alpha(1);
		}

	}

//	for(int i=0;i<h;i++){
//		colors[i].print();
//	}
#ifdef DEBUG_IMAGES
		printf("%-20s BUILDING 1 X %d %s IMAGE %s\n","TNbands",h,aflag?"alpha":"",name);
#endif
	image=new Image(colors,h,1);
	if(aflag){
	    image->set_alpha(1);
	}
	lastn=last_rand;
	images.save(name,image,this);
}

//************************************************************
// Class TNimage
//************************************************************
bool TNimage::show_tmps=false;
TNimage::TNimage(char *s, int l, TNode *r) : TNinode(s,l,r)
{
	opts|=IMAGE|BMP|JPG|SPX;
	TNarg *arg=(TNarg*)right;
	int height=1,width=1;
	arg->eval();
	height=(int)S0.s;

	arg=arg->next();
	arg->eval();
	width=(int)S0.s;
	if(width==1 || height==1)
	    opts|=T1D;
	else
	    opts|=T2D;
}

//-------------------------------------------------------------
// TNimage::save() save the node
//-------------------------------------------------------------
void TNimage::save(FILE *f)
{
    //bool inlayer=getLayer(this);
	//cout<<"TNimage::save inlayer"<<endl;

//    if(inlayer){
//    	TNode *p=getParent();
//    	if(p->typeValue()==ID_ADD){
//    		TNadd *a=(TNadd *)p;
//    		if(a)
//    			a->right->save(f);
//    	}
//    	return;
//    }
	char tmp[512];
	tmp[0]=0;
    optionString(tmp);
    if(tmp[0]==0)
	    fprintf(f,"%s%s(\"%s\",",tabs,symbol(),name);
	else
	    fprintf(f,"%s%s(\"%s\",%s",tabs,symbol(),name,tmp);

	if(right){
	    TNarg *arg=(TNarg*)right;
		arg->save(f);
	}
	fprintf(f,");\n");
}

//-------------------------------------------------------------
// TNimage::init() build image
//-------------------------------------------------------------
void TNimage::init()
{
	if(image)
		return;

	TNarg *arg=(TNarg*)right;
	TNode *value=0;
	Image *grad=0;
	double rw=0.5,rh=0.5;
	int h=1,w=1;

	arg->eval();
	h=(int)S0.s;

	arg=arg->next();
	arg->eval();
	w=(int)S0.s;

	Image *im = 0;

	arg=arg->next();
	value=arg->left;
	arg=arg->next();
	bool changed=false;
	if(arg && !(opts&GRAY)){
		arg->eval();
		if(S0.strvalid()){
			if(gname && strcmp(gname,S0.string))
				//BIT_ON(opts,CHANGED);
				changed=true;
			setGName(S0.string);
			grad=images.load(gname,BMP|JPG);
		}
	}
	//if (!(opts & CHANGED)) {
	if (!(opts & CHANGED)) {
		im = images.load(name, this);
		if (im) {
#ifdef DEBUG_IMAGES
			printf("%-20s LOADING SAVED %d X %d X %d IMAGE %s %s\n", "TNimage", w,
					h, im->comps(), name,gname?gname:"");
#endif
			image = im;
			return;
		}
	}

	double start=clock();

	
#ifdef DEBUG_IMAGES
	//Noise::resetStats();
	printf("%-20s BUILDING %d X %d IMAGE %s\n","TNimage",w,h,name);
	im=new Image(opts,h,w,value,grad);
	double build_time=(double)(clock() - start)/CLOCKS_PER_SEC;
	cout<< "image build time:"<<build_time<<endl;
	//Noise::showStats();
#else
	im=new Image(opts,h,w,value,grad);
#endif
	images.save(name,im,this);
	image=images.load(name,this);

}

//************************************************************
// TNtexture class
//************************************************************
TNtexture::TNtexture(char *s, int l, TNode *d)
     : TNinode(s,l,d)
{
	texture=0;
	image=0;
}

//-------------------------------------------------------------
// TNtexture::~TNtexture() destructor
//-------------------------------------------------------------
TNtexture::~TNtexture()
{
	DFREE(texture);
}

//-------------------------------------------------------------
// TNtexture::setTexActive() enable color in texture
//-------------------------------------------------------------
void TNtexture::setTexActive(bool b){
	TNinode::setTexActive(b);
	if(texture){
		texture->tex_active=b;
	   	texture->invalidate();
	}
}
//-------------------------------------------------------------
// TNtexture::setBumpActive() enable bumpmap in texture
//-------------------------------------------------------------
void TNtexture::setBumpActive(bool b){
	TNinode::setBumpActive(b);
	if(texture){
		texture->bump_active=b;
    	texture->invalidate();
	}
}

//-------------------------------------------------------------
// TNtexture::valueString() node value substring
//-------------------------------------------------------------
void TNtexture::valueString(char *s)
{
    TNtexture *tex=this;
    if(expr)
        tex=(TNtexture*)expr;
	setStart(s);
	if(CurrentScope->tokens() && token)
        strcat(s,token);
	else{
		sprintf(s+strlen(s),"%s(\"%s\",",symbol(),tex->name);
	 	tex->optionString(s+strlen(s));
		TNarg *arg=(TNarg*)tex->right;
		while(arg){
			arg->valueString(s+strlen(s));
			arg=arg->next();
			if(arg)
				strcat(s,",");
		}
		strcat(s,")");
	}
	setEnd(s);
}

//-------------------------------------------------------------
// TNtexture::setExpr() set expr string
//-------------------------------------------------------------
void TNtexture::setExpr(char *c)
{
	TNtexture *n=(TNtexture*)TheScene->parse_node(c);
	if(n){
	    DFREE(expr);
	    expr=n;
	    expr->setParent(parent);
		expr->init();
	}
}

//-------------------------------------------------------------
// TNtexture::applyExpr() apply expr value
//-------------------------------------------------------------
void TNtexture::applyExpr()
{
    if(expr){
    	Image *old_image=0;
    	if(texture)
    		old_image=texture->timage;
		DFREE(texture);
        DFREE(right);
        TNtexture *tex=(TNtexture*)expr;
        setName(tex->name);
		texture=tex->texture;
		if(texture) { // BUG: shouldn't be here if texture==0 ?
			texture->expr=this;
			if(texture->timage!=old_image)
				texture->invalidate();
		}
        right=tex->right;
		opts=tex->opts;
		if(right)
			right->setParent(this);
		tex->texture=0;
		tex->right=0;
		delete tex;
        expr=0;
    }
}

//-------------------------------------------------------------
// TNtexture::init() init for adapt of make
//-------------------------------------------------------------
void TNtexture::init()
{
    Image *timage=images.load(name,BMP);
	if(!timage){
	    //printf("TNtexture ERROR image %s not found\n",name);
	    return;
	}
	if(Render.invalid_textures()){
		delete texture;
		texture=0;
	}
    if(texture==0){
	    texture=new Texture(timage,opts,this);
	    texture->aveColor=timage->aveColor();
	    //texture->aveColor.print();
    }
    timage->set_accessed(true);
	if(opts & SEXPR){
		texture->s_data=true;
	}
	if(opts & AEXPR){
		texture->a_data=true;
	}
}

//-------------------------------------------------------------
// TNtexture::eval() evaluate the node
// special note: this node expr is called AFTER the terrain stack
// is evaluated not in-line as other nodes. This is done so that
// the final Ht etc. can be used in the texture calculation.
// args: a=color_ampl, b=bump_ampl
// name,opts[?BUMP][?HMAP][|S|A],[Sexpr],[Aexpr],start,BUMP?[a,b:a,b=0],
// bias,num_orders,orders_delta,orders_ampl,bump_damp,HMAP?[hamp,hbias],
// phi_bias,ht_bias,bump_bias,slope_bias
//-------------------------------------------------------------
void TNtexture::eval()
{
 	SINIT;
	S0.set_flag(TEXFLAG);
    if(CurrentScope->zpass() && !hmapActive())
        return;
	if(getFlag(NODE_BAD))
	    return;
	if(texture==0 || texture->image()==0){
	    setFlag(NODE_BAD);
		return;
	}
	texture->enabled=true;
	if(!isEnabled()){
		texture->enabled=false;
		return;
	}
	if((!texActive() || !Render.textures())&&(!bumpActive() || !Render.bumps())&&(!hmapActive() || !Render.hmaps())){
		S0.clr_svalid();
		texture->enabled=false;
		return;
	}

	if(CurrentScope->rpass() && (bumpActive()||texActive())){
		int nid=TerrainData::tp->noise.size;
		Td.add_texture(texture);
		TNarg *arg=(TNarg*)right;

		if(opts & SEXPR){
			arg->eval();
			if(TerrainData::tp->noise.size==nid)
				s_noise=false;
			else
				s_noise=true;
			if(opts & AEXPR)
				arg=arg->next();
		}
		if(opts & AEXPR){
			nid=TerrainData::tp->noise.size;
			arg->eval();
			if(TerrainData::tp->noise.size==nid)
				a_noise=false;
			else
				a_noise=true;
		}
		return;
	}

	int n=0;
	double arg[20];
	n=getargs(right,arg,20);

	double s=0,t=0,f=1,a=1,b=0,bias=0;
	double orders=1,orders_delta=2.0,orders_atten=1.0;
	double bumpdamp=0;
	double hmval=0,hmbias=0;
	double pbias=0,hbias=0,bbias=0,sbias=0;

	extern double Phi,Theta;
	double phi = Phi / 180;
	double theta = Theta / 180.0 - 1;
	//double avalue=1;
	texture->avalue=1;
	texture->bumpamp=0;
	int i = 0;

	if(opts & SEXPR){
		texture->s_data=true;
		s=arg[i++]/texture->scale;
		if(opts & AEXPR){
			texture->a_data=true;
			texture->avalue=arg[i++];
		}
		else if(texture->t2d())
			t=s;
	}
	else if(opts & AEXPR){
		texture->a_data=true;
		texture->avalue=arg[i++];
	}

	if(i<n)
		f=arg[i++];
	//if(i<n && bumpActive())
	if(i<n)
		//texture->bumpamp=arg[i++];
		a=arg[i++];
	if(i<n)
		//texture->texamp=arg[i++];
		b=arg[i++];
	if(i<n)
		bias=arg[i++];
	if(i<n)
		orders=arg[i++];
	if(i<n)
		orders_delta=arg[i++];
	if(i<n)
		orders_atten=arg[i++];
	if(i<n)
		bumpdamp=arg[i++];
	if(opts & HMAP){
		if(i<n)
			hmval=arg[i++];
		if(i<n)
			hmbias=arg[i++];
	}
	if(i<n)
		pbias=arg[i++];
	if(i<n)
		hbias=arg[i++];
	if(i<n)
		bbias=arg[i++];
	if(i<n)
		sbias=arg[i++];

    //avalue=clamp(avalue,0,1);
    //a*=avalue;
   // b*=avalue;

	if(CurrentScope->zpass() && hmapActive() && hmval==0)
		return;

	texture->scale=f;
	texture->s=s;
	texture->t=t;

	if(bumpActive()){
	    texture->bumpamp=a;
	    texture->texamp=b;
	}
	else{
	    texture->texamp=a;
	    texture->bumpamp=0;
	}
    texture->bias=bias;

    texture->orders=orders;
    texture->orders_atten=orders_atten;
    texture->orders_delta=orders_delta;
    texture->bump_damp=bumpdamp;
    texture->hmap_amp=hmval;
    texture->hmap_bias=hmbias;
    texture->phi_bias=pbias;
    texture->height_bias=hbias;
    texture->bump_bias=bbias;
    texture->slope_bias=sbias;
    
    double tbias=1;
	texture->tilt_bias=tbias;

	S0.clr_svalid();
	if(hmapActive() && hmval!=0){
	    double hpb=fabs(pbias);
	    double pbf=hpb>1?0.5*hpb:0.5;
	    double hmb=1;
	    double p=pow(abs(Phi/90),pbf);
	    double hb=1-clamp(hpb,0,1);
	    double pb;
	    if(pbias>=0)
	        pb=lerp(p,0,1,hb,1);
	    else
	    	pb=lerp(p,0,1,1,hb);
	    hmb=pb;

 		texture->s+=phi;
		texture->t+=theta;

		texture->svalue=0;
		texture->tvalue=0;

		int mode=texture->intrp();

		double scale=texture->scale;
		double amp0=hmb*texture->hmap_amp/scale;
		double amp=amp0;
		double z=0;
		for(int i=0;i<(int)texture->orders;i++){
			double v=texture->getTexAmpl(mode)-0.5;
			z+=v*amp;
			amp*=texture->orders_atten;
			texture->scale*=texture->orders_delta;
		}
		texture->scale=scale;
		double h=z+texture->hmap_bias;
		S0.p.z=h;
		S0.set_pvalid();
		S0.clr_svalid();
	}
}

#define SEXPR 1
#define VEXPR 2
//-------------------------------------------------------------
// TNtexture::exprString() evaluate a texture expression argument
//-------------------------------------------------------------
int TNtexture::exprString(TNarg *arg,char *nstr){
	char sstr[256];
	sstr[0]=0;
	nstr[0]=0;
#ifdef DEBUG_TEXS
	char vstr[1024];
	vstr[0]=0;
	arg->valueString(vstr);
	cout << "TNtexture ? expr:"<<vstr << endl;
#endif
	CurrentScope->set_shader(1);
	arg->valueString(nstr);
	SINIT;
	arg->eval();
	bool is_const=S0.constant();
	int s_expr=(strlen(nstr)>0 && !is_const)?SEXPR:0;
	SINIT;
	CurrentScope->set_texture(0);

	CurrentScope->set_shader(0);
	CurrentScope->set_texture(1); // non-shader expr
	SINIT;
	arg->eval();
	is_const=S0.constant();

	arg->valueString(sstr);
	int v_expr=(strlen(sstr)>0 && !is_const)?VEXPR:0;
#ifdef DEBUG_TEXS
	if(s_expr)
		cout << "TNtexture s expr:"<<nstr << endl;
	if(v_expr){
		cout << "TNtexture v expr:"<<sstr << endl;
		strcpy(nstr,sstr);
	}
#endif
	SINIT;
	CurrentScope->set_texture(0);
	return v_expr|s_expr;

}
//-------------------------------------------------------------
// TNtexture::initProgram() set shader uniform variables
//-------------------------------------------------------------
bool TNtexture::initProgram(){
	char defs[2048];
	char nstr[512];
	nstr[0]=0;
	defs[0]=0;
	if(texture==0 || !texture->enabled)
		return false;
	if(!texture->tex_active && ! texture->bump_active)
		return false;

	int id=texture->tid;
	sprintf(defs+strlen(defs),"#define TX%d\n",id);
	texture->cid=-1;
	if(!S0.inactive())
		texture->cid=texture->num_coords;
	TNarg *arg=(TNarg*)right;
	if(texture->a_data || texture->s_data  || texture->d_data){
		INIT;
		int cmode=0;
		if(texture->s_data){
			cmode=exprString(arg,nstr);
			if(s_noise)
				sprintf(defs+strlen(defs),"#define N%d %s\n",id,nstr);
			if(texture->a_data)
				arg=arg->next();
		}
		if(texture->a_data){
			if(arg)
				cmode=exprString(arg,nstr);
			else
				cmode=VEXPR;
			switch(cmode){
			case VEXPR:
				sprintf(defs+strlen(defs),"#define A%d AT%d\n",id,id);
				break;
			case SEXPR:
				sprintf(defs+strlen(defs),"#define A%d %s\n",id,nstr);
				break;
			}
		}
		else if(texture->d_data){
			sprintf(defs+strlen(defs),"#define A%d AT%d\n",id,id);
		}
	}

	if(texture->cid>=0)
		sprintf(defs+strlen(defs),"#define C%d CS%d\n",id,texture->num_coords++);
	else
		sprintf(defs+strlen(defs),"#define C%d BIAS\n",id);
	strcat(GLSLMgr::defString,defs);
	return true;
}

//-------------------------------------------------------------
// TNtexture::save() archive the node
//-------------------------------------------------------------
void TNtexture::save(FILE *f)
{
	char buff[1024];
	buff[0]=0;
	valueString(buff);
	//if(addtabs)
	    fprintf(f,"\n%s",tabs);
	fprintf(f,"%s",buff);
}
