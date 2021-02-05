// ########## begin geometry_funcs.frag #########################

#define PRODUCE_VERTICES\
	float dt = 1.0 / float( TESSLVL ); \
	float t_top = 1.0; \
	for( int it = 0; it < TESSLVL; it++ ){ \
		float t_bot = t_top - dt;\
		float smax_top = 1.0 - t_top;\
		float smax_bot = 1.0 - t_bot;\
		int nums = it + 1;\
		float ds_top = smax_top / float(nums - 1 );\
		float ds_bot = smax_bot / float(nums);\
		float s_top = 0.0;\
		float s_bot = 0.0;\
		int j=0;\
		for( int is = 0; is < nums; is++ ,j++)\
		{\
			ProduceVertex(s_bot, t_bot);\
			ProduceVertex(s_top, t_top);\
			s_top += ds_top;\
			s_bot += ds_bot;\
		}\
		ProduceVertex(s_bot, t_bot);\
		EndPrimitive();\
		t_top = t_bot;\
		t_bot -= dt;\
	}

#define PRODUCE_INDEXED_VERTICES \
	for(int it = 0,index=0,ntop=0,nbot=1; it < TESSLVL; it++ ){\
		int nums = it + 1; \
		for( int is = 0; is < nums; is++ ) { \
		   index=nbot+is; \
		   ProduceVertex(index); \
		   index=ntop+is; \
		   ProduceVertex(index); \
		} \
		index=nbot+nums; \
		ProduceVertex(index); \
		EndPrimitive(); \
		ntop=nbot; \
		nbot+=nums+1; \
	}

#define CALC_ZNOISE(func) \
	v1= Vertex1.xyz; \
 	gv = func; \
 	g=gv.x; \
	vec3 v=normalize(pv)*g; \
	p.xyz+=v;

#define CALC_ZNORMAL(func) \
    { \
	    Normal.xyz=(Normal.xyz); \
		float delta=1e-5; \
		float nbamp = 5e-4/delta; \
		v1 = vec3(Vertex1.x+delta,Vertex1.y,Vertex1.z);  \
		gv = func; \
		df.x =gv.x; \
		v1 = vec3(Vertex1.x,Vertex1.y+delta,Vertex1.z); \
		gv = func; \
		df.y =gv.x; \
		v1 = vec3(Vertex1.x,Vertex1.y,Vertex1.z+delta); \
		gv = func; \
		df.z =gv.x; \
		df = (df-vec3(g,g,g))*(nbamp); \
	    Normal.xyz=normalize(Normal.xyz-2e5*gl_NormalMatrix *df); \
	} \

#define NOISE_COLOR(func) \
	v1= Vertex1.xyz; \
    vec4 ncolor=func; \
 	color =ncolor+color;

#define NOISE_VARS \
    df=vec3(0); \
    nbamp=0.0; \
    g=0.0; \
    b=0.0; \
    noise_fade=0.0; \
	gv=vec4(0.0);

// ########## end geometry_funcs.frag #########################
