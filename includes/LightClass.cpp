// LightClass.cpp

#include "LightClass.h"
#include "ObjectClass.h"
#include "GLheaders.h"
#include "NoiseClass.h"
#include <math.h>

int maxlights=8;

Light::Light()
{
	position=Point(0.0,0.0,0.0);
	id=-1;
	diffuse=1;
	specular=0.2;
	enabled=false;
	color=Specular=Diffuse=WHITE;
}

void Light::mixDiffuse(Color m)
{
	FColor a=Diffuse*diffuse;
	FColor b=m*m.alpha();
	FColor c=a*b*diffuse;
	c.set_alpha(1.0);
	glLightfv(id,GL_DIFFUSE,c.values());
}

void Light::modDiffuse(Color m)
{
	glLightfv(id,GL_DIFFUSE,dcolor.values());
}

void Light::modDiffuse(double m)
{
	FColor b=dcolor*m;
	glLightfv(id,GL_DIFFUSE,b.values());
}

void Light::mixSpecular(Color m)
{
	FColor a=Specular*specular;
	FColor b=m*m.alpha();
	FColor c=a*b;
	c.set_alpha(1.0);
	glLightfv(id,GL_SPECULAR,c.values());
}
void Light::modSpecular(Color m)
{
	m=m*m.alpha();
	FColor b=color*m;
	FColor a=color;
	scolor=a+b;
	scolor=scolor*Lights.albedo;
	glLightfv(id,GL_SPECULAR,scolor.values());
}

void Light::modSpecular(double m)
{
	FColor b=scolor*m;
	glLightfv(id,GL_SPECULAR,b.values());
}

void Light::setIntensity(double a)
{
	intensity=a;
}

void Light::setAttenuation(Point p)
{
    double a=getAttenuation(p);
	glLightf(id,GL_CONSTANT_ATTENUATION,(float)a);
}

double Light::getIntensity(Point p)
{
	return 1/getAttenuation(p);
}

double Light::getAttenuation(Point p)
{
    double d=p.distance(point);// distance to star

   // Theoretical
   // - light intensity ~1/d^2
   // - eye light perception ~sqrt(intensity) so perceived intensity ~1/d
   // - intensity based on star temperature (1000-30000 K)
    double brightness=0.01*intensity*pow(d,-0.65); // ad-hoc increase brightness with distance (should be 1 ?)
    brightness=brightness>1?sqrt(brightness):brightness; // HVS saturation ?
    //cout << brightness<<endl;

	return 1/brightness;
}

void Light::setDiffuse(double d)
{
	dcolor=Diffuse*d*diffuse;
	dcolor.set_alpha(d*diffuse);
	glLightfv(id,GL_DIFFUSE,dcolor.values());
}

void Light::setSpecular(double s)
{
	scolor=Specular*specular;//*s;
	scolor.set_alpha(1.0);
	glLightfv(id,GL_SPECULAR,scolor.values());
}

void Light::defaultLighting()
{
	setDiffuse(1.0);
	setSpecular(1.0);
}

void Light::setPosition(Point p)
{
	float pos[4];
	pos[0]=(float)p.x;
	pos[1]=(float)p.y;
	pos[2]=(float)p.z;
	pos[3]=(float)enabled;
	glLightfv(id,GL_POSITION,pos);
}

void Light::enable()
{
	glEnable(id);
	enabled=true;
}

void Light::disable()
{
	glDisable(id);
	enabled=false;
}

bool Light::isEnabled(){
	return enabled;
}
// LightMgr

LightMgr Lights;  // one-and-only object

LightMgr::LightMgr()
{
	glGetIntegerv(GL_MAX_LIGHTS,&maxlights);
	MALLOC(maxlights,Light*,lights);
	albedo=1;
	shine=10;
	size=0;
}

LightMgr::~LightMgr()
{
	free();
	FREE(lights);
}

void LightMgr::enable()
{
	disable();
	for(int i=0;i<size;i++)
		lights[i]->enable();
}

void LightMgr::disable()
{
	for(int i=0;i<size;i++)
		lights[i]->disable();
}
int LightMgr::numLights(){
	int active=0;
	for(int i=0;i<Lights.size;i++){
		if(lights[i]->enabled)
			active++;
	}
	return active;
}
void LightMgr::free()
{
	//disable();
	for (int i=0;i<size;i++)
		delete lights[i];
	size=0;
}

Light *LightMgr::newLight()
{
	if(size>=maxlights)
		return 0;
	Light *l=new Light();
	l->id=GL_LIGHT0+size;
	lights[size++]=l;
	return l;
}

void LightMgr::setShininess(double d)
{
    shine=d;
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, (float)d);
}

void LightMgr::setEmission(Color c)
{
 	float emission[4];
 	double alpha=c.alpha();
 	c=c*c.alpha();
 	c.floatArray(emission);
 	emission[3]=(float)alpha;

    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void LightMgr::setAmbient(Color c)
{
 	float ambient[4];
 	double alpha=c.alpha();
 	c=c*alpha;
 	c.floatArray(ambient);
 	ambient[3]=(float)alpha;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
}

void LightMgr::setDiffuse(Color c)
{
 	float diffuse[4];
  	c.floatArray(diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
}

void LightMgr::defaultLighting()
{
	for(int i=0;i<size;i++)
		lights[i]->defaultLighting();
	setShininess(0);
	setEmission(Color(1,1,1,0));
	setAmbient(Color(1,1,1,0));
	//setDiffuse(Color(1,1,1,1));
}

void LightMgr::setSpecular(double d)
{
    albedo=d;
	for(int i=0;i<size;i++)
		lights[i]->setSpecular(d);
}

void LightMgr::modSpecular(Color c)
{
    if(c.alpha())
	for(int i=0;i<size;i++)
		lights[i]->modSpecular(c);
}

void LightMgr::mixSpecular(Color c)
{
	for(int i=0;i<size;i++)
		lights[i]->mixSpecular(c);
}

void LightMgr::setDiffuse(double d)
{
	for(int i=0;i<size;i++)
		lights[i]->setDiffuse(d);
}

void LightMgr::modDiffuse(Color c)
{
    if(c.alpha())
	for(int i=0;i<size;i++)
		lights[i]->modDiffuse(c);
}

void LightMgr::mixDiffuse(Color c)
{
	for(int i=0;i<size;i++)
		lights[i]->mixDiffuse(c);
}

void LightMgr::setAttenuation(Point p)
{
	for(int i=0;i<size;i++)
		lights[i]->setAttenuation(p);
}

double LightMgr::intensityFraction(Light *l, Point p)
{
    double sum=0;
	for(int i=0;i<size;i++)
		if(Lights[i]->isEnabled())
			sum+=Lights[i]->getIntensity(p);
	return l->getIntensity(p)/sum;
}
