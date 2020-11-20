#ifndef __KEYIF__
#define __KEYIF__

enum state_codes {
    FN_DFLTS        = 0x00000000, // F1 <default>
	FN_VIEW         = 0x00000100, // F2
	FN_MOVIE        = 0x00000200, // F3
	FN_INFO         = 0x00000400, // F4
	FN_NCOL         = 0x00000500, // F5
	FN_TEST         = 0x00000600, // F6
	FN_TYPES        = 0x00000f00, // function type

	// commands

	CMD_QUIT	 	= 0x10000000,
	CMD_DEBUG	 	= 0x20000000
};

enum special_keys {
	KEY_F1    			= 0xffbe,
	KEY_F2    			= 0xffbf,
	KEY_F3    			= 0xffc0,
	KEY_F4    			= 0xffc1,
	KEY_F5    			= 0xffc2,
	KEY_F6    			= 0xffc3,
	KEY_F7    			= 0xffc4,
	KEY_F8    			= 0xffc5,
	KEY_F9    			= 0xffc6,
	KEY_F10   			= 0xffc7,
	KEY_LEFT  			= 0xff51,
	KEY_RIGHT 			= 0xff53,
	KEY_FORWARD    		= 0xff52,
	KEY_BACKWARD  		= 0xff54,

    KEY_FN_DFLT         = KEY_F1,
	KEY_FN_VIEW         = KEY_F2,
    KEY_FN_MOVIE        = KEY_F3,
	KEY_FN_INFO         = KEY_F4,
	KEY_FN_NCOL         = KEY_F5,
	KEY_FN_TEST         = KEY_F6
};

enum standard_keys{
	KEY_HELP    		= '?',
	KEY_TEST1    		= '/',
	KEY_TEST2    		= '\'',
    KEY_TEST3           = '@',
    KEY_TEST4           = '#',

	KEY_UP    		    = 'u',
	KEY_DOWN  			= 'd',
	KEY_MOVE_SELECT     = 'M',
	KEY_REWIND_VIEWS    = '_',
	KEY_DECR_VIEW       = '-',
	KEY_INCR_VIEW       = '+',
	KEY_UNSELECT        = ' ',
	KEY_MARK_VIEW       = '=',
	KEY_UNDO_MOVE       = 'z',
	KEY_CLEAR_VIEWS     = 'Z',
	KEY_RAND_SEED       = 'I',

	KEY_CMND_YYDEBUG   	= 'y',
	KEY_CMND_QUIT 	 	= 'q',
    KEY_CMND_MV_INC     = '.',
    KEY_CMND_MV_DEC     = ',',
    KEY_CMND_TM_INC     = '>',
    KEY_CMND_TM_DEC     = '<',
	KEY_CMND_MV_GO   	= 'g',
    KEY_CMND_TM_GO      = 't',
	KEY_CMND_MV_RST  	= ';',
    KEY_CMND_TM_RST     = ':',
    KEY_MORE_DETAIL     = ']',
    KEY_LESS_DETAIL     = '[',

	KEY_VIEW_SURFACE 	= 'G',
	KEY_VIEW_ORBIT   	= 'O',
	KEY_VIEW_GLOBAL  	= 'U',
	KEY_VIEW_VERT  		= 'V',
	KEY_VIEW_HORZ  		= 'H',

    KEY_RASTER_SHADOW_MAP = '\\',
    KEY_RASTER_LINES    = '!',

	KEY_RENDER_SHADED   = 'S',
	KEY_RENDER_SOLID   	= 'N',
	KEY_RENDER_LINES   	= 'L',
	KEY_RENDER_POINTS  	= 'P',
	KEY_RENDER_WATER 	= 'W',
	KEY_RENDER_DEPTH 	= 'w',
	KEY_RENDER_MIX 	    = 'c',
	KEY_RENDER_TEXTURES = 'x',
	KEY_RENDER_SPECULAR = 'Y',
	KEY_RENDER_BUMPS    = 'b',
	KEY_RENDER_REFLECT 	= 'r',
	KEY_RENDER_LIGHTING = 'l',
	KEY_RENDER_SMOOTH   = 'n',
	KEY_RENDER_SHADOWS  = 's',
	KEY_RENDER_TWILITE  = 'T',
	KEY_RENDER_FOG      = 'f',
	KEY_RENDER_AVES     = 'v',
	KEY_RENDER_NVIS     = 'i',
	KEY_RENDER_FACE     = 'F',
    KEY_RENDER_AA       = 'a',
    KEY_RENDER_HDR      = 'm',
    KEY_RENDER_GEOMETRY = 'X',

	KEY_ADAPT_OVERLAP   = 'o',
	KEY_ADAPT_LOD       = 'e',
	KEY_ADAPT_UNIFORM   = 'E',
	KEY_ADAPT_HIDDEN    = 'h',
	KEY_ADAPT_REBUILD   = 'R',

	KEY_ADAPT_BEST      = 'A',
	KEY_ADAPT_HIGH      = 'B',
	KEY_ADAPT_MED       = 'C',
	KEY_ADAPT_LOW       = 'D',

	KEY_WRITE_JPEG      = 'j',
	KEY_READ_JPEG       = 'J',
	KEY_WRITE_SAVE      = 'k',
	KEY_OPEN_SAVE       = 'K'

};

class KeyIF
{
protected:
	void show_fogtype();
public:
    static int test;
	void animate(unsigned &motion);
	int standard_key(unsigned &state, unsigned  key);
	int special_key(unsigned &state, unsigned key);
	void set_key(unsigned &state, unsigned key);
	void get_state(unsigned &state);
};

extern KeyIF KIF;

#endif
