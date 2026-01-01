// TextureClass.cpp

#include "TextureClass.h"
#include "ImageMgr.h"
#include "NoiseClass.h"
#include "TerrainNode.h"
#include "TerrainData.h"

#include "TerrainClass.h"
#include "GLSLMgr.h"
#include "GLSLVars.h"
#include "Effects.h"
#include "RenderOptions.h"

#include <math.h>

extern double Theta, Phi,Rscale,Gscale,Hscale;
static TerrainData Td;

#define TEXFLOOR // makes tex coords modulo scale (fixes float precision problems)
#define FIX_T0	 // corrects tex coords discontinuity at theta=0.0

//#define DEBUG_TEXTURES
//************************************************************
// Class Texture
//************************************************************

int Texture::num_attrs=0;
int Texture::num_coords=0;

//-------------------------------------------------------------
// Texture::Texture() Constructor
//-------------------------------------------------------------
Texture::Texture(Image *i, int l, TNode *e)
{
	options=l;
    timage=i;
    type=GL_TEXTURE_2D;
	expr=e;
	id=0;
	scale=1;
	tvalue=svalue=0;
	texamp=1;
	bumpamp=0;
	avalue=1;
	bias=0;
	valid=false;
	orders=1;
	orders_delta=2.0;
	orders_atten=0.5;
	hmap_amp=0;
	hmap_bias=0;
	tex_active=(options & TEX)?true:false;
	bump_active=(options & BUMP)?true:false;
	hmap_active=(options & HMAP)?true:false;
	enabled=true;
	s_data=false;
	a_data=false;
	d_data=false;
	bump_damp=0.0;
	bump_mip=1;
	cid=0;
	bump_bias=0;
	phi_bias=0;
	height_bias=0;
	slope_bias=0;
	tilt_bias=0;
	tid=0;
	s=0;
	t=0;
#ifdef DEBUG_TEXTURES
	cout<<"Texture::Texture new Texture "<<timage->height<<"x"<<timage->width<<endl;
#endif
}

//-------------------------------------------------------------
// Texture::~Texture() Destructor
//-------------------------------------------------------------
Texture::~Texture()
{
	//cout<<"Texture::~Texture Deleting Texture:"<<this<<" image:"<<timage<<endl;
	del();
}

//-------------------------------------------------------------
// Texture::reset() reset for shader pass
//-------------------------------------------------------------
void Texture::reset() {
	num_attrs=num_coords=0;
}

//-------------------------------------------------------------
// Texture::image() return pointer to Image object
//-------------------------------------------------------------
Image *Texture::image()
{
 	return timage;
}

//-------------------------------------------------------------
// Texture::pixels() return pointer to image data
//-------------------------------------------------------------
const void *Texture::pixels()
{
	return image()->data;
}

//-------------------------------------------------------------
// Texture::del() free resources
//-------------------------------------------------------------
void Texture::del() {
	if(id>0)
		glDeleteTextures(1, &id);
	id=0;
	valid = false;
}

//-------------------------------------------------------------
// Texture::getTexCoords() return amplitude of color at current index
//-------------------------------------------------------------
double Texture::getTexAmpl(int mode){
	double x,y;
	getTexCoords(x,y);
	x+=1;
	//printf("x:%-10g y:%-10g fm:%-10g ym:%-10g\n",x,y,fmod(x,1),fmod(y,1));
	return timage->value(mode, x,y);

}

//-------------------------------------------------------------
// Texture::texCoords() set 2d texture coordinates
//-------------------------------------------------------------
void Texture::texCoords(int tchnl)
{
	double sv=0,tv=0;
	getTexCoords(sv,tv);
	glMultiTexCoord4d(tchnl,sv,tv,0,1);

}
//-------------------------------------------------------------
// Texture::texCoords() set 3d texture coordinates
//-------------------------------------------------------------
void Texture::texCoords(int tchnl,Point p)
{
	Point tp=getTexCoords(p);
	glMultiTexCoord4d(tchnl,tp.x,tp.y,tp.z,1);
}

//-------------------------------------------------------------
// Texture::texCoords() set Triplanar texture coordinates 
//-------------------------------------------------------------
// note: we need to subtract the "floor" of the render_cycle "root" 
// node texture coordinates from each "leaf" to avoid striping artifacts 
// see note in following function for phi,theta mapping
//-------------------------------------------------------------
Point Texture::getTexCoords(Point pt){
	double wscale=Gscale*Hscale*0.3;

	Point pm=pt*wscale*scale;
	//cout<<"scale:"<<scale<<" length:"<<scale/pm.length()<<endl;
	//pm.print("\n");
	Point pf=p*wscale*scale; // p=MapPt of root node
	Point pr=pm-pf.floor();
	return pr;
}

