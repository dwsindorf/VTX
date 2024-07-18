
#include "Effects.h"
#include "RenderOptions.h"
#include "AdaptOptions.h"
#include "matrix.h"
#include "MapNode.h"
#include "MapClass.h"
#include "KeyIF.h"
#include "matrix.h"
#include "GLSLMgr.h"
#include "GLSLVars.h"
#include "defs.h"

extern int test3,test5;

EffectsMgr Raster;	// one and only global object

#define	JITTER_SIZE 16
#define SMAPTEXID   1
#define JMAPTEXID   2
//#define DEBUG_EFFECTS

static const char *pgmnames[]={"RENDER","EFFECTS","POSTPROC","SHADOW_ZVALS","SHADOWS","SHADOWS_FINISH","PLANT_ZVALS","PLANT_SHADOWS"};
// Effects supported (using shaders)
//  1. haze (horizontal)
//  2. vfog (vertical)
//     TODO: support shader density distribution functions
//  3. water (reflection, transparency)
//     TODO: generate and use "sky map"
//  4. anti-aliasing
//  5. shadows
//     TODO: multiple objects (background shadows)
//     TODO: optimize blur (in current scheme shadow edge width is view dependent)
//     TODO: figure out why "proj" mode results are so view dependent
//  6. HDR (new)
//     TODO: develop a better tone mapping algorithm
// Algorithms
// I. haze,fog, water effects
//  1. Scene objects rendered in three passes
//  2. Render pass (all objects): (output=RTT)
//     writes gl_FragData[0] = image
//     writes gl_FragData[1] = FBO data (r=illumination)
//     shader generated from Object::setProgram
//     CallList from MapNode::SVertex data
//  3. Raster-render pass (viewobj only): (output=RTT)
//     gl_FragData[0] = FBOTex1   pass-thru image data from Render pass
//     gl_FragData[1] = raster data
//     shader from Effects.cpp::setProgram (auximage.vert,auximage.frag)
//     CallList generated from EffectsMgr::Vertex data
//     in auximage.frag gl_FragCoord.z saved in gl_FragData[2].r
//  4. Raster-apply passes: (output=Frame Buffer)
//     FBOTex1   contains image
//     FBOTex2   contains raster helper data
//     Effects.cpp::setProgram loads effects.vert,effects.frag
//     depth data in FBOTex3.a converted to distance
//     Haze color alpha blended with FBOTex1 (image) using distance
//  Notes:
//  1. In (2-4) gl_FragData[1].b = tid acts as a fg/bg flag
//     - In (2) Object shaders need to write out gl_FragData[1].b=0
//  2. In (2) for sky semi-transparency illumination for outside
//     objects (moons,stars etc) encoded in gl_FragData[1].r
//     - in sky shader background objects get alpha ~ illumination
//     - at sky shader end (and cloud shader) gl_FragData[1].r reset to zero.
//  3. disabling GL_BLEND in (3) and (4) frees up .a component in FBOTex1
//     - in (3) illumination is calculated in v.s. and saved in gl_FragData[1].a
//  4. passing depth in as an attribute results in insufficient precision unless
//     RTT buffers given a data size of 32 bits (which causes older graphics cards
//     to run glacially slow)
//     - could not figure out how to create a texture from GLSLMgr::dbo_rect
//     - but capturing depth buffer in (2) and converting z to distance in (3)
//       resulted in similar result to using 32 bit RTT buffers (not sure why).
// II. anti-aliasing, HDR
//  1. image processing
//     - uses RTT texture generated in I above as input
// III. shadows

EffectsMgr::EffectsMgr(){
	shadowMapFBO=0;
	depthTextureId=0;
	jitterTextureId=0;
	shadow_map_ratio=4;
	shadow_map_height=0;
	shadow_map_width=0;
	shadow_proj=false;
}
EffectsMgr::~EffectsMgr(){
	if(shadowMapFBO>0)
		glDeleteFramebuffers(1, &shadowMapFBO);
	if(depthTextureId>0)
		glDeleteTextures(1,&depthTextureId);
	if(jitterTextureId>0)
		glDeleteTextures(1,&jitterTextureId);
}

