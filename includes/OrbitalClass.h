
//  OrbitalClass.h

#ifndef _ID_ORBITALCLASS
#define _ID_ORBITALCLASS

#include "LightClass.h"
#include "ObjectClass.h"
#include "TerrainMgr.h"
#include "States.h"

class ExprMgr;
class TNode;
class StarTree;
class DensityTree;
class Map;
class MapData;
class TNinode;
class TNode;
class TNtexture;
class TNclist;
class TNclouds;
class Sky;
class Ring;
class CloudLayer;

class Triangle;

enum orbital_codes{
	ID_SPHEROID 	= ID_ORBITAL|0x0100,
	ID_PLANETOID   	= ID_ORBITAL|0x0300,
	ID_SHELL    	= ID_ORBITAL|0x0500,
	ID_UNIVERSE  	= ID_ORBITAL|0x0003,
	ID_GALAXY  		= ID_ORBITAL|0x0004,
	ID_NEBULA  		= ID_ORBITAL|0x0005,
	ID_SYSTEM  		= ID_ORBITAL|0x0006,
	ID_STAR     	= ID_LEVEL1|ID_ORBITAL|0x0107,
	ID_CORONA  		= ID_LEVEL1|ID_ORBITAL|0x0508,
	ID_FLARES  		= ID_LEVEL1|ID_ORBITAL|0x0509,
	ID_HALO  		= ID_LEVEL1|ID_ORBITAL|0x050a,
	ID_PLANET   	= ID_LEVEL2|ID_ORBITAL|0x0309,
	ID_CLOUDS  		= ID_LEVEL2|ID_ORBITAL|0x050c,
	ID_SKY      	= ID_LEVEL2|ID_ORBITAL|0x050b,
	ID_RING	 		= ID_LEVEL4|ID_ORBITAL|0x000e,
	ID_MOON     	= ID_LEVEL5|ID_ORBITAL|0x030a,
};



//************************************************************
// Orbital base class
//************************************************************
class Orbital : public ObjectNode
{
protected:
	enum {
		maxstr=64
	};
	void set_defaults();
	void set_wscale();
	Color      _color;
    char name_str[maxstr];
    static LinkedList<double>seedlist;
	double rseed;

public:

	ExprMgr exprs;
	double rot_angle;
	double orbit_angle;    // time dependent rotation angle
	double orbit_distance; // time dependent rotation radius
	double rot_phase;
	double orbit_phase;
	double year;
	double day;
	double tilt;
	double pitch;
	double orbit_skew;
	double orbit_radius;
	double orbit_eccentricity;
	double detail;
	Color  emission;
	Color  ambient;
	Color  diffuse;
	Color  specular;
	double albedo;
	double shine;
	double sunset;
	double shadow_intensity;

	void pushSeed();
	void popSeed();
	void setSeed(double);

	Point  origin;

	Orbital(double s);
	Orbital(Orbital *m);
	Orbital(Orbital *m, double s);
	Orbital(Orbital *m, double s, double r);
	virtual ~Orbital();

	virtual void setName(char *c);
	virtual char *getName();
	virtual void getDateString(char *);
	virtual void getTempString(char *);
	virtual double getTemperature();
    virtual bool randomize();
    virtual bool canRandomize() { return true;}
    virtual void setDefault();

	virtual void locate();
	virtual void set_tilt();
	virtual void orient();
	virtual void setMatrix();

	virtual void set_rotation();
	virtual void set_ref();
	virtual void adapt();
    virtual void render();
    virtual void select();
    virtual void set_lights();
    virtual void set_lighting();
	virtual void init_view();
	virtual void set_view();
	virtual void init();
	virtual void animate();
	virtual void save(FILE*);
	virtual Bounds *bounds();
	virtual int type()		{ return ID_ORBITAL;}
	virtual TNode *add_expr(int t, char *s, TNode *r);
	virtual TNode    *add_image(TNode *r);
	virtual TNinode  *get_image(char *r, int m);
	virtual TNode *set_terrain(TNode *n);
	virtual void set_surface(TerrainData&);
	virtual void set_debug(int d);
	virtual void set_mode(int);
	virtual void set_color(Color);
	virtual Color color();
	virtual void free();
	virtual void visit(void  (Object3D::*func)());
	virtual void visit(void  (*f)(Object3D*));
	virtual void get_vars();
	virtual void set_vars();
	virtual int inside_sky() { return 0;}
	virtual Sky *get_sky() { return 0;}
	double resolution()	{ return detail;}
    void setOrbitFrom(Orbital *prev);
    virtual bool containsViewobj();
    virtual bool isViewobj();
	virtual void setRseed(double s) {rseed=s;}
	virtual double getRseed() {return rseed;}
	TNvar *addExprVar(const char *,const char *);