//-------------------------------------------------------------
// Texture::getTexCoords() return texture lookup coordinates
//-------------------------------------------------------------
// note:svalue and tvalue contain the theta & phi values for the 
// "root" node of render_cycle whereas s and t are the phi
// and theta values of the "leaf" nodes. we need to subtract the 
// floor of the root node from each leaf to avoid stripping artifacts 
// in the texture 
// explanation from "Claude" AI
// BAD: Each vertex computes its own floor
//  root_uv  = root_coord  - floor(root_coord);   // e.g., 1.8 - 1.0 = 0.8
//  left_uv  = left_coord  - floor(left_coord);   // e.g., 1.9 - 1.0 = 0.9
//  right_uv = right_coord - floor(right_coord);  // e.g., 2.1 - 2.0 = 0.1  ← WRAP!
// GOOD: All vertices use the same reference
//  float root_floor = floor(root_coord);
//  root_uv  = root_coord  - root_floor;  // 1.8 - 1.0 = 0.8
//  left_uv  = left_coord  - root_floor;  // 1.9 - 1.0 = 0.9
//  right_uv = right_coord - root_floor;  // 2.1 - 1.0 = 1.1 <- smooth (no wrap)
//-------------------------------------------------------------
void Texture::getTexCoords(double &x, double &y){
	double sf=0,tf=0,tv=0,sv=0,sc=scale,a=timage->aspect();
	if(t1d()){
		x=s*sc+bias;
		y=0;
		return;
	}
    sv=s*sc-0.5;
	tv=a*t*sc;
#ifdef TEXFLOOR
	sf=svalue*sc-0.5;
	sf=FLOOR(sf);
	tf=a*tvalue*sc;
	tf=FLOOR(tf);
#ifdef FIX_T0
	if(tv>0 && tf<-1)
		tv-=a*2*sc;
	else if(tv<0 && tf>=0)
		tv+=a*2*sc;
#endif
	x=tv-tf; // width lookup
	y=sv-sf; // tx=s-0.5+1

#else
	sf=floor(svalue*sc-0.5);
	tf=floor(a*tvalue*sc);
	x=tv-tf;
	y=sv-sf;
#endif
}


//-------------------------------------------------------------
// Texture::bumpCoords() set texture coordinates for bumpmap
//  - called from Raster.cpp for OGL bump mapping only
//-------------------------------------------------------------
void Texture::bumpCoords(int tchnl,double x, double y)
{
	double sv=0,tv=0;
	getTexCoords(sv,tv);
	if(t1d())
		glMultiTexCoord4d(tchnl,sv+x,0,0,1);
	else
		glMultiTexCoord4d(tchnl,sv+x,tv+y,0,1);
}

//-------------------------------------------------------------
// Texture::eval() evaluate TNtexture string
//-------------------------------------------------------------
void Texture::eval()
{
	int mode=CurrentScope->passmode();
	CurrentScope->set_tpass();
	expr->eval();
	CurrentScope->set_passmode(mode);
}

//-------------------------------------------------------------
// Texture::set_state() set texture OGL state
//-------------------------------------------------------------
void Texture::set_state() {
	float fLargest=1.0f;
	glEnable(GL_TEXTURE_2D);
	if(Raster.filter_aniso())
		//fLargest=8;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	//cout << "Anisotopic filtering level: "<<fLargest<<endl;
	//cout << "Anisotopic filtering : "<<Raster.filter_aniso()<<endl;
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, style());
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, TheScene->tex_mip);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	if(t2d()){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (options & BORDER) ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, intrp());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, intrp());

}