//-------------------------------------------------------------
// EffectsMgr::init_render()	collect enables for raster scans
//-------------------------------------------------------------
void EffectsMgr::init_render()
{
	bool old_mode=do_shaders;
	RasterMgr::init_render();
	do_shaders=do_vfog||do_haze||do_water||do_edges||do_hdr;
	if(!Render.draw_shaded())
		do_shaders=false;
	if(TheScene->light_view()|| TheScene->test_view()){
		do_shaders=true;
		do_shadows=do_water=do_haze=do_fog=false;
	}

	if(old_mode !=do_shaders){
	   TheScene->set_changed_render();
	   Raster.shadow_count=0;
	}
}

//-------------------------------------------------------------
// EffectsMgr::render()	create effects image
//-------------------------------------------------------------
void EffectsMgr::render() {
	//setView();
	//cout << "EffectsMgr::render()"<<endl;
	if (!Render.draw_shaded()){  //  if using shaders all done in apply call
		RasterMgr::render();
	}
	else {
		GLSLMgr::setFBOReset();
		GLSLMgr::renderToFrameBuffer=false; // divert output to RTT
		//glEnable(GL_BLEND);
	}
}

//-------------------------------------------------------------
// EffectsMgr::setApplyProgram()	apply raster effects to image
//-------------------------------------------------------------
void EffectsMgr::setProgram(int type){
	GLint    vport[4];
	TheScene->getViewport(vport);
	GLSLVarMgr vars;
	bool effects=do_vfog||do_haze||do_water;
	Color c;
	Point p;
	double zn=TheScene->znear;
	double zf=TheScene->zfar;
	double ws1=1/zn;
	double ws2=(zn-zf)/zf/zn;
	double af=rampstep(0,TheScene->maxht,TheScene->height,1,0);
	double wht=(TheScene->elevation-sea_level)/FEET;
	GLSLMgr::clrDefString();

#ifdef DEBUG_EFFECTS
	   cout << "loading program:"<<pgmnames[type]<<endl;
#endif
	char defs[512]="";
	switch(type){
	case PLANT_ZVALS:
	case SHADOW_ZVALS:
		if(type==PLANT_ZVALS)
			GLSLMgr::loadProgram("plants.shadows.gs.vert","shadows_zvals.frag","plants.shadows.geom");
		else if(TheMap && TheMap->hasGeometry()){
			sprintf(GLSLMgr::defString+strlen(GLSLMgr::defString),"#define TESSLVL %d\n",Map::tessLevel());
			TheMap->setGeometryDefs();
		    GLSLMgr::loadProgram("shadows.gs.vert","shadows_zvals.frag","shadows_zvals.geom");
		    TheMap->setGeometryPrgm();
	    }
		else
		    GLSLMgr::loadProgram("shadows_zvals.vert","shadows_zvals.frag");
		vars.newBoolVar("lighting",false);

		break;
	case PLANT_SHADOWS:
		if(test5)
			sprintf(defs+strlen(defs),"#define LEAF_TEST\n");
		sprintf(defs+strlen(defs),"#define SHADOWS\n");
	case SHADOWS:
		if(shadow_proj)
			sprintf(defs+strlen(defs),"#define USING_PROJ\n");
		if(debug_shadows() && shadow_test>0)
			sprintf(defs+strlen(defs),"#define SHADOW_TEST\n");

		GLSLMgr::setDefString(defs);
		if(type==PLANT_SHADOWS)
			GLSLMgr::loadProgram("plants.shadows.gs.vert","shadows.frag","plants.shadows.geom");
		else if(TheMap && TheMap->hasGeometry()){
			sprintf(GLSLMgr::defString+strlen(GLSLMgr::defString),"#define TESSLVL %d\n",Map::tessLevel());
			TheMap->setGeometryDefs();
			GLSLMgr::loadProgram("shadows.gs.vert","shadows.frag","shadows.geom");
		    TheMap->setGeometryPrgm();
	    }
		else
		    GLSLMgr::loadProgram("shadows.vert","shadows.frag");

		vars.newFloatArray("smat",smat,16);
		vars.newIntVar("ShadowMap",SMAPTEXID);
		vars.newIntVar("JitterMap",JMAPTEXID);
		vars.newFloatVar("shadowmap_ratio",shadow_map_ratio);
		vars.newFloatVar("xmax",GLSLMgr::width*shadow_map_ratio);
		vars.newFloatVar("ymax",GLSLMgr::height*shadow_map_ratio);
		vars.newFloatVar("zmax",s_zmax);
		vars.newFloatVar("zmin",s_zmin);
		vars.newFloatVar("shadow_intensity",shadow_intensity);
		vars.newIntVar("light_index",light_index());
		vars.newBoolVar("lighting",true);

		if(shadow_proj)
			vars.newFloatVar("fwidth",shadow_blur*0.1);
		else
			vars.newFloatVar("fwidth",shadow_blur);
		p=TheScene->viewobj->point.mm(TheScene->invViewMatrix);
		p=p.mm(TheScene->viewMatrix);
		Raster.center=p;
		p=light()->point;
		vars.newFloatVec("Light",p.x,p.y,p.z);
		vars.newIntVar("view",shadow_vcnt);
		vars.newIntVar("views",shadow_vsteps);
		vars.newIntVar("shadow_test",shadow_test);
		break;

	case SHADOWS_FINISH:
		GLSLMgr::setDefString(defs);
		GLSLMgr::loadProgram("shadows_finish.vert","shadows_finish.frag");
		break;


	case RENDERPGM:  // auximage.frag - water effects
		if(Render.lighting())
			sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),Lights.size);
		else
			sprintf(defs,"#define LMODE %d\n#define NLIGHTS %d\n",Render.light_mode(),0);

		if(shadows())
			sprintf(defs+strlen(defs),"#define SHADOWS\n");
		if(TheScene->inside_sky())
			sprintf(defs+strlen(defs),"#define SKY\n");
		else{
			twilite_min=-1;
			twilite_max=1;
			twilite_dph=0;
		}
		GLSLMgr::setDefString(defs);
		GLSLMgr::loadProgram("auximage.vert","auximage.frag");
		c=sky_color;

		vars.newFloatVec("WaterSky",c.red(),c.green(),c.blue(),c.alpha());

		vars.newFloatVec("WaterColor1",water_color1.red(),water_color1.green(),water_color1.blue(),water_color1.alpha());
		vars.newFloatVec("WaterColor2",water_color2.red(),water_color2.green(),water_color2.blue(),water_color2.alpha());
		vars.newFloatVec("IceColor1",ice_color1.red(),ice_color1.green(),ice_color1.blue(),ice_color1.alpha());
		vars.newFloatVec("IceColor2",ice_color2.red(),ice_color2.green(),ice_color2.blue(),ice_color2.alpha());

		vars.newFloatVar("ice_clarity",ice_clarity);
		vars.newFloatVar("water_clarity",water_clarity);

		vars.newFloatVar("water_reflection",water_color1.alpha());
		vars.newFloatVar("ice_reflection",ice_color1.alpha());

		vars.newFloatVar("water_saturation",water_color2.alpha()*water_modulation());
		vars.newFloatVar("ice_saturation",ice_color2.alpha()*water_modulation());

		vars.newFloatVar("water_mix",water_mix*af);
		vars.newFloatVar("ice_mix",ice_mix*af);

		vars.newFloatVar("water_dpr",water_dpr*water_color1.alpha());
		vars.newFloatVar("ice_dpr",water_dpr*ice_color1.alpha());

		vars.newFloatVar("dpm",water_dpm);

		vars.newFloatVar("twilite_min",twilite_min);
		vars.newFloatVar("twilite_max",twilite_max);
		vars.newFloatVar("twilite_dph",twilite_dph);
		vars.newFloatVec("Shadow",shadow_color.red(),shadow_color.green(),shadow_color.blue(),shadow_value);
		break;

	case EFFECTSPGM:
		if(do_haze)
			sprintf(defs+strlen(defs),"#define HAZE\n");
		if(do_vfog)
			sprintf(defs+strlen(defs),"#define FOG\n");
		if(do_water){
			sprintf(defs+strlen(defs),"#define WATER\n");
			if(reflections()){
				sprintf(defs+strlen(defs),"#define REFLECT\n");
				if(Render.raytrace())
					sprintf(defs+strlen(defs),"#define RAYTRACE\n");
			}
		}
		if(shadows()){
			if(shadow_test>0 && debug_shadows())
				sprintf(defs+strlen(defs),"#define SHADOW_TEST\n");
			else
				sprintf(defs+strlen(defs),"#define SHADOWS\n");
		}
		GLSLMgr::setDefString(defs);
		GLSLMgr::loadProgram("effects.vert","effects.frag");
		vars.newIntVar("shadow_test",shadow_test);
		c = haze_color;
		//c.print();
		vars.newFloatVec("Haze",c.red(),c.green(),c.blue(),c.alpha());
		vars.newFloatVar("haze_grad",haze_grad);
		vars.newFloatVar("haze_zfar",haze_zfar);
		vars.newFloatVar("haze_ampl",haze_hf);
		c=fog_color;
		vars.newFloatVec("Fog",c.red(),c.green(),c.blue(),c.alpha());
		vars.newFloatVar("fog_ampl",fog_vf);

		vars.newFloatVar("ice_reflection",ice_color1.alpha());
		vars.newFloatVar("water_reflection",water_color1.alpha());

		vars.newFloatVar("water_dpr",water_dpr*water_color1.alpha());
		vars.newFloatVar("ice_dpr",water_dpr*ice_color1.alpha());

		vars.newFloatVec("Shadow",shadow_color.red(),shadow_color.green(),shadow_color.blue(),shadow_value);
		break;

	case POSTPROCPGM:
	    float texCoordOffsets[5*5*2];
	    for(int i = 0; i< 5; i++){
	    	for(int j = 0; j< 5; j++){
	    		int idx=(((i*5)+j)*2)+0;
	            texCoordOffsets[idx] = -2.0 + j;
	            texCoordOffsets[idx+1] = -2.0 + i;
	    	}
	    }
		if(do_hdr)
			sprintf(defs+strlen(defs),"#define HDR\n");
    	if(Raster.filter_show())
    		sprintf(defs+strlen(defs),"#define SHOW\n");
		if(do_edges){
			sprintf(defs+strlen(defs),"#define EDGES\n");
			if(filter_big())
				sprintf(defs+strlen(defs),"#define BIG_KERNEL\n");
//			switch(filter_type()){
//			case FILTER_TYPE1:
//				sprintf(defs+strlen(defs),"#define FILTER1\n");
//				break;
//			case FILTER_TYPE2:
//				sprintf(defs+strlen(defs),"#define FILTER2\n");
//				break;
//			default:
//				sprintf(defs+strlen(defs),"#define FILTER3\n");
//				break;
//			}
		}
		if(!effects)
			sprintf(defs+strlen(defs),"#define ONEPASS\n");
		GLSLMgr::setDefString(defs);
		GLSLMgr::loadProgram("postproc.vert","postproc.frag");
		GLhandleARB program=GLSLMgr::programHandle();
		int uniformLoc = glGetUniformLocationARB(program, "tc_offset" );
		if( uniformLoc > -1 )
			glUniform2fvARB( uniformLoc, 25, texCoordOffsets );
		else
			printf("tc_offset not in program");
		vars.newFloatVar("color_ampl",filter_color_ampl);
		vars.newFloatVar("normal_ampl",filter_normal_ampl);
		vars.newFloatVar("hdr_min",hdr_min);
		vars.newFloatVar("hdr_max",hdr_max);
		break;
	}
	vars.newFloatVar("fog_znear",fog_min);
	vars.newFloatVar("fog_zfar",fog_max);
	vars.newFloatVar("fog_vmin",fog_vmin);
	vars.newFloatVar("fog_vmax",fog_vmax);
	vars.newFloatVar("ws1",ws1);
	vars.newFloatVar("ws2",ws2);
	vars.newFloatVar("dh",1.0/vport[2]);
	vars.newFloatVar("dv",1.0/vport[3]);
	vars.newFloatVar("zf",TheScene->zfar);
	vars.newFloatVar("zn",TheScene->znear);
	vars.newFloatVar("feet",FEET);
	vars.newFloatVar("fov",TheScene->fov*RPD);
	p=TheScene->xpoint;
	vars.newFloatVec("pv",p.x,p.y,p.z);

	// Render Program variables

	vars.newFloatVec("center",center.x,center.y,center.z);
	GLhandleARB program=GLSLMgr::programHandle();

	if(!program){
		cout << "could not load program"<<endl;
		return;
	}
	if(GLSLMgr::CommonID1 >= 0)
		GLSLMgr::CommonID1=glGetAttribLocation(program,"CommonAttributes1"); // Constants1

 	vars.setProgram(program);
	vars.loadVars();
	GLSLMgr::loadVars();
	GLSLMgr::setProgram();
	TheScene->setProgram();
}

