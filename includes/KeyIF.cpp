
// entry point for stand-alone operation (no GUI)

#include "SceneClass.h"
#include "ModelClass.h"
#include "TerrainClass.h"
#include "AdaptOptions.h"
#include "RenderOptions.h"
#include "Effects.h"
#include "KeyIF.h"
#include "FileUtil.h"
#include "FrameClass.h"
#include "Octree.h"
#include "MapClass.h"

#include <stdio.h>
#include <iostream>



using std::cout;
using std::endl;

extern int yydebug;
KeyIF KIF;
int KeyIF::test=0;

extern int show_visits;
int ktest1=0;

int cntest=0;
//-------------------------------------------------------------
// KeyIF::get_state() return current system state
//-------------------------------------------------------------
void KeyIF::get_state(unsigned &state)
{
	BIT_SET(state,AUTOTM,TheScene->view->autotm());
	BIT_SET(state,AUTOMV,TheScene->view->automv());
}

extern int clouds_mode;
extern int test2;
extern int test3;
extern int test4;
extern int test5;

//-------------------------------------------------------------
// KeyIF::standard_key() process ascii key code function
//-------------------------------------------------------------
int KeyIF::standard_key(unsigned &state, unsigned key)
{
#ifdef DEBUG_KEYS
    printf("std key %c %x\n",key,state);
#endif
	char path[256];

	switch((char)key){
	// command options
	case KEY_CMND_QUIT:
		BIT_ON(state,CMD_QUIT);
		break;

	case KEY_HELP:
		TheScene->set_help(!TheScene->help());
		TheScene->set_changed_render();
		break;

	case KEY_TEST1:
		if(TheView->autoh())
			TheView->clr_autoh();
		else
			TheView->set_autoh();
//		switch(test1){
//		case 0: // billboards
//			if(TheScene->geometry_shader()){
//				cout << "geometry"<<endl;
//				test1=2;
//			}
//			else{
//				cout << "triangles"<<endl;
//				test1=3;
//			}
//			break;
//		case 2: // geometry
//			cout << "triangles"<<endl;
//			test1=3;
//			break;
//		case 3: // triangles
//			cout << "billboards"<<endl;
//			test1=0;
//			break;
//		}
         TheScene->set_changed_render();

		break;
	case KEY_TEST2:
		test2=test2?0:1;
        //Raster.set_filter_show(!Raster.filter_show());
		cout << "test2="<<test2<<endl;
        TheScene->set_changed_detail();
		break;

	case KEY_TEST3:
		test3=test3?0:1;
 		cout << "test3="<<test3<<endl;
        TheScene->set_changed_detail();
		break;

	case KEY_TEST4:
		test4=test4?0:1;
        //Raster.set_filter_show(!Raster.filter_show());
		cout << "test4="<<test4<<endl;
        TheScene->set_changed_detail();
		break;
	case KEY_RAND_SEED:
		TheScene->setRandomSeed();
		 break;
    case KEY_RASTER_SHADOW_MAP:
    	//Raster.shadow_test=Raster.shadow_test?0:1;
    	if(Raster.shadow_test==0)
    		Raster.shadow_test=1;
     	else
     		Raster.shadow_test=0;
    	cout << "shadow mode="<<Raster.shadow_test<<endl;
        TheScene->set_changed_render();

        break;
	case KEY_RASTER_LINES:
        Raster.set_lines(!Raster.lines());
        TheScene->set_changed_detail();
		break;

    case KEY_MARK_VIEW:
    	TheScene->views_mark();
  		break;
    case KEY_UNDO_MOVE:
    	TheScene->views_undo();
  		break;
    case KEY_DECR_VIEW:
    	TheScene->views_decr();
  		break;
    case KEY_INCR_VIEW:
    	TheScene->views_incr();
        break;
    case KEY_UNSELECT:
    	TheScene->selobj=0;
    	TheScene->set_focus_object(0);
		TheScene->focusobj=0;
		TheScene->view_tilt=0;
		if(TheScene->viewtype==SURFACE){
			TheScene->pitch=0;
			TheScene->view_tilt=-15;
		}
		else{
			TheScene->view_tilt=0;
			TheScene->pitch=-90;
		}
		TheScene->view_skew=0;
		TheScene->set_moved();
  		break;
    case KEY_CLEAR_VIEWS:
		TheScene->views_reset();
  		break;
    case KEY_REWIND_VIEWS:
		TheScene->views_rewind();
  		break;
    case KEY_CMND_MV_GO:
        if(TheScene->automv())
            TheScene->view->clr_automv();
        else
            TheScene->view->set_automv();
        break;

	case KEY_CMND_TM_GO:
        if(TheScene->view->autotm())
			TheScene->view->clr_autotm();
		else
			TheScene->view->set_autotm();
		break;
	case KEY_CMND_MV_INC:
		TheScene->view->increase_vstride();
		TheScene->view->increase_hstride();
		break;
    case KEY_CMND_TM_INC:
        TheScene->view->increase_time();
        break;
    case KEY_CMND_MV_DEC:
		TheScene->view->decrease_vstride();
		TheScene->view->decrease_hstride();
		break;
    case KEY_CMND_TM_DEC:
         TheScene->view->decrease_time();
        break;
    case KEY_CMND_MV_RST:
        TheScene->view->reset_stride();
		if(!TheScene->sidestep()){
			TheScene->view->pitch=0;
			TheScene->view->view_tilt=-15;
		}
		else{
			TheScene->view->view_tilt=0;
			TheScene->view->pitch=-90;
		}
		TheScene->set_moved();
		TheScene->auto_stride();
		break;
    case KEY_CMND_TM_RST:
        TheScene->view->reset_time();
        break;
	// function key mode options

	case '0':
		switch(state&FN_TYPES){
        case FN_DFLTS:
            Render.set_defaults();
            Raster.set_defaults();
            TheScene->set_changed_render();
            break;
		case FN_TEST:
			TheScene->set_eye_view();
			Render.set_draw_nvis(0);
			TheScene->set_changed_detail();
			break;
		case FN_INFO:
			Render.clr_display(ALLINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(0);
			TheScene->set_changed_render();
			break;
		case FN_MOVIE:
			TheScene->movie_reset();
			break;
		case FN_VIEW:
	    	TheScene->view->view_tilt=-15;
	    	TheScene->view->pitch=0;
            TheScene->view->view_tilt=0;
            TheScene->view->view_angle=0;
			TheScene->view->set_moved();
			if(!TheScene->view->automv())
		    	TheScene->show_status(INFO_POSITION);
			break;
		}
		break;

	case '1':
		switch(state&FN_TYPES){
        case FN_DFLTS:
            Adapt.set_defaults();
            TheScene->set_changed_detail();
            break;
		case FN_INFO:
			Render.clr_display(ALLINFO);
			Render.set_display(OBJINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(CSIZE);
			TheScene->set_changed_render();
			break;
		case FN_VIEW:
	        TheScene->heading-=90;
			TheScene->set_changed_position();
			TheScene->set_changed_marker();
			TheScene->set_moved();
			break;
		case FN_TEST:
			TheScene->set_test_view();
			Render.set_draw_nvis(1);
			TheScene->set_changed_detail();
			break;
		case FN_MOVIE:
			TheScene->view->clr_automv();
			TheScene->movie_stop();
			break;
		}
		break;
	case '2':
		switch(state&FN_TYPES){
		case FN_INFO:
			Render.clr_display(ALLINFO);
			Render.set_display(MAPINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(CNODES);
			TheScene->set_changed_render();
			break;
		case FN_VIEW:
			TheScene->heading+=90;
			TheScene->set_changed_position();
			TheScene->set_changed_marker();
			TheScene->set_moved();
			break;
		case FN_TEST:
			TheScene->set_light_view();
			Render.set_draw_nvis(1);
			TheScene->set_changed_render();
			break;
		case FN_MOVIE:
			TheScene->movie_record();
			break;
		}
		break;
	case '3':
		switch(state&FN_TYPES){
		case FN_TEST:
			Render.set_bounds(!Render.draw_bounds());
			TheScene->set_changed_render();
			break;
		case FN_INFO:
			Render.clr_display(ALLINFO);
			Render.set_display(NODEINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(VNODES);
			TheScene->set_changed_render();
			break;
		case FN_VIEW:
			TheScene->look_left();
			break;
		case FN_MOVIE:
			TheScene->movie_rewind();
			break;
		}
		break;
	case '4':
		switch(state&FN_TYPES){
		case FN_TEST:
			Render.set_refs(!Render.draw_refs());
			TheScene->set_changed_render();
			break;
		case FN_INFO:
			Render.clr_display(ALLINFO);
			Render.set_display(RASTINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(LNODES);
			TheScene->set_changed_render();
			break;
		case FN_VIEW:
			TheScene->look_right();
			break;
		case FN_MOVIE:  // replay path
		    if(TheScene->movie->size()==0)
				TheScene->movie_open();
			TheScene->set_intrp(0);
			TheScene->movie_play();
			break;
		}
		break;
	case '5':
		switch(state&FN_TYPES){
		case FN_TEST:
			Render.set_bounds(!Render.draw_bounds());
			TheScene->set_changed_render();
			break;
		case FN_INFO:
			Render.clr_display(ALLINFO);
			Render.set_display(OCTINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(ENODES);
			TheScene->set_changed_render();
			break;
		case FN_VIEW:
			TheScene->view->view_tilt-=10;
			TheScene->view->set_moved();
			break;
		case FN_MOVIE:  // save path
    		TheScene->movie_save();
			break;
		}
		break;
	case '6':
		switch(state&FN_TYPES){
		case FN_INFO:
 			Render.clr_display(ALLINFO);
			Render.set_display(CRTRINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(MNODES);
			TheScene->set_changed_render();
			break;
		case FN_VIEW:
			TheScene->view->view_tilt+=10;
			TheScene->view->set_moved();
			break;
		case FN_MOVIE:  // open path
			TheScene->movie_open();
			break;
		}
		break;
	case '7':
		switch(state&FN_TYPES){
		case FN_INFO:
 			Render.clr_display(ALLINFO);
			Render.set_display(TRNINFO);
			TheScene->set_changed_render();
			break;
		case FN_NCOL:
			Render.set_colors(RNODES);
			TheScene->set_changed_render();
			break;
		case FN_VIEW:
			TheScene->scan_left();
			break;
		case FN_MOVIE:  // record live move
			TheScene->record_movie_from_path();
		}
		break;

	case '8':
		switch(state&FN_TYPES){
		case FN_INFO:
 			print_strings=print_strings?0:1;
			break;
        case FN_NCOL:
            Render.set_colors(SNODES);
            TheScene->set_changed_render();
            break;
		case FN_VIEW:
			TheScene->scan_right();
			break;
		case FN_MOVIE:
			TheScene->movie_record_video();
			break;
		}
		break;

	case '9':
		switch(state&FN_TYPES){
		case FN_INFO:
 			draw_strings=draw_strings?0:1;
			break;
		case FN_MOVIE:
			TheScene->movie_play_video();
			break;
		case FN_VIEW:
	    	TheScene->view->view_tilt=0;
            TheScene->view->view_angle=0;
	    	TheScene->view->pitch=-90;
			TheScene->view->set_moved();
			if(!TheScene->view->automv())
		    	TheScene->show_status(INFO_POSITION);
			break;
		}
		break;

	// rendering options
    case KEY_RENDER_AA:
        if(Render.dealias())
            cout << "AA OFF"<<endl;
        else
            cout << "AA ON"<<endl;

        Render.set_dealias(!Render.dealias());
        TheScene->set_changed_render();
        break;
    case KEY_RENDER_HDR:
        Raster.set_hdr(!Raster.hdr());
        if(!Raster.hdr())
            cout << "HDR OFF"<<endl;
        else
            cout << "HDR ON"<<endl;
        TheScene->set_changed_render();
        break;

	case KEY_RENDER_SHADED:
		if(!Render.draw_shaded())
			Render.pop();

		TheScene->set_changed_render();
		TheScene->set_changed_detail();
		Render.show_shaded();
		break;
	case KEY_RENDER_SOLID:
		if(Render.draw_shaded())
			Render.push();
		Render.set_lighting(false);
		Render.set_fog(false);
		Render.set_haze(false);
		Render.set_textures(false);
		Render.set_bumps(false);
		TheScene->set_changed_render();
		Render.show_solid();
		break;
	case KEY_RENDER_LINES:
		if(Render.draw_shaded())
			Render.push();
		Render.set_lighting(false);
		Render.set_fog(false);
		Render.set_haze(false);
		Render.set_textures(false);
		Render.set_bumps(false);
		TheScene->set_changed_render();
		Render.show_lines();
		break;
	case KEY_RENDER_POINTS:
		if(Render.draw_shaded())
			Render.push();
		Render.set_lighting(false);
		Render.set_fog(false);
		Render.set_haze(false);
		Render.set_textures(false);
		Render.set_bumps(false);
		TheScene->set_changed_render();
		Render.show_points();
		break;
	case KEY_RENDER_LIGHTING:
		Render.set_lighting(!Render.lighting());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_GEOMETRY:
		Render.set_geometry(!Render.geometry());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_AVES:
		Render.set_avenorms(!Render.avenorms());
		TheScene->set_changed_detail();
        if(Render.avenorms())
        	cout << "ave norms on"<<endl;
        else
        	cout << "ave norms off"<<endl;
        TheScene->rebuild_all();
		break;
	case KEY_RENDER_TWILITE:
		Render.set_twilite(!Render.twilite());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_TEXTURES:
		Render.set_textures(!Render.textures());
		TheScene->set_changed_render();
        TheScene->set_changed_detail();

		break;
	case KEY_RENDER_SPECULAR:
		TheScene->set_spectex(TheScene->spectex()?0:1);
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_SMOOTH:
		Render.set_smooth_shading(!Render.smooth_shading());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_WATER:
		Render.set_water_show(!Render.show_water());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_DEPTH:
		Raster.set_water_depth(!Raster.water_depth());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_MIX:
		Raster.set_water_modulation(!Raster.water_modulation());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_REFLECT:
		Raster.set_reflections(!Raster.reflections());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_FOG:
		Render.set_fog(!Render.fog());
		Render.set_haze(!Render.haze());
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_BUMPS:
		Render.set_bumps(!Render.bumps());
		TheScene->set_changed_render();
        TheScene->set_changed_detail();

		break;
	case KEY_RENDER_SHADOWS:
		Raster.set_shadows(!Raster.shadows());
		if(Raster.shadows())
		    cout << "shadows on" << endl;
		else
		    cout << "shadows off" << endl;
		TheScene->set_changed_render();
		break;
	case KEY_RENDER_NVIS:
		Render.set_draw_nvis(!Render.draw_nvis());
		TheScene->set_changed_render();
		if(Render.draw_nvis())
		    cout << "draw all" << endl;
		else
		    cout << "draw visible" << endl;
		break;
	case KEY_RENDER_FACE:
		if(Render.showall()){
		    cout << "SHOW FRONT" << endl;
			Render.set_front();
			Render.set_draw_nvis(0);
		}
		else if(Render.frontface()){
		    cout << "SHOW BACK" << endl;
			Render.set_back();
			Render.set_draw_nvis(1);
		}
		else {
			Render.set_both();
		    cout << "SHOW ALL" << endl;
			Render.set_draw_nvis(1);
		}
		TheScene->set_changed_render();
		break;

	// view options
    case KEY_MOVE_SELECT:
  		TheScene->move_selected();
  		break;
	case KEY_VIEW_SURFACE:
		TheScene->change_view(SURFACE);
		break;
	case KEY_VIEW_ORBIT:
		TheScene->change_view(ORBITAL);
		break;
	case KEY_VIEW_GLOBAL:
	    if(TheScene->viewtype != GLOBAL)
		{
	   		ObjectNode *viewobj=TheScene->viewobj;
	    	TheScene->viewobj=0;
	    	TheScene->selobj=0;
			TheScene->set_moved();
			TheScene->change_view(GLOBAL);
			TheScene->selobj=viewobj;
		}
		break;

	case KEY_VIEW_VERT:
	    TheScene->view->view_tilt=0;
	    TheScene->view->pitch=-90;
		TheScene->view->set_moved();
		if(!TheScene->view->automv())
		    TheScene->show_status(INFO_POSITION);
		break;

	case KEY_VIEW_HORZ:
	    TheScene->view->view_tilt=-15;
	    TheScene->view->pitch=0;
		TheScene->view->set_moved();
		if(!TheScene->view->automv())
		    TheScene->show_status(INFO_POSITION);
		break;

    // misc.
	case KEY_WRITE_JPEG:
		TheScene->jpeg_save((char*)"image");
		break;
	case KEY_READ_JPEG:
		TheScene->jpeg_show((char*)"image");
		break;
	case KEY_WRITE_SAVE:
		File.makeFilePath(File.saves,(char*)"save.spx",path);
		TheScene->save(path);
		break;
	case KEY_OPEN_SAVE:
		File.makeFilePath(File.saves,(char*)"save.spx",path);
		TheScene->open(path);
		break;

	//Adapt options

	case KEY_ADAPT_OVERLAP:
		Adapt.set_overlap_test(!Adapt.overlap_test());
		TheScene->rebuild_all();
		break;
	case KEY_ADAPT_UNIFORM:
		Adapt.set_uniform(!Adapt.uniform());
		TheScene->rebuild_all();
		break;
	case KEY_ADAPT_LOD:
		Adapt.set_lod(!Adapt.lod());
		TheScene->rebuild_all();
		break;
	case KEY_ADAPT_HIDDEN:
		Adapt.set_mindcnt(!Adapt.mindcnt());
		//TheScene->rebuild_all();
		break;
	case KEY_ADAPT_REBUILD:
		TheScene->rebuild_all();
		break;
	case KEY_MORE_DETAIL:
		TheScene->increase_detail();
		break;
	case KEY_LESS_DETAIL:
		TheScene->decrease_detail();
		TheScene->rebuild_all();
		break;
	case KEY_ADAPT_BEST:
		TheScene->set_quality(BEST);
		break;
	case KEY_ADAPT_HIGH:
		TheScene->set_quality(HIGH);
		break;
	case KEY_ADAPT_MED:
		TheScene->set_quality(NORMAL);
		break;
	case KEY_ADAPT_LOW:
		TheScene->set_quality(DRAFT);
		break;
	//case KEY_CMND_YYDEBUG:
	//	yydebug=yydebug?0:1;
	//	break;
	case KEY_UP:
		TheScene->view->up();
		if(!TheScene->view->automv())
		    TheScene->show_status(INFO_POSITION);
  		break;
	case KEY_DOWN:
		TheScene->view->down();
		if(!TheScene->view->automv())
		    TheScene->show_status(INFO_POSITION);
  		break;
	default:
		return 0;
	}
	return 1;
}

//-------------------------------------------------------------
// KeyIF::special_key() process non-ascii key code function
//-------------------------------------------------------------
int KeyIF::special_key(unsigned &state, unsigned key)
{
	switch(key){
	case KEY_FORWARD:
		TheScene->view->forward();
		if(!TheScene->view->automv())
		    TheScene->show_status(INFO_POSITION);
		break;
	case KEY_BACKWARD:
		TheScene->view->backward();
		if(!TheScene->view->automv())
			TheScene->show_status(INFO_POSITION);
		break;
	case KEY_LEFT:
	    TheScene->view->left();
		if(!TheScene->view->automv())
			TheScene->show_status(INFO_POSITION);
  		break;
	case KEY_RIGHT:
	    TheScene->view->right();
		if(!TheScene->view->automv())
			TheScene->show_status(INFO_POSITION);
  		break;

	// function key mode select
    case KEY_FN_DFLT:
        MSK_SET(state,FN_TYPES,0);
        printf("NORMAL MODE\n");
        TheScene->set_function(DFLT);
        TheScene->set_changed_render();
        break;

	case KEY_FN_VIEW:
        MSK_SET(state,FN_TYPES,FN_VIEW);
		printf("VIEWS MODE\n");
		TheScene->set_views_mode();
		TheScene->set_function(VIEWS);
		TheScene->set_changed_render();
		break;
	case KEY_FN_INFO:
		MSK_SET(state,FN_TYPES,FN_INFO);
		TheScene->set_function(INFO);
		printf("INFO MODE\n");
		TheScene->set_changed_render();
		break;
	case KEY_FN_NCOL:
		MSK_SET(state,FN_TYPES,FN_NCOL);
		TheScene->set_function(NCOLS);
		printf("DEBUG MODE\n");
		TheScene->set_changed_render();
		break;
	case KEY_FN_TEST:
		MSK_SET(state,FN_TYPES,FN_TEST);
		TheScene->set_function(TEST);
		TheScene->set_changed_render();
		printf("TEST VIEW MODE\n");
		break;
	case KEY_FN_MOVIE:
		MSK_SET(state,FN_TYPES,FN_MOVIE);
		printf("MOVIE MODE\n");
		TheScene->set_movie_mode();
		TheScene->set_function(MOVIE);
		TheScene->set_changed_render();
		break;

	default:
		return 0;
	}
  	//TheScene->set_changed_render();
	//printf("special key %x %x\n",key,state);
	return 1;
}

//-------------------------------------------------------------
// KeyIF::set_key() process a key function
//-------------------------------------------------------------
void KeyIF::set_key(unsigned &state,unsigned key)
{
	if(!special_key(state, key))
		standard_key(state, key);
	get_state(state);
}