	// NodeIF methods

	virtual const char *typeName();
	virtual char *nodeName();
	int nodeClass()	{ return ID_ORBITAL;}
	virtual int getChildren(LinkedList<NodeIF*>&l);
	virtual NodeIF *addChild(NodeIF *);
	virtual NodeIF *addAfter(NodeIF *,NodeIF *);
	virtual NodeIF *removeChild(NodeIF *);
	virtual NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	virtual NodeIF *getInstance();
};

//************************************************************
// Universe class
//************************************************************
class Universe : public Orbital
{
public:
	Universe(double s);
	const char *name()				{ return "Universe";}
	int  type()						{ return ID_UNIVERSE;}
	int scale(double&n, double&f)   { return OFFSCREEN;}
	NodeIF *addChild(NodeIF *);
	NodeIF *addAfter(NodeIF *,NodeIF *);
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);

	void save(FILE *);
};

//************************************************************
// DensityCloud class
//************************************************************
class DensityCloud : public Orbital
{
protected:
	virtual void set_ref();
	virtual void animate();
public:
	TNode *noise_expr;
	TNode *color_expr;
	TNclist *color_list;
	TNtexture *texture;

	DensityTree *tree;
	DensityCloud(Orbital *m, double s);
	DensityCloud(double s);
	virtual ~DensityCloud();
	virtual DensityTree *defaults() { return 0;}
	virtual void setMatrix();
	virtual void render();
	virtual void adapt();
	virtual void save(FILE *);
	virtual void set_vars();
	virtual void get_vars();
	virtual void render_object();
	virtual void adapt_object();
	virtual int scale(double&, double&);
	virtual double radius()			{ return 0;}
	virtual void setDefaultNoise();
	virtual int getNoiseFunction(char *c);
	virtual void setNoiseFunction(char *c);
	virtual void applyNoiseFunction();
	virtual int getColorFunction(char *c);
	virtual void setColorFunction(char *c);
	virtual void applyColorFunction();
	virtual void setColor(int i,Color c);
	virtual Color getColor(int i);

	void set_surface(TerrainData &d);

	int cells();
};

//************************************************************
// Nebula class
//************************************************************
class Nebula : public DensityCloud
{
protected:
    static DensityTree _defaults;
public:
	Nebula(Orbital *m, double s);
	Nebula(double s);
	~Nebula();
	const char *name()				{ return "Nebula";}
	int  type()						{ return ID_NEBULA;}
	DensityTree *defaults() 		{ return &_defaults;}
	void  init();
	void render();
	void adapt();
};

//************************************************************
// Galaxy class
//************************************************************
class Galaxy : public DensityCloud
{
protected:
    static StarTree _defaults;
	static GLuint star_image[3];
	void setStarTexture(int id,char *name);
public:
	Galaxy(Orbital *m, double s);
	Galaxy(double s);
	~Galaxy();
	const char *name()				{ return "Galaxy";}
	int  type()						{ return ID_GALAXY;}
	DensityTree *defaults() 		{ return (DensityTree*)&_defaults;}
	void set_ref();
	void init_view();
	void render();
	void render_object();

	void adapt();
	void rebuild();
    void select();
	int selection_pass();
	int render_pass();
	int adapt_pass();
	void set_focus(Point &p);
	void move_focus(Point &p);
	Point get_focus(void *);
	bool setProgram();
	void set_vars();
	void get_vars();
	void addNewSystem();
};

//************************************************************
// System class
//************************************************************
class System : public Orbital
{
private:
    void get_system_stats();
public:
 	int m_planets;
	int m_stars;
   void set_system(Point p);
    static Point galaxy_origin;
    static bool building_system;
    static System *TheSystem;

	System(Orbital *m, double s);
	System(double s);
	int  type()				{ return ID_SYSTEM;}
	const char *name()		{ return "System";}
	double radius()			{ return 0;}
	void save(FILE *);
	void set_vars();
	void get_vars();
	void init_view();
	void render();
	void adapt();
	void set_ref();
	void locate();
	int scale(double&n, double&f);
	int select_pass();
	int stars();
    int planets();
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	NodeIF *replaceNode(NodeIF *n);
	static System *newInstance();
	NodeIF *getInstance();
	bool randomize();
	double adjustOrbits();
	NodeIF *addChild(NodeIF *);
	NodeIF *addAfter(NodeIF *,NodeIF *);
	NodeIF *removeChild(NodeIF *);
};

