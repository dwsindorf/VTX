
//  SceneClass.h

#ifndef _SCENECLASS
#define _SCENECLASS

#include "LightClass.h"
#include "ObjectClass.h"
#include "OrbitalClass.h"
#include "TerrainMgr.h"
#include "Renderer.h"
#include "NodeIF.h"
#include "ModelClass.h"
#include <time.h>

class Model;
class TreeNode;
class FrameMgr;
class ViewFrame;

extern class Scene *TheScene;
extern void	add_initializer(VFV func);
extern void	add_finisher(VFV func);

class NameSym {
public:
	char   _name[256];
	NameSym(char *n) {
		strcpy(_name,n);
	}
	char *name() { return _name;}
};
enum{
	INFO_ELEVATION,
	INFO_POSITION,
	INFO_TIME,
	INFO_CELLSIZE,
	INFO_VSTRIDE,
	INFO_HSTRIDE
};

enum{
	ADAPT,
	RENDER,
	BUFFERS,
	SHADOWS,
	SELECT_OBJ,
	SELECT_NODE
};

enum {
	RECORD    = 0x00000001,
	PLAY      = 0x00000002,
	JSAVE     = 0x00000004,
	JPLAY     = 0x00000008,
	MMODE     = RECORD|PLAY|JSAVE|JPLAY,
	INTRP     = 0x00000010,
	MOPEN     = 0x00000020,
	HELP      = 0x00000040,
    DFLT      = 0x00000000,
    VIEWS     = 0x00000080,
	MOVIE     = 0x00000100,
	NCOLS     = 0x00000200,
	INFO      = 0x00000400,
	TEST      = 0x00000800,
	VLOG      = 0x00001000,
	LOG       = 0x00002000,
	VIDEO     = 0x00004000,
	PATH      = 0x00008000,
	REVERSE   = 0x00010000,
	VMODE     = LOG|PATH|VIDEO,
	FUNCTION  = MOVIE|VIEWS|NCOLS|INFO|TEST
};

#define BEGIN_TIMING double start,duration;start=clock();
#define END_TIMING (duration=(double)(clock() - start) / CLOCKS_PER_SEC);

class Scene : public Renderer, public NodeIF
{
protected:
	int         selx;
	int         sely;
	int         self;
	int			rmode;
	int         vmode;
	int 		nsobs;
	int 		vsobs;
	ExprMgr     exprs;
	ExprMgr     vars;
	LinkedList<ObjectNode*>objpath;

	void		save(FILE *);
	void 		render_ids_();
	void 		show_select_info();
	void 		show_mode_info();
	void		show_log_info();
	void 		shadow_group(int);
	void 		pass_group();
	void 		pass_reset();
	void 		pass_init();
	void 		set_obj_path();
	void 		locate_objs();
	void 		save_frame();
	int			make_movie_dir(char *,char*);
	int			get_movie_frames();


public:
	Point       selm;
	NameList<NameSym*>frame_files;
	char        filename[256];
	char        moviename[256];
	ObjectNode *selobj;
	ObjectNode *viewobj;
	ObjectNode *shadowobj;
	ObjectNode *localobj;
	ObjectNode *groupobj;
	ObjectNode *focusobj;
	ObjectNode *rootobj;
	Model      *model;

	View       *view;
	FrameMgr   *views;
	FrameMgr   *movie;
	int         inter_frames;
	int         inter_index;
	int         frame_index;
	double      animate_rate;
	double      playback_rate;
	double      refresh_rate;
	double      redraw_rate;
	double      init_time;
	double		render_time;
	double		adapt_time;
	double		build_time;
	int    	cells;		// total # cells in scene
	int    	cycles;		// total adapt cycles
	Scene(Model *m);
	~Scene();
	void set_focus_object(int i){ self=i;set_changed_render();}

	bool inside_sky();

	void		set_view();
	void set_movie(char *s);
	char *movie_name()			{ return moviename;}

	void set_file(char *s)      { strcpy(filename,s);}
	char *file_name()			{ return filename;}

	void set_showlog(int i)		{ BIT_SET(vmode,VLOG,i);}
	int showlog()               { return vmode & VLOG;}
	void set_help(int i)		{ BIT_SET(vmode,HELP,i);}
	int help()                  { return vmode & HELP;}
	void set_mopen(int i)		{ BIT_SET(vmode,MOPEN,i);}
	int mopen()                 { return vmode & MOPEN;}
	void set_intrp(int i)		{ BIT_SET(vmode,INTRP,i);}
	int intrp()                 { return vmode & INTRP;}

	void set_video()			{ MSK_SET(vmode,VMODE,VIDEO);}
	void set_log()			    { MSK_SET(vmode,VMODE,LOG);}
	void set_path()			    { MSK_SET(vmode,VMODE,PATH);}

	int view_mode()             { return vmode&VMODE;}

	void set_record()			{ MSK_SET(vmode,(RECORD|PLAY),RECORD);}
	void set_play()			    { MSK_SET(vmode,(RECORD|PLAY),PLAY);}
	int playing()				{ return vmode&PLAY;}
	int recording()				{ return vmode&RECORD;}
	void set_jsave(int i)		{ BIT_SET(vmode,JSAVE,i);}
	int jsave()					{ return vmode&JSAVE;}
	void set_jplay(int i)		{ BIT_SET(vmode,JPLAY,i);}
	int jplay()					{ return vmode&JPLAY;}
	void stop()					{ MSK_SET(vmode,MMODE,0);}
	void pause()				{ BIT_OFF(vmode,(RECORD|PLAY));}
	int reverse()				{ return (vmode&REVERSE)?1:0;}
	void set_reverse(int i)		{ BIT_SET(vmode,REVERSE,i);}
	void set_movie_mode()		{ BIT_ON(vmode,MOVIE);set_function(MOVIE);}
	void set_views_mode()		{ BIT_OFF(vmode,MOVIE);set_function(VIEWS);}
	int movie_mode()			{ return (vmode&MOVIE)?1:0;}
	int views_mode()			{ return (vmode&MOVIE)?0:1;}
	int function()              { return vmode & FUNCTION;}
	void set_function(int i)    { BIT_OFF(vmode,FUNCTION);BIT_ON(vmode,i);}