//-------------------------------------------------------------
// Texture::begin() create/bind texture image
//-------------------------------------------------------------
void Texture::begin() {
	glEnable(GL_TEXTURE_2D);
	if(!valid || Render.invalid_textures()) {
		if (id>0)
			glDeleteTextures(1, (GLuint*)&id);

		glGenTextures(1, &id); // Generate a unique texture ID
		cout<<"glGenTextures "<<tid<<" id:"<<id<<endl;
		glBindTexture(GL_TEXTURE_2D, id);
		set_state();

		int w=width();
		int h=height();
		bool rgba_image=(image()->gltype()==GL_RGBA)?true:false;
		bool alpha_image=image()->alpha_image();
		int rgb_step=rgba_image?4:3;

		unsigned char* data=(unsigned char*)malloc(h*w*4); // temporary data (rgba)
		unsigned char* rgb=(unsigned char*)pixels();       // original image data (rgba or rgb)

		// build texture image for Shader and OGL usage

		bool shader_pass=Render.draw_shaded()?true:false;
		double alpha=shader_pass?1.0:texamp;

		bool norm=normalize();
		bool set_alpha=shader_pass || alpha_image;
		
		//cout<<tid <<" 2d:"<< image()->tx2d()<< " rgba_image:"<< rgba_image << " alpha_image:"<< alpha_image << endl;

		for (int i = 0; i < h; i++){
			for (int j = 0; j< w ; j++) {
			    unsigned char ac=255;
			    int index=i*w+j;
			    int rgb_index=index*rgb_step;
			    data[index*4+0]=rgb[rgb_index+0];
			    data[index*4+1]=rgb[rgb_index+1];
			    data[index*4+2]=rgb[rgb_index+2];

			    if(set_alpha && rgba_image)
			    	ac=alpha*rgb[rgb_index+3];
			    
			    data[index*4+3]=ac;
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		FREE(data);
		valid=true;
	}
	std::cout << "Binding texture id=" << id << " to unit " << tid << std::endl;

	glBindTexture(GL_TEXTURE_2D, id);

}

//-------------------------------------------------------------
// Texture::end() end of texture usage call
//-------------------------------------------------------------
void Texture::end()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_1D, 0);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
}

//-------------------------------------------------------------
// Texture::initProgram() set shader #defines for texture
// - called BEFORE shader program is created-info.logf
//-------------------------------------------------------------
bool Texture::initProgram(){
	if(!tex_active && !bump_active)
		return false;
	if(!Render.textures() && !Render.bumps())
		return false;
	if(!enabled)
		return false;
	char defs[1024];
	defs[0]=0;
	int tid=TerrainProperties::tid;
	if(tex_active && Render.textures())
		sprintf(defs+strlen(defs),"#define T%d\n",tid);
	if(orders>1)
		sprintf(defs+strlen(defs),"#define M%d\n",tid);
	if(expr)
		expr->initProgram();

	if(bump_active && Render.bumps()){
		sprintf(defs+strlen(defs),"#define B%d\n",tid);
	}
	strcat(GLSLMgr::defString,defs);
	return true;
}