//************************************************************
// Spheroid class
//************************************************************
class Spheroid : public Orbital
{
protected:
	int 	   first;
public:
	Color 	   shadow_color;
	double     symmetry;
	double     hscale;
	Map        *map;
	TerrainMgr terrain; // exprs evaluated for each map node

	Spheroid(Orbital *m, double s, double r);
	Spheroid(Orbital *m, double s);

	~Spheroid();

	virtual bool force_adapt();
 	virtual void set_tilt();
	virtual void set_rotation();
    virtual void render();
    virtual void render_object();
    virtual void select();
	virtual void save(FILE *);

	virtual void locate();
	virtual void init();
	virtual void init_view();
	virtual void adapt();
    virtual void adapt_object();
	virtual Bounds *bounds();
	virtual int  type()		{ return ID_SPHEROID;}
	virtual void set_scene();
    virtual void map_color(MapData*, Color&);
	virtual void rebuild();
	virtual void set_geometry();
	virtual void set_vars();
	virtual void get_vars();
	virtual int adapt_pass();
	virtual int render_pass();
	virtual int selection_pass();
	virtual int scale(double&n, double&f);
	virtual double height(double t, double p);
	virtual void set_focus(Point &p);
	virtual void move_focus(Point &p);
	virtual Point get_focus(void *);
	virtual int inside_sky();
	virtual Sky *get_sky();

	virtual double far_height();
	virtual double max_height();
	virtual NodeIF *replaceChild(NodeIF *c,NodeIF *n);
	NodeIF *replaceNode(NodeIF *n);

	TNode *add_expr(int t, char *c, TNode *r);
	TNode *set_terrain(TNode *n);
	void set_surface(TerrainData &d) 	{ terrain.set_surface(d);}
	void set_debug(int d)				{ terrain.set_debug(d);}
	int cells();
	int cycles();
	bool hasChildren();
	int getChildren(LinkedList<NodeIF*>&l);
	Map *getMap()                       { return map;}
	
	void addTerrainVar(const char *,const char *);

};

//************************************************************
// Star class
//************************************************************
class Star : public Spheroid
{
public:
	static const int ntypes=7;
	static Color star_color[ntypes];
	static double star_temp[ntypes];
	static char star_class[ntypes];
	static double star_frequency[ntypes];
	static double star_luminocity[ntypes];
	static double star_radius[ntypes];
    static int star_id;
	
	static double *probability;
	static double *temps;
	static int num_temps;
	static int expand_factor;

	static void star_info(Color c, double *t, char *m);
	static double star_size(double t);
	double max_height();
	static void make_temps_table();
	double temperature;
	char startype[3];

	Star(Orbital *m, double s, double r);
	~Star();

	void map_color(MapData*, Color&);
	void set_lights();
	const char *name()			{ return "Star";}
	int  type()					{ return ID_STAR;}
	void set_vars();
	void get_vars();
	void set_lighting();
	void render_object();
	bool setProgram();
	int render_pass();
	int adapt_pass();
    void render();
    void setRadiance(Color c);
    void getStarData(double *d, char *m);
    NodeIF *getInstance();
    static Star *newInstance();
    static void random(double &temp, double &r, Color &color);
    static TNinode *image(Color color);
    static TNtexture *texture();
};

//************************************************************
// Shell class
//************************************************************
class Shell : public Spheroid
{
public:
	Shell(Orbital *m);
	Shell(Orbital *m, double s);
	~Shell();
	const char *name()			{ return "Shell";}
	int  type()					{ return ID_SHELL;}
	double ht;
	//virtual void init();
    virtual void render_object();
    virtual void adapt_object();
	virtual void set_geometry();
	virtual int scale(double&n, double&f);
    virtual void select();
	virtual int selection_pass();
	virtual void dpvars(double &dht, double &dpmin, double &dpmax);
	virtual double max_height() { return 0;}
	bool isLeaf()						{ return true;}
	NodeIF *replaceNode(NodeIF *n);
};

