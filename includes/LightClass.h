
//  LightClass.h

#ifndef _LIGHTCLASS
#define _LIGHTCLASS

#include "ColorClass.h"
#include "PointClass.h"
#include "ListClass.h"

class Light
{
  protected:
	void applyDiffuse(double r);
	void applySpecular(double r);
  public:
	void enable();
	void disable();
	bool isEnabled();
	void setDiffuse(double r);
	void setSpecular(double r);
	void setAttenuation(Point p);
	double getAttenuation(Point p);
	double getIntensity(Point p);
	void setIntensity(double a);
	void modDiffuse(double r);
	void modSpecular(double r);
	void modDiffuse(Color c);
	void modSpecular(Color c);
	void mixDiffuse(Color c);
	void mixSpecular(Color c);
	void defaultLighting();
	void setColor(Color c);
	void modColor(Color c, double a);
	void setColor();
	void setPosition(Point p);
	Light();

	FColor scolor;
	FColor acolor;
	FColor dcolor;

	Color  Specular;
	Color  Diffuse;

	Color  color;
	Color  lcolor;
	double intensity,diffuse,specular;
	double size;
	Point point;
	Point position;
	int		id;
	bool enabled;

	friend class LightMgr;
};

class LightMgr
{
	Light **lights;
public:
	double shine;
	double albedo;
	double emmision;
	int size;
	LightMgr();
	~LightMgr();
	void enable();
	void disable();
	void free();
	void reset();
	void defaultLighting();
	void setDiffuse(double r);
	void setSpecular(double r);
	void mixDiffuse(Color c);
	void mixSpecular(Color c);
	void modDiffuse(Color c);
	void modSpecular(Color c);
	void modDiffuse(double c);
	void modSpecular(double c);
	void setIntensity(double a);
	void setAttenuation(Point p);
	double intensityFraction(Light *,Point p);
	
	void setShininess(double d);
	void setEmission(Color c);
	void setAmbient(Color c);
	void setDiffuse(Color c);
			
	Light *newLight();
	Light *first()	{ return lights[0];}
	Light *last()	{ return lights[size-1];}
	Light *operator[](int i) {return lights[i];}
	int numLights();

};
extern LightMgr Lights;
#endif
