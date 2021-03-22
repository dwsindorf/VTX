#include "TerrainData.h"
#include "NodeData.h"
#include "ImageClass.h"
#include "TerrainClass.h"
#include "GLSLMgr.h"
#include "RenderOptions.h"

#include <iostream>
#include <math.h>

class Image;
extern void     set_info(char *c,...);

// notes:
// Timing (330 MHz Ultra10)
//    2.5 us    terrain tree traverse
//    1.0 us    MapPt calculation
//    9.7 us    noise calculation

extern double Rand();

extern NameList<DoubleSym*> Units;
extern int mindex;
extern char tabs[];

// externs used in TerrainClass.cpp

int             test_flag = 1;
TerrainData     Tstack[64];
int             tindex = 0;

//************************************************************
// TerrainSym class
//************************************************************
//-------------------------------------------------------------
// TerrainSym::save() archive
//-------------------------------------------------------------
void TerrainSym::save(FILE *f)
{
	DoubleSym *ds=0;
	if(units)
		ds=Units.inlist((char*)units);
	fprintf(f,"%s%s=",tabs,text);
	if(value.strvalid())
		fprintf(f,"\"%s\"",value.string);
	else if(value.svalid()){
		if(ds)
			fprintf(f,"%g %s",value.s/ds->value,units);
		else
			fprintf(f,"%g",value.s);
	}
	else if(value.pvalid()){
		fprintf(f,"Point(");
		if(ds){
			fprintf(f,"%g %s,",value.p.x/ds->value,units);
			fprintf(f,"%g %s,",value.p.y/ds->value,units);
			fprintf(f,"%g %s)",value.p.z/ds->value,units);
		}
		else
			fprintf(f,"%g,%g,%g)",value.p.x,value.p.y,value.p.z);
	}
	else if(value.cvalid()){
		fprintf(f,"Color(");
		fprintf(f,"%g,",round(value.c.red(),2));
		fprintf(f,"%g,",round(value.c.green(),2));
		fprintf(f,"%g",round(value.c.blue(),2));
		if(value.c.alpha()==1.0)
			fprintf(f,")");
		else
			fprintf(f,",%g)",round(value.c.alpha(),2));

	}
	fprintf(f,";\n");
}

//************************************************************
// TerrainData class
//************************************************************

Image *TerrainData::image=0;
TNclouds *TerrainData::clouds=0;
TerrainData *TerrainData::data[MAX_TDATA];
TerrainData  TerrainData::lower;
TerrainData  TerrainData::zlevel[MAX_TDATA];
int  TerrainData::zlevels=0;

int TerrainData::datacnt=0;
int TerrainData::flags=0;
int TerrainData::tids=0;
int TerrainData::pass=0;

double TerrainData::level=0;
double TerrainData::fractal=0;
double TerrainData::density=0;
double TerrainData::erosion=0;
double TerrainData::hardness=0;
double TerrainData::albedo=1;
double TerrainData::shine=1;
Color TerrainData::ambient=Color(1,1,1,0);
Color TerrainData::emission=Color(1,1,1,0);
Color TerrainData::specular=Color(1,1,1,1);
Color TerrainData::diffuse=Color(1,1,1,1);
double TerrainData::sediment=0;
double TerrainData::height=0;
double TerrainData::clarity=100*FEET;
double TerrainData::rock=0;
double TerrainData::depth=0;
double TerrainData::margin=0;
double TerrainData::extent=0;
double TerrainData::texht=0;

Array<TerrainProperties*>TerrainData::properties;
TerrainProperties *TerrainData::tp=0;

void TerrainData::add_texture(Texture *t)
{
	// add new texture to last property only
	int id=0;
	if(tp)
		id=tp->textures.size;
	t->tid=id;

	if(get_flag(CLRTEXS)){
		tp=properties.last();
		tp->textures.add(t);
	}

	// add new texture to all properties

	else{
		for(int i=ID0;i<properties.size;i++){
			properties[i]->textures.add(t);
		}
	}
}

void TerrainData::add_TNcolor(TNcolor *t){
	tp=properties.last();
	if(tp)
		tp->tncolor=t;
}

void TerrainData::add_TNpoint(TNpoint *t){
	tp=properties.last();
	if(tp){
		tp->set_geometry(true);
		tp->tnpoint=t;
	}
}

void TerrainData::add_TNclouds(TNclouds *t){
	clouds=t;
}

void TerrainData::add_TNnoise(TNnoise *t){
	tp=properties.last();
	if(tp){
		t->id=tp->noise.size;
		tp->noise.add(t);
		if(get_flag(COMPLEX))
			tp->set_complex(true);
	}
}