//-------------------------------------------------------------
// EffectsMgr::apply()	apply effects image
//-------------------------------------------------------------
void EffectsMgr::apply(){

	if (Render.draw_shaded()&& do_shaders){
		set_render_type(SHADERS);
		GLSLMgr::beginRender();
	    // pass 1 : load auximage shaders. write depth etc. to FBO2
		GLSLMgr::pass=0;
		TheScene->set_frontside();
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear(GL_DEPTH_BUFFER_BIT); // note: color buffer has scene image
		//glDisable(GL_BLEND);
		surface=1;
		if(do_water){
			set_show_water(1);
			surface=2;
			glDisable(GL_BLEND);
			//set_all();
			setProgram(RENDERPGM);

			render_auximage(); // render land surface note: calls glDisable(GL_BLEND)
			glFlush();
			Color ambient=((Planetoid*)TheScene->viewobj)->ambient;
			Color diffuse=((Planetoid*)TheScene->viewobj)->diffuse;
			Lights.setAmbient(ambient);
			Lights.setDiffuse(diffuse);
			glColor4d(1,0,0,1);
			set_show_water(0);
			surface=1;
		}

	    // following passes use full screen render (2 triangles)

		surface=1;
		bool effects=do_vfog||do_haze||do_water;

		GLSLMgr::renderToFrameBuffer=true;// for read pass set output to FB
		if(effects /*&& (do_edges||do_hdr)*/){
			// sometimes get artifacts when reading and writing to same draw buffers in two-pass render
			// - need to "pong-pong" between buffer objects
			// - write to buffers 3&4 in effects pass (maps to FBOTex3 & FBOTex4)
			// - in postproc pass read image and flags from FBOTex3 & FBOTex4 and write out image to buffer 0
			set_effects(true);
			GLSLMgr::setFBOWritePass();
		    GLenum mrt1[] = {GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT};
			glDrawBuffers(2,mrt1);
			setProgram(EFFECTSPGM);
			GLSLMgr::drawFrameBuffer();
		    GLenum mrt2[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT}; // restores default case
			glDrawBuffers(2,mrt2);
			GLSLMgr::setFBOReadPass();
			setProgram(POSTPROCPGM);
			GLSLMgr::drawFrameBuffer();
		}
		else if(shadows()&&shadow_test&&debug_shadows()){
			GLSLMgr::setFBOReadPass();
			setProgram(EFFECTSPGM);
			GLSLMgr::drawFrameBuffer();
		}
		else if(do_edges||do_hdr){
			GLSLMgr::setFBOReadPass();
			setProgram(POSTPROCPGM);
			GLSLMgr::drawFrameBuffer();
		}
		GLSLMgr::endRender();
		set_render_type(0);
		glEnable(GL_BLEND);
	}
	else if(!Render.draw_shaded()){ // uses old non-shader code
		RasterMgr::apply(); //
	}
}