	void record_movie_from_path();


	void jpeg_save(char *);
	void jpeg_show(char *);

	void animate();
	void free();
	bool setProgram();
	// render mode functions

	void rebuild_all();

	void set_select_object()	{ rmode=SELECT_OBJ;}
	void set_select_node()		{ rmode=SELECT_NODE;}
	void set_render_mode()		{ rmode=RENDER;}
	void set_adapt_mode()		{ rmode=ADAPT;}
	void set_shadows_mode()		{ rmode=SHADOWS;}
	void set_buffers_mode()		{ rmode=BUFFERS;}
	int  select_object()		{ return rmode==SELECT_OBJ ? 1 :0;}
	int  select_node()		    { return rmode==SELECT_NODE ? 1 :0;}
	int  select_mode()		    { return (rmode==SELECT_NODE||rmode==SELECT_OBJ) ? 1 :0;}
	int  adapt_mode()			{ return rmode==ADAPT ? 1 :0;}
	int  render_mode()			{ return rmode==RENDER ? 1 :0;}
	int  shadows_mode()			{ return rmode==SHADOWS ? 1 :0;}
	int  buffers_mode()			{ return rmode==BUFFERS ? 1 :0;}

	void init();
	void reset();
	void make();
	void make(Model *);
	void select(int x, int y);
	void select();
    void *select_pass();
	void wrect(int,int,int,int);
	void reset_view();

	// Model methods

	TreeNode *buildTree();
	void freeTree(TreeNode *t);
	void printTree(TreeNode *t,char *s);
    int  treeNodes(TreeNode *t);
	int  getPrototype(NodeIF*, int,char *);
	int  setPrototype(NodeIF*, NodeIF*);
	void change_view(int);
	void change_view(int,double);
	void change_view(int,Point);
	void setview_test();
	void render_test_view();
	void render_light_view();
	void init_view();
	void set_surface_view();
	void set_orbital_view();
	void set_global_view();
	void set_matrix(Object3D *);

	void render();
	void render_objects();
	void render_bgpass();
	void render_shadows();
	void render_zvals();
	void render_normals();
	void render_raster();
	void render_image();
	void set_lights();
	void adapt();

	void save();
	void save(char *);
	void show();
	void open();
	void open(char *);
	void show_status(int);
	void show_info(int);

	double get_expr(char *s);
	void add_expr(char *s,double v);
	void remove_expr(char *s);
	void add_expr(char *s, TNode *r);
	void view_expr(char *s, TNode *r);
	TNsubr *subr_expr(char *s,TNode *r);
	void add_orbital(Orbital *);
	void orbital_node(Orbital *);
	void terrain_node(TNode *);
	NodeIF *parse_node(char *);
	NodeIF *parse_node(NodeIF *,char *);
	NodeIF *open_node(NodeIF *,char *);
	void save_node(NodeIF *obj,char *path);
	Object3D *findObject(char *name);

	bool hasChildren();
	bool isLeaf()							{ return false;}
	int getChildren(LinkedList<NodeIF*>&l);
	NodeIF *addChild(NodeIF *c);
	NodeIF *addAfter(NodeIF *,NodeIF *);
	NodeIF *removeChild(NodeIF *c);
	NodeIF *replaceChild(NodeIF *c,NodeIF *n);

	int typeValue()							{ return ID_SCENE;}
	const char *typeName()					{ return "Scene";}
	void valueString(char *s)				{ strcpy(s,typeName());}
	void printNode(char *s)					{ strcpy(s,typeName());}

	int surface_view();
	int orbital_view();
	void set_viewtype();
	void set_quality(int q);
	void regroup();

	void set_viewobj(ObjectNode *obj);
	void set_viewobj(char *s);
	void set_view(int);
	void eval_view();
	void move_selected();
	void views_mark();
	void views_undo();
	void views_decr();
	void views_incr();
	void views_reset();
	void views_rewind();
	void views_clip();
	void views_clr();
	void views_restart();

	void set_frame(ViewFrame *v);
	int has_movie_frames();
	int has_movie_path();
	void delete_frames();

	int  make_movie_dir(char *);
	void delete_movie_dir(char *);
	void movie_stop();
	void movie_reset();
	void movie_restart();
	void movie_rewind();
	void movie_record();
	void movie_play();
	void movie_play_video();
	void movie_record_video();
	void movie_interpolate();
	void movie_open();
	void movie_open(char *path);

	void movie_save();
	void movie_save(char *path);

	void movie_add();

	void movie_clip();
	void movie_clr();

	void movie_ss();
	void movie_se();
	void movie_incr();
	void movie_decr();
	void movie_clip_right();
	void movie_clip_left();
	void movie_del();

	void frames_ss();
	void frames_se();
	void frames_incr();
	void frames_decr();
	void frames_clip_right();
	void frames_clip_left();
	void frames_del();

};
#endif