//-------------------------------------------------------------
// Texture::setProgram() set shader uniform variables
// - called AFTER shader program is created
//-------------------------------------------------------------
bool Texture::setProgram(){
	GLuint program=GLSLMgr::programHandle();
	if(!program)
		return false;
	if(!tex_active&&!bump_active)
		return false;
	if(!enabled)
		return false;

	//int tid=num_tids;
	int texid=TerrainProperties::tid;
	tid=texid;
	cout << "Texture::setProgram tid:"<< tid << " 2d:"<< t2d()<<endl;
	glActiveTexture(GL_TEXTURE0+texid);
	begin();

	GLSLVarMgr vars;

	char str[MAXSTR];

	float delta=width()>height()?width():height();
	double tex_ampl=(tex_active && Render.textures()) ? texamp:0.0;
	double bump_ampl=(Render.bumps() && bump_active) ? 3.0*bumpamp:0.0;

	float ts= scale;
	float orders_bump=1.0/delta;
	//float logf = log2(ts)+log2(orders_delta)-1+0.5*log2(height())+TheScene->freq_mip;

	float dlogf=log2(orders_delta); //
	//float hlog=0.25*log2(height()); // hack to reduce number of tex orders for larger images
	float hlog=0.25*log2(delta); // hack to reduce number of tex orders for larger images
	float logf = log2(ts)+dlogf+hlog; // reduces tex max_orders set in MapNode::Svertex
	//if(t3d())
	//	logf-=log2(1e7);
    logf-=t3d()?log2(5e7):0;
	TerrainProperties *tp=TerrainData::tp;
	double bumpmin=1e-5;
	double minscale=32;
	if(tp){
		for(int i=0;i<tp->noise.size;i++){
			TNnoise* noise=tp->noise[i];
			minscale=minscale<noise->scale?minscale:noise->scale;
		}
	}

	// adjust bump delta using lowest order scale
	double bumpdelta=pow(10,-(0.25*minscale+3.5-2*TheScene->bump_mip));
	bumpdelta=bumpdelta<1e-9?1e-9:bumpdelta;
	double tex_bias=t2d()?bias:0;
 	sprintf(str,"samplers2d[%d]",tid);    		
 	glUniform1iARB(glGetUniformLocationARB(program,str),tid);

 	float near_bias=mipmap()?0:1;
 	float far_bias=mipmap()?0:1;
 	float tbias=0;
//	Planetoid *orb=(Planetoid *)TNode::getOrbital(get_root());
//	if(orb && (orb->type()==ID_PLANET || orb->type()==ID_MOON)){
//		
//	}

  	//sprintf(str,"tex2d[%d].id",tid);    		glUniform1iARB(glGetUniformLocationARB(program,str),tid);
	sprintf(str,"tex2d[%d].texamp",tid);   		glUniform1fARB(glGetUniformLocationARB(program,str),tex_ampl);
	sprintf(str,"tex2d[%d].bumpamp",tid);  		glUniform1fARB(glGetUniformLocationARB(program,str),bump_ampl);
	sprintf(str,"tex2d[%d].bump_delta",tid);    glUniform1fARB(glGetUniformLocationARB(program,str),bumpdelta);
	sprintf(str,"tex2d[%d].bump_damp",tid);     glUniform1fARB(glGetUniformLocationARB(program,str),bump_damp);
	sprintf(str,"tex2d[%d].orders",tid);   		glUniform1fARB(glGetUniformLocationARB(program,str),orders);
	sprintf(str,"tex2d[%d].scale",tid);    		glUniform1fARB(glGetUniformLocationARB(program,str),scale);
	sprintf(str,"tex2d[%d].bias",tid); 			glUniform1fARB(glGetUniformLocationARB(program,str),tex_bias);
	sprintf(str,"tex2d[%d].orders_delta",tid);  glUniform1fARB(glGetUniformLocationARB(program,str),orders_delta);
	sprintf(str,"tex2d[%d].orders_bump",tid);	glUniform1fARB(glGetUniformLocationARB(program,str),orders_bump);
	sprintf(str,"tex2d[%d].orders_atten",tid);  glUniform1fARB(glGetUniformLocationARB(program,str),orders_atten);
	sprintf(str,"tex2d[%d].logf",tid);          glUniform1fARB(glGetUniformLocationARB(program,str),logf);
	sprintf(str,"tex2d[%d].dlogf",tid);         glUniform1fARB(glGetUniformLocationARB(program,str),dlogf);
	sprintf(str,"tex2d[%d].phi_bias",tid);      glUniform1fARB(glGetUniformLocationARB(program,str),phi_bias);
	sprintf(str,"tex2d[%d].height_bias",tid);   glUniform1fARB(glGetUniformLocationARB(program,str),height_bias/Rscale);
	sprintf(str,"tex2d[%d].bump_bias",tid);     glUniform1fARB(glGetUniformLocationARB(program,str),bump_bias);
	sprintf(str,"tex2d[%d].slope_bias",tid);    glUniform1fARB(glGetUniformLocationARB(program,str),slope_bias);
	sprintf(str,"tex2d[%d].far_bias",tid);      glUniform1fARB(glGetUniformLocationARB(program,str),far_bias);
	sprintf(str,"tex2d[%d].near_bias",tid);     glUniform1fARB(glGetUniformLocationARB(program,str),near_bias);
    sprintf(str,"tex2d[%d].tilt_bias",tid);     glUniform1fARB(glGetUniformLocationARB(program,str),tilt_bias);
    sprintf(str,"tex2d[%d].randomize",tid);     glUniform1iARB(glGetUniformLocationARB(program,str),randomized());
    sprintf(str,"tex2d[%d].seasonal",tid);      glUniform1iARB(glGetUniformLocationARB(program,str),seasonal());
    sprintf(str,"tex2d[%d].t1d",tid);           glUniform1iARB(glGetUniformLocationARB(program,str),t1d());
    sprintf(str,"tex2d[%d].triplanar",tid);     glUniform1iARB(glGetUniformLocationARB(program,str),triplanar());
    sprintf(str,"tex2d[%d].t3d",tid);           glUniform1iARB(glGetUniformLocationARB(program,str),t3d());
#ifdef DEBUG_TEXTURES
    cout<<tid<<" Terrain ID:"<<tp->id<<" texture id:"<<tid<<" 1d:"<<t1d()<<" scale:"<<scale<<" triplanar:"<<triplanar()<<endl;
#endif
    //	double dfactor=0.5*GLSLMgr::wscale;
//    double zn=log2(0.2*dfactor/TheScene->znear);
//    double zf=log2(1.5*dfactor/TheScene->zfar);
//
//    cout<<"zn:"<<zn<<" zf:"<<zf<<" logf:"<<logf<<" df1:"<<zn-logf<<" df2:"<<zf-logf<<endl;
	vars.setProgram(program);
	vars.loadVars();
	glActiveTexture(GL_TEXTURE0);
	return true;
}