//-------------------------------------------------------------
// EffectsMgr::render_shadows() render scene with shadows
//-------------------------------------------------------------
void EffectsMgr::render_shadows(){
	GLenum mrt1[] = {GL_COLOR_ATTACHMENT0_EXT,GL_COLOR_ATTACHMENT1_EXT};
	GLenum mrt2[] = {GL_COLOR_ATTACHMENT2_EXT,GL_COLOR_ATTACHMENT3_EXT};

	if(!Render.draw_shaded()){
		glViewport(0,0,GLSLMgr::width,GLSLMgr::height);
		RasterMgr::render_shadows();
		return;
	}

	if(shadow_map_width!=GLSLMgr::width || shadow_map_height!=GLSLMgr::height){
		shadow_map_width=GLSLMgr::width;
		shadow_map_height=GLSLMgr::height;
		initShadowTexture();
	}
	int shadowMapWidth = shadow_map_width * shadow_map_ratio;
	int shadowMapHeight = shadow_map_height * shadow_map_ratio;
	double nscale;
	int cnt=0,i,start,end,j;

	if(jitterTextureId==0)
		create_jitter_lookup(JITTER_SIZE, 8, 8);	// 8 'estimation' samples, 64 total samples

	glViewport(0,0,shadowMapWidth,shadowMapHeight);
	//set_shadows_mode(1);

	init_shadows();  // set "vport" in Raster.cpp to light depth size

	nscale=all_lights()?1.0/Lights.size:1;

	end=Lights.size;
	start=0;

	if(last_light())
		start=end-1;
	else if(first_light())
		end=start+1;

	// multi-light loop

	int shadow_lights=end-start;
	double shad_scale=shadow_value*shadow_darkness;

	bool tmp=Map::use_call_lists;
	Map::use_call_lists=false;
	glUseProgramObjectARB(0); //Using the fixed pipeline to render to the depth buffer

	GLSLMgr::clrBuffers();

	GLSLMgr::setFBOWritePass();
	glDrawBuffers(1,mrt2);
	GLSLMgr::clrBuffers();
	glDrawBuffers(1,mrt1);
	GLSLMgr::clrBuffers();

	for(i=start;i<end;i++){
		set_light(i);
		if(!light()->isEnabled())
			continue;
		shadow_intensity=Lights.intensityFraction(light(),TheScene->shadowobj->point);
		//shadow_intensity*=lerp(ldp,-0.5,-1,1,0.5);
		//cout << shadow_intensity << endl;
		init_view();
		j=0;
		glEnable(GL_DEPTH_TEST);

		while(more_views()){
			//GLSLMgr::setFBOWritePass();
			// set light POV
			// 1. render depth from light pov (using shadow map FBO)

			glBindFramebuffer(GL_FRAMEBUFFER_EXT,shadowMapFBO); //Rendering offscreen

			glUseProgramObjectARB(0); //Using the fixed pipeline to render to the depth buffer
			glViewport(0,0,shadowMapWidth,shadowMapHeight);

			//enableShadowMap(false);
			Render.set_back();
			GLSLMgr::clrDepthBuffer();

			set_light_view();

		    glPolygonOffset(4.0f, 2.0f); // increased from 2,1 reduced artifacts
			glEnable(GL_POLYGON_OFFSET_FILL);

			TheScene->shadows_zvals();

			glDisable(GL_POLYGON_OFFSET_FILL);

			// 2. render depth from eye (using shadows shader)
			//    shadow->FBOTex1
			glViewport(0,0,GLSLMgr::width,GLSLMgr::height);

			Render.set_front();
			TheScene->project();

			GLSLMgr::setFBOWritePass();

			enableShadowMap(true);

			glDrawBuffers(1,mrt1);

			TheScene->shadows_normals();

			// 3. copy shadow view intensity to accumulation buffer
			//    FBOTex1->FBOTex3

			glDrawBuffers(1,mrt2);

			enableShadowMap(false);
			setProgram(SHADOWS_FINISH);
			GLSLMgr::drawFrameBuffer();

			next_view();

			j++;
			cnt++;
		}
		if(!all_lights())
			break;
	}
	glDrawBuffers(2,mrt1);
	set_render_type(0);
	glUseProgramObjectARB(0);
	//glEnable(GL_BLEND);

	//shadow_vsteps=views;
	GLSLMgr::clrBuffers();
	Map::use_call_lists=tmp;
	GLSLMgr::setTessLevel(Map::tessLevel());
	//set_shadows_mode(0);

	// use shadow intensity data from FBO3 in later passes
}

