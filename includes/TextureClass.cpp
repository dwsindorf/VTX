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

extern double Theta, Phi,Rscale;
static TerrainData Td;

#define TEXFLOOR // makes tex coords modulo scale (fixes float precision problems)
#define FIX_T0	 // corrects tex coords discontinuity at theta=0.0

#define DEBUG_TEXTURES
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
	id[0]=id[1]=0;
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
	bump_bias=0;
	phi_bias=0;
	height_bias=0;
	slope_bias=0;
	tid=0;
	s=0;
	t=0;
}

//-------------------------------------------------------------
// Texture::~Texture() Destructor
//-------------------------------------------------------------
Texture::~Texture()
{
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
	if(id[0]>0)
		glDeleteTextures(2, &id[0]);
	id[0]=id[1]=0;
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
// Texture::getTexCoords() return texture lookup coordinates
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
#endif
	x=tv-tf; // width lookup
	y=sv-sf; // tx=s-0.5+1
}

//-------------------------------------------------------------
// Texture::texCoords() set texture coordinates in OGL passes
//-------------------------------------------------------------
void Texture::texCoords(int tchnl)
{
	double sv=0,tv=0;
	getTexCoords(sv,tv);
	glMultiTexCoord2d(tchnl,sv,tv);
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
		glMultiTexCoord2d(tchnl,sv+x,0);
	else
		glMultiTexCoord2d(tchnl,sv+x,tv+y);
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
	int tid=0;
	if(!valid || Render.invalid_textures()) {
		//cout<<"Texture::begin()"<<endl;
		if (id[tid]>0)
			glDeleteTextures(2, (GLuint*)&id);
		glGenTextures(1, &id[tid]); // Generate a unique texture ID
		glBindTexture(GL_TEXTURE_2D, id[0]);
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
		double amin=255;
		double amax=0;
		bool set_alpha=shader_pass || alpha_image;
		bool auto_alpha= alpha_image;
		
		//cout<<"2d:"<< image()->tx2d()<< " auto_alpha:"<< auto_alpha << " rgba_image:"<< rgba_image << " alpha_image:"<< alpha_image << endl;

		double a=1;
		double b=0;
		double ave=0;
		int n=h*w;
		if(norm && rgba_image){
			for (int i = 0; i < h; i++){
				for (int j = 0; j< w ; j++) {
					int index=i*w+j;
					int rgb_index=index*rgb_step;
					unsigned char ac=255;
			    	if(alpha_image)
			    		ac=rgb[rgb_index+3];			    	
			    	else 
			    		ac=(rgb[rgb_index]+rgb[rgb_index+1]+rgb[rgb_index+2])/3;
					ac*=alpha;
					ac=ac>=255?255:ac;
					amin=ac<amin?ac:amin;
					amax=ac>=amax?ac:amax;
					ave+=ac;
				}
			}
			a=255/(amax-amin);
			b=-a*amin;
			ave/=n;
			cout << "min:"<< amin << " max:"<< amax<< " ave:"<<ave<<endl;
		}
		for (int i = 0; i < h; i++){
			for (int j = 0; j< w ; j++) {
			    unsigned char ac=255;
			    int index=i*w+j;
			    int rgb_index=index*rgb_step;
			    data[index*4+0]=rgb[rgb_index+0];
			    data[index*4+1]=rgb[rgb_index+1];
			    data[index*4+2]=rgb[rgb_index+2];
			    int ci=index*4;

			    if(set_alpha){
			    	if(rgba_image)
			    		ac=alpha*rgb[rgb_index+3];
			    	else if(auto_alpha)
			    		ac=alpha*(rgb[rgb_index]+rgb[rgb_index+1]+rgb[rgb_index+2])/3;
			    	ac=a*ac+b;
					ac=ac>=255?255:ac;
					data[index*4+3]=(unsigned char)(ac);
			    }
			    else
			    	data[index*4+3]=255;
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		// build bump texture image for OGL legacy bumpmap usage (non-shader)

		if(bump_active && !Render.draw_shaded()){
			glGenTextures(1, &id[++tid]); // Generate a unique texture ID
			glBindTexture(GL_TEXTURE_2D, id[1]);
			set_state();
			for (int i = 0; i < h; i++){
				for (int j = 0; j< w ; j++) {
				    unsigned char ac=255;
					int index=i*w+j;
				    int rgb_index=index*rgb_step;
					if(rgba_image)
						ac=rgb[rgb_index+3];
					else{
						ac=(rgb[rgb_index]+rgb[rgb_index+1]+rgb[rgb_index+2])/3;
					}
					data[index*4+0]=data[index*4+1]=data[index*4+2]=ac;
					data[index*4+3]=255;
				}
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		FREE(data);
		valid=true;
	}
	if(Raster.bumps())
		glBindTexture(GL_TEXTURE_2D, id[1]);
	else{
		glBindTexture(GL_TEXTURE_2D, id[0]);
	}
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
	GLhandleARB program=GLSLMgr::programHandle();
	if(!program)
		return false;
	if(!tex_active&&!bump_active)
		return false;
	if(!enabled)
		return false;

	//int tid=num_tids;
	int texid=TerrainProperties::tid;
	tid=texid;
	//cout << "Texture::setProgram tid:"<< tid << " texid:"<< texid<<endl;
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
	float hlog=0.25*log2(height()); // hack to reduce number of tex orders for larger images
	float logf = log2(ts)+dlogf+hlog; // reduces tex max_orders set in MapNode::Svertex

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
 	sprintf(str,"samplers2d[%d]",tid);    		glUniform1iARB(glGetUniformLocationARB(program,str),tid);

 	float near_bias=mipmap()?0:1;
 	float far_bias=mipmap()?0:1;

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
	sprintf(str,"tex2d[%d].phi_bias",tid);      glUniform1fARB(glGetUniformLocationARB(program,str),20*phi_bias);
	sprintf(str,"tex2d[%d].height_bias",tid);   glUniform1fARB(glGetUniformLocationARB(program,str),height_bias/Rscale);
	sprintf(str,"tex2d[%d].bump_bias",tid);     glUniform1fARB(glGetUniformLocationARB(program,str),bump_bias);
	sprintf(str,"tex2d[%d].slope_bias",tid);    glUniform1fARB(glGetUniformLocationARB(program,str),slope_bias);
	sprintf(str,"tex2d[%d].far_bias",tid);      glUniform1fARB(glGetUniformLocationARB(program,str),far_bias);
	sprintf(str,"tex2d[%d].near_bias",tid);     glUniform1fARB(glGetUniformLocationARB(program,str),near_bias);

    sprintf(str,"tex2d[%d].randomize",tid);     glUniform1iARB(glGetUniformLocationARB(program,str),randomized());
    sprintf(str,"tex2d[%d].t1d",tid);           glUniform1iARB(glGetUniformLocationARB(program,str),t1d());
#ifdef DEBUG_TEXTURES
    cout<<"Terrain ID:"<<tp->id<<" texture id:"<<tid<<" 1d:"<<t1d()<<" bias:"<<bias<<" scale:"<<scale<<" texamp:"<<tex_ampl<<" far_bias:"<<far_bias<<" near_bias:"<<near_bias<<endl;
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