TerrainProperties *TerrainData::add_id()
{
	tp=properties.last();
	if(tp){
		tp->albedo=albedo;
		tp->shine=shine;
		tp->glow=emission;
		//tp->color=c;
	}
	set_id(tids++);
	tp=new TerrainProperties(id());
	tp->set_type(tids-1);
	properties.add(tp);
	albedo=1;
	shine=1;
	emission=Color(1,1,1,0);
	return tp;
}
void TerrainData::eval_properties()
{
	tp=properties.last();
	tp->set_type(tids-1);
	tp->albedo=albedo;
	tp->shine=shine;
	tp->glow=emission;
	//tp->color=c;

	int i,j;
	Texture *tex;
	for(i=0;i<properties.size;i++){
		tp=properties[i];
		tp->info.s.tsize=0;
		for(j=0;j<tp->textures.size;j++){
			tex=tp->textures[j];
			if(tex->s_data)
				tp->info.s.tsize++;
			if(tex->a_data)
				tp->info.s.tsize++;
		}
	}
	tp=properties.last();
}

//-------------------------------------------------------------
// TerrainData::insert_strata() insert terrain layer into ht
//              ordered strata stack
//-------------------------------------------------------------
void TerrainData::insert_strata(TerrainData &S)
{
	int i,j;
	for(i=0;i<MAX_TDATA-1;i++){
		if(S.p.z>=zlevel[i].p.z){
			for(j=MAX_TDATA-1;j>i;j--){
				if(zlevel[j-1].p.z>TZBAD)
					zlevel[j].copy(zlevel[j-1]);
			}
			zlevel[j].copy(S);
			return;
		}
	}
}

//-------------------------------------------------------------
// TerrainData::begin() initialize strata stack
//-------------------------------------------------------------
void TerrainData::begin()
{
	int i;
	for(i=0;i<MAX_TDATA;i++){
		//zlevel[i].clr_active();
		zlevel[i].p.z=TZBAD;
	}
	//lower.p.z=TZBAD;
	datacnt=0;
	zlevels=0;
}

//-------------------------------------------------------------
// TerrainData::end() process strata stack
//-------------------------------------------------------------
void TerrainData::end()
{
	int i;
	zlevels=0;
	datacnt=0;
	for(i=1;i<MAX_TDATA;i++){
		if(zlevel[i].p.z<=TZBAD)
			break;
		zlevels++;
		datacnt++;
		data[i-1]=&zlevel[datacnt];
	}
}
//************************************************************
// TerrainProperties class
//************************************************************
int TerrainProperties::nid=0;
int TerrainProperties::tid=0;

TerrainProperties::TerrainProperties(int t)
{
	id=t;
	albedo=shine=1;
	glow=Color(1,1,1,0);
	color=Color(1,1,1,0);
	info.l=0;
	tncolor=0;
	tnpoint=0;
}
TerrainProperties::~TerrainProperties(){
	textures.reset();
	noise.reset();
}
void TerrainProperties::initProgram(){
	//Texture::reset();
	char defs[1024]="";
	if(has_color() || Render.colors()){
		sprintf(defs,"#define COLOR\n");
	}
	int bumps=0;
	for(int i=0;i<textures.size;i++){
		if(textures[i]->bump_active && Render.bumps())
		    bumps++;
	}
	int rand=0;
	for(tid=0;tid<textures.size;tid++){
		if(textures[tid]->randomized())
			rand=1;
	}
	int nvals=noise.size+rand;
	sprintf(defs+strlen(defs),"#define NTEXS %d\n#define NBUMPS %d\n#define NVALS %d\n",
			textures.size,bumps,nvals);

	if(noise.size && is_complex())
		sprintf(defs+strlen(defs),"#define CPX\n");
	if(rand)
		sprintf(defs+strlen(defs),"#define NOTILE\n");
	strcat(GLSLMgr::defString,defs);
	Texture::reset();

	for(tid=0;tid<textures.size;tid++){
		if(textures[tid]->randomized())
			rand=true;
		textures[tid]->initProgram();
	}
	if(tncolor)
		tncolor->initProgram();
	if(tnpoint)
		tnpoint->initProgram();
}
int TerrainProperties::passes(){
	int passes=textures.size/4;
	return passes;
}
void TerrainProperties::setProgram(){
	GLSLMgr::setProgram();
	for(tid=0;tid<textures.size;tid++){
		textures[tid]->setProgram();
	}
	for(nid=0;nid<noise.size;nid++){
		noise[nid]->setProgram();
	}
	if(tnpoint)
		tnpoint->setProgram();
}
void TerrainProperties::reset(){
	noise.reset();
	textures.reset();
}