//-------------------------------------------------------------
// RasterMgr::vertex() render image vertex
// 			  called from MapNode::Rvertex
//-------------------------------------------------------------
void EffectsMgr::renderBgShadows(){
	if(!Render.draw_shaded()){
		RasterMgr::renderBgShadows();
		return;
	}
	//renderFgShadows();

}
void EffectsMgr::applyBgShadows(){
	if(!Render.draw_shaded()){
		RasterMgr::applyBgShadows();
		return;
	}
}
//-------------------------------------------------------------
// EffectsMgr::init_shadows() initialize shadow pass
//-------------------------------------------------------------
void EffectsMgr::init_shadows(){
	RasterMgr::init_shadows();
}

void EffectsMgr::applyShadows(){
	if(!Render.draw_shaded()){
		RasterMgr::applyShadows();
		return;
	}
}

//-------------------------------------------------------------
// EffectsMgr::renderFgShadows() generate shadow map
//-------------------------------------------------------------
void EffectsMgr::renderFgShadows(){
	if(!Render.draw_shaded()){
		glViewport(0,0,GLSLMgr::width,GLSLMgr::height);
		RasterMgr::renderFgShadows();
	}
	else
		render_shadows();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

}

void EffectsMgr::setShadowProj(bool b){
	shadow_proj=b;
	initShadowTexture();
}
//-------------------------------------------------------------
// EffectsMgr::initShadowFBO() create an FBO to store a shadow map
//-------------------------------------------------------------
void EffectsMgr::initShadowTexture()
{
	if(shadowMapFBO>0)
		glDeleteFramebuffers(1, &shadowMapFBO);
	if(depthTextureId>0)
		glDeleteTextures(1,&depthTextureId);

	int shadowMapWidth = shadow_map_width * shadow_map_ratio;
	int shadowMapHeight = shadow_map_height * shadow_map_ratio;

	GLenum FBOstatus;
	glActiveTextureARB(GL_TEXTURE0+SMAPTEXID);
	glGenTextures(1, &depthTextureId);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, depthTextureId);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(shadow_proj){
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	}
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	// create a framebuffer object
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, shadowMapFBO);

	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_RECTANGLE_ARB, depthTextureId, 0);

	// check FBO status
	FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");

	// switch back to window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glActiveTextureARB(GL_TEXTURE0); // Switch active texture unit back to 0 again
}
//-------------------------------------------------------------
// EffectsMgr::setShadowMap() set
//-------------------------------------------------------------
void EffectsMgr::enableShadowMap(bool b)
{
	glActiveTextureARB(GL_TEXTURE0+SMAPTEXID);
	if(b)
		glBindTexture(GL_TEXTURE_2D,depthTextureId);
	else{
		glBindTexture(GL_TEXTURE_2D,0);
		glActiveTextureARB(GL_TEXTURE0);
	}
}
// Create 3D texture for per-pixel jittered offset lookup
void EffectsMgr::create_jitter_lookup(int size, int samples_u, int samples_v)
{
	if(jitterTextureId>0)
		glDeleteTextures(1,&jitterTextureId);
    glGenTextures(1, &jitterTextureId); // Generate a unique texture ID
    glActiveTexture(GL_TEXTURE0+JMAPTEXID); // Activate a different texture unit
    glBindTexture(GL_TEXTURE_3D, jitterTextureId); // Bind the texture

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	signed char * data = new signed char[size * size * samples_u * samples_v * 4 / 2];

	for (int i = 0; i<size; i++) {
		for (int j = 0; j<size; j++) {
			float rot_offset = ((float)rand() / RAND_MAX - 1) * 2 * 3.1415926f;
			for (int k = 0; k<samples_u*samples_v/2; k++) {

				int x, y;
				float v[4];

				x = k % (samples_u / 2);
				y = (samples_v - 1) - k / (samples_u / 2);

				// generate points on a regular samples_u x samples_v rectangular grid
				v[0] = (float)(x * 2 + 0.5f) / samples_u;
				v[1] = (float)(y + 0.5f) / samples_v;
				v[2] = (float)(x * 2 + 1 + 0.5f) / samples_u;
				v[3] = v[1];

				// jitter position
				v[0] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_u);
				v[1] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_v);
				v[2] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_u);
				v[3] += ((float)rand() * 2 / RAND_MAX - 1) * (0.5f / samples_v);

				// warp to disk
				float d[4];
				d[0] = sqrtf(v[1]) * cosf(2 * 3.1415926f * v[0]);
				d[1] = sqrtf(v[1]) * sinf(2 * 3.1415926f * v[0]);
				d[2] = sqrtf(v[3]) * cosf(2 * 3.1415926f * v[2]);
				d[3] = sqrtf(v[3]) * sinf(2 * 3.1415926f * v[2]);

				data[(k * size * size + j * size + i) * 4 + 0] = (signed char)(d[0] * 127);
				data[(k * size * size + j * size + i) * 4 + 1] = (signed char)(d[1] * 127);
				data[(k * size * size + j * size + i) * 4 + 2] = (signed char)(d[2] * 127);
				data[(k * size * size + j * size + i) * 4 + 3] = (signed char)(d[3] * 127);
			}
		}
	}

	glTexImage3D(GL_TEXTURE_3D, 0, GL_SIGNED_RGBA_NV, size, size, samples_u * samples_v / 2, 0, GL_RGBA, GL_BYTE, data);

	delete [] data;

}