//************************************************************
// Sky class
//************************************************************
class Sky : public Shell
{
public:
	Color 	   twilite_color;
	Color 	   halo_color;
	Color 	   night_color;
	double     twilite_value;
	double     twilite_max;
	double     twilite_min;
	double     twilite_dph;
	double     sky_grad;
	double     density;
	Color 	   haze_color;
	double     haze_value;
	double     haze_min;
	double     haze_max;
	double     haze_grad;
	double     haze_height;
	double     haze_zfar;
	double     pressure;
	double     ghg_fraction;

	Sky(Orbital *m, double s);
	~Sky();
	const char *name()			{ return "Sky";}
	int  type()					{ return ID_SKY;}
	void init_render();
	void map_color(MapData*,Color&);
	void render_object();
	void adapt_object();
	double   getHeatCalacity();

	void set_vars();
	void get_vars();
	void set_ref();
	int render_pass();
	int adapt_pass();
	bool setProgram();
	int scale(double&n, double&f);
	bool adapt_needed() { return false;}
	NodeIF *getInstance();
	static Sky *newInstance();
};


//************************************************************
// Halo class
//************************************************************
class Halo : public Shell
{
public:
	Color 	   color1;
	Color 	   color2;
	double     gradient;
	double     density;
	Halo(Orbital *m, double s);
	~Halo();
	const char *name()			{ return "Halo";}
	int  type()					{ return ID_HALO;}
	void set_vars();
	void get_vars();
	void render_object();
	void orient();
	void setMatrix();
	void map_color(MapData*n,Color &col);
	int render_pass();
	bool setProgram();
};
//************************************************************
// Corona class
//************************************************************
class Corona : public Shell
{
public:
	double     gradient;
	TNode *noise_expr;
	TNdensity *density_expr;
	Color      color1;
	Color      color2;
	bool       animation;
	bool       internal;
	double     rate;
	Corona(Orbital *m, double s);
	~Corona();
	const char *name()			{ return "Corona";}
	int  type()					{ return ID_CORONA;}
	void map_color(MapData*,Color&);
	void set_vars();
	void get_vars();
	void render_object();
	void orient();
	void setMatrix();
	int render_pass();
	int adapt_pass();
	bool setProgram();
	int getNoiseFunction(char *c);
	void setNoiseFunction(char *c);
	void applyNoiseFunction();
	bool adapt_needed() { return false;}
	void set_surface(TerrainData &d);
	virtual NodeIF *getInstance();
};

//************************************************************
// CloudLayer class
//************************************************************
class CloudLayer : public Shell
{
protected:
	GLuint sprites_image;
	char sprites_file[256];
	GLuint sprites_dim;
	TNclouds *clouds;

public:
	static char *dflt_sprites_file;
	double     cmin;
	double     cmax;
	double     smax;
	double     crot;
	double     num_sprites;
	double     diffusion;
	bool   	   animation;
	double     rate;
	CloudLayer(Orbital *m, double s);
	~CloudLayer();
	const char *name()			{ return "Clouds";}
	int  type()					{ return ID_CLOUDS;}
	void map_color(MapData*,Color&);
	void render();
	void render_object();
	void adapt_object();
	void set_vars();
	void get_vars();
	void set_ref();
	void set_tilt();

	int render_pass();
	int shadow_pass();
	int adapt_pass();
	void animate();
	bool setProgram();
	int scale(double&n, double&f);
	bool threeD();
	TNclouds *getClouds() { return clouds;}
	void setClouds(TNclouds *c);
	void deleteClouds();
	void init();
	bool force_adapt();
	void setSpritesTexture();
	char *getSpritesFile()       { return sprites_file;}
	void setSpritesFile(char *s) { strcpy(sprites_file,s);}
	void setSpritesDim(GLuint d) { sprites_dim=d;}
	GLuint getSpritesDim()       { return sprites_dim;}
	void setSpritesFile(char *s,GLuint dim);
	char *getSpritesFile(GLuint &dim);
	void getSpritesFilePath(char *);
	void getSpritesDir(int dim,char *);
    NodeIF *getInstance(NodeIF *prev);
    
	static CloudLayer *newInstance(bool is3d);

};

//************************************************************
// Planet class
//************************************************************
class Planetoid : public Spheroid
{
protected:

public:
	enum {GASGIANT,OCEANIC,ROCKY,ICY,VOLCANIC};
	
	static std::string randFeature(int type);
    //OceanState oceanState;
    OceanState *ocean;
	enum {GAS=0,LIQUID=1,SOLID=2};

	int        terrain_type;
	double 	   ocean_level;
	int        ocean_state;
	bool       ocean_auto;
	bool       seasonal;
	bool       have_water;

	Color 	   fog_color;
	double     fog_min;
	double     fog_max;
	double     fog_vmin;
	double     fog_vmax;
	double     fog_value;
	double     fog_glow;
	double     temperature;
	double 	   surface_temp;
	double 	   season_factor;
	double     heat_factor;
	double 	   temp_factor;
	double 	   last_dt;
	double 	   last_temp;
	double     light_theta;

	static int planet_id;
	static int moon_cnt;
	static int planet_cnt;
	static int layer_cnt;
	static int num_layers;

	static int tcount;

	static double planet_orbit;
	
    static const int THEMES = 7;
	static Color themes[THEMES * 4];
	static const int COLORS = 6;
	static Color colors[COLORS];
	static Color mix;
	static int ncolors;
	static Color tc;

	void 	   init_render();
	Planetoid(Orbital *m, double s, double r);
	~Planetoid();
	const char *name()			{ return "Planetoid";}
	virtual int  type()			{ return ID_PLANET;}
	virtual void setDateString();
	virtual void getDateString(char *);
	virtual void getTempString(char *);
    virtual void setOcean(OceanState *s);
    virtual OceanState *getOcean();
	virtual void set_lighting();
	virtual void set_tod();
    virtual void map_color(MapData*,Color&);
	virtual void set_vars();
	virtual void get_vars();
	virtual void set_ocean_vars();
	virtual void get_ocean_vars();
	virtual void render();
	virtual int adapt_pass();
	virtual int render_pass();
	virtual int shadow_pass();
    virtual void adapt_object();
    virtual void render_object();
	virtual bool setProgram();
	virtual double calc_delt();
	virtual double dlt();
	virtual double calc_time(double t);
	virtual void set_time(double t);
	virtual void calcAveTemperature();
	virtual double calcLocalTemperature(bool w);
	virtual double tilt_bias();
	virtual double season_bias();
	virtual void animate();
	virtual bool liquid();
	virtual bool solid();
	virtual bool gas();
	virtual bool water();
	virtual void checkForOcean();
	virtual bool tidalLocked();
	virtual void setTidalLocked();
	virtual double solidToLiquid();
	virtual double liquidToGas();
    virtual void set_view_info();
	virtual void set_surface(TerrainData&);
	static void newRocky(Planetoid *);
	static std::string newLocalTex(Planetoid *);
	static std::string newGlobalTex(Planetoid *);
	static std::string newDualGlobalTex(Planetoid *);
	static std::string newOcean(Planetoid *);
	static std::string newLayer(Planetoid *);

	static std::string newHmapTex(Planetoid *);
    static void pushInstance(Planetoid *);
    static void popInstance(Planetoid *);
	void setColors();
	void initInstance();
	
	void setIceColors();

	virtual Sky *newSky();
	virtual Ring *newRing();
	virtual CloudLayer *newClouds(bool is3d);

};

//************************************************************
// Planet class
//************************************************************
class Planet : public Planetoid
{
public:
	Planet(Orbital *m, double s, double r);
	~Planet();

	const char *name()			{ return "Planet";}
	int  type()					{ return ID_PLANET;}
	NodeIF *getInstance(NodeIF *prev);
	void newInstance();
	static void newGasGiant(Planet *);
	static void newRocky(Planet *);
	void addMoon();
};

//************************************************************
// Moon class
//************************************************************
class Moon : public Planetoid
{
public:
	Moon(Planet *m, double s, double r);
	~Moon();

	const char *name()			{ return "Moon";}
	int  type()					{ return ID_MOON;}
	void newInstance();
	void set_tilt();
	NodeIF *getInstance(NodeIF *prev);
};

//************************************************************
// Ring class
//************************************************************
class Ring : public Shell
{
public:
	static int ring_id;
	double width;
	double inner_radius;
	Ring(Planet *m, double s, double r);
	~Ring();

	void set_tilt();
	const char *name()			{ return "Ring";}
	int  type()					{ return ID_RING;}
    void render_object();
	void save(FILE *);
	int scale(double&n, double&f);
    void map_color(MapData*,Color&);
	void rebuild();
	void set_geometry();
	int render_pass();
	int shadow_pass();
    void select();
	bool setProgram();
    NodeIF *getInstance();
    static Ring *newInstance();
};


#endif
