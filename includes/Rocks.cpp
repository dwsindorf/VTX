
//************************************************************
// classes Rock, RockMgr
//************************************************************
#include "SceneClass.h"
#include "RenderOptions.h"
#include "Rocks.h"
#include "Util.h"
#include "MapNode.h"
#include "ModelClass.h"
#include "AdaptOptions.h"
#include "TerrainClass.h"
#include "UniverseModel.h"
#include "GLSLMgr.h"
#include "Effects.h"

#include <map>
#include <set>

extern double Hscale, Drop, MaxSize,Height,Theta,Phi,Slope,MaxHt;
extern int test7, test8;

extern Point MapPt;
extern double ptable[];

static const char *def_rnoise_expr="noise(GRADIENT,0,2)\n";

static TerrainData Td;

#define PSCALE 0.006 // placement scale factor

//#define PRINT_STATS
#define PRINT_CACHE_STATS

// 3d rocks using marching cubes
// Tasks:
// 1. generate color test if 3d flag is set - done
// 2. set density gradient on surface if 3d - done)
// 3. capture size and position information for placement - done
// 4. create array of 3d rocks with placement and size info - done
// 7. for each layer and rock in layer emit rock as a marchingCubes Object
//    - start with simple sphere (done)
// 8. refactor rock3d and rock classes to use the same wxWidgets GUI
//    - change both to use Point in base [] for modulation rather than argument 
//      note: 2d rocks now always map from surface
//    - in both modify arguments to "standard" as in plants and sprites
//      so all rocks can have biases
//    - add "3d" checkbox to UI
//      in UI change only class name (rock or rock3d) to regenerate
// 9. add noise modulation to 3d rocks using existing methods 
// 10. add texture and color
//    - may need to capture into a private TerrainProperties object ?

//************************************************************
// Rock3D class
//************************************************************
Rock3D::Rock3D(int t, TNode *e):PlaceObj(t,e)
{
}
PlacementMgr *Rock3D::mgr() { 
	return ((TNrocks3D*)expr)->mgr;
}

//************************************************************
// helper functions
//************************************************************

static SurfaceFunction makeCenteredSphere(const Point& center, double radius) {
    return [center, radius](double x, double y, double z) -> double {
        double dx = x - center.x;
        double dy = y - center.y;
        double dz = z - center.z;
        double distance = sqrt(dx*dx + dy*dy + dz*dz);
        return radius - distance;  // Positive inside sphere of given radius
    };
}

static SurfaceFunction makeCenteredEllipsoid(const Point& center, double rx, double ry, double rz) {
    return [center, rx, ry, rz](double x, double y, double z) -> double {
        double dx = (x - center.x) / rx;
        double dy = (y - center.y) / ry;
        double dz = (z - center.z) / rz;
        double distance = sqrt(dx*dx + dy*dy + dz*dz);
        return 1.0 - distance;  // Positive inside ellipsoid
    };
}
// Noisy sphere field for marching cubes iso-surface extraction using built-in noise function
static SurfaceFunction makeNoisyRockField(const Point& center, double radius, int seed, double noiseAmpl) {
    return [center, radius, seed, noiseAmpl](double x, double y, double z) -> double {
        static std::map<int, PerlinNoise> noiseGens;
        if (noiseGens.find(seed) == noiseGens.end()) {
            noiseGens[seed] = PerlinNoise(seed);
        }
        PerlinNoise& noise = noiseGens[seed];
        
        double dx = x - center.x;
        double dy = y - center.y;
        double dz = z - center.z;
        double distance = sqrt(dx*dx + dy*dy + dz*dz);
        
        // Base sphere
        double baseSphere = radius - distance;
        
        // Add noise to the iso-surface
        // Scale sample position to get consistent look regardless of rock size
        double scale = 2.0 / radius;
        double n = noise.octaveNoise(dx * scale, dy * scale, dz * scale, 6, 0.5, 2.0, 0.5, 0.5);
        
        return baseSphere + n * noiseAmpl * radius;
    };
}

// Noisy sphere field for marching cubes iso-surface extraction using standard noise function
// Noisy ellipsoid field for marching cubes iso-surface extraction using standard noise function
static SurfaceFunction makeRockField(const Point& center, double rx, double ry, double rz, int seed, double noiseAmpl, TNode *tc) {
    return [center, rx, ry, rz, seed, noiseAmpl, tc](double x, double y, double z) -> double {
    	int rseed=TheNoise.rseed;
     	TheNoise.rseed = seed;
        
        double dx = x - center.x;
        double dy = y - center.y;
        double dz = z - center.z;
        
        // Ellipsoid distance (not sphere)
        double ex = dx / rx;
        double ey = dy / ry;
        double ez = dz / rz;
        double ellipsoidDist = sqrt(ex*ex + ey*ey + ez*ez);
        
        // Base ellipsoid (positive inside)
        double baseEllipsoid = 1.0 - ellipsoidDist;
        
        double scale = 2.0 / ((rx + ry + rz) / 3.0);  // Average radius for scale
        Point np = Point(dx, dy, dz);
 		TheNoise.push(np);
 		CurrentScope->revaluate();
 		double rz = 0;
 		tc->eval();
		if (tc->typeValue() == ID_POINT)
 			rz = S0.p.z;
 		else
 			rz = S0.s;

 		TheNoise.pop();
 		TheNoise.rseed = rseed;
        
        return baseEllipsoid - rz * noiseAmpl;
    };
}

// Post-mesh vertex displacement OLD uses Perlin noise function
static void applyVertexDisplacement(MCObject* rock, int seed, double amplitude) {
    static std::map<int, PerlinNoise> noiseGens;
    if (noiseGens.find(seed) == noiseGens.end()) {
        noiseGens[seed] = PerlinNoise(seed);
    }
    PerlinNoise& noise = noiseGens[seed];
    
    Point center = rock->worldPosition;
    double rockSize = rock->baseSize;
    
    for (auto& tri : rock->mesh) {
        for (int v = 0; v < 3; v++) {
            Point& vertex = tri.vertices[v];
            
            // Direction from center to vertex (outward normal)
            Point dir = (vertex - center).normalize();
            
            // Normalize position relative to rock center and size
            // This gives us unit-scale coordinates for noise sampling
            double nx = (vertex.x - center.x) / rockSize;
            double ny = (vertex.y - center.y) / rockSize;
            double nz = (vertex.z - center.z) / rockSize;
            
            // Sample noise at normalized coordinates with some frequency
            double scale = 4.0;
            double n = noise.octaveNoise(
                nx * scale + seed * 0.1,  // Offset by seed for variation
                ny * scale, 
                nz * scale, 
                3, 0.5, 2.0, 0.5, 0.5
            );
            
            // Displace vertex along normal direction
            vertex.x += dir.x * n * amplitude;
            vertex.y += dir.y * n * amplitude;
            vertex.z += dir.z * n * amplitude;
        }
    }
}
// Post-mesh vertex displacement NEW uses standard noise function
static void applyVertexDisplacement(MCObject* rock, int seed, double amplitude, TNode *tc) {
    
    Point center = rock->worldPosition;
    double rockSize = rock->baseSize;
    
    static int cnt=0;
    int rseed=TheNoise.rseed;
   	TheNoise.rseed=seed;
    CurrentScope->revaluate();
      
    for (auto& tri : rock->mesh) {
        for (int v = 0; v < 3; v++) {
            Point &vertex = tri.vertices[v];
            
            // Direction from center to vertex (outward normal)
            Point dir = (vertex - center).normalize();
            
            // Normalize position relative to rock center and size
            // This gives us unit-scale coordinates for noise sampling
            double nx = (vertex.x - center.x) / rockSize;
            double ny = (vertex.y - center.y) / rockSize;
            double nz = (vertex.z - center.z) / rockSize;
			Point np(nx,ny,nz);
			TheNoise.push(np);
			Point pn;
			double rz=0;
			Point pv=vertex;
			tc->eval();
			
			Point pd=S0.p;
			if(pd.x==0 && pd.y==0)
				pd.x=pd.y=pd.z;
			
			Point delta=dir*pd*amplitude;
			vertex=vertex+delta;
			cnt++;

			TheNoise.pop();
        }
    }
    TheNoise.rseed=rseed;
}
// LOD template cache - now keyed by resolution AND noise parameters
struct LODKey {
    int resolution;
    bool noisy;
    
    bool operator<(const LODKey& other) const {
        if (resolution != other.resolution) return resolution < other.resolution;
        return noisy < other.noisy;
    }
};

static std::map<LODKey, MCObject*> lodTemplates;


//************************************************************
// Rock3DMgr class
//************************************************************
int Rock3DMgr::stats[MAX_ROCK_STATS][2];

Rock3DMgr::Rock3DMgr(int i) : PlacementMgr(i)
{
	MSK_SET(type,PLACETYPE,MCROCKS);
	comp=0.0;
	drop=0.0;
	vnoise=0;
	rnoise=0;

#ifdef TEST_ROCKS
    set_testColor(true);
#endif
    set_testDensity(true);
}

void Rock3DMgr::eval(){	
	PlacementMgr::eval(); 
}

void Rock3DMgr::init()
{
	PlacementMgr::init();
  	reset();
}

//-------------------------------------------------------------
// Rock3DMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *Rock3DMgr::make(Point4DL &p, int n)
{
    return new Placement(*this,p,n);
}
bool Rock3DMgr::testColor() { 
	return PlacementMgr::testColor()?true:false;
}
bool Rock3DMgr::testDensity(){ 
	return true;
}

struct RockLodEntry {
    int    res;     // voxel resolution
    double maxPts;  // upper bound for pts (pts < maxPts)
};

static const RockLodEntry kRockLodTable[MAX_ROCK_STATS] = {
    {  2,  2.0 },  // pts < 2   → res 2
    {  3,  3.0 },  // pts < 3   → res 3
    {  4,  5.0 },  // pts < 5   → res 4
    {  8, 10.0 },  // pts < 10  → res 6
    { 16, 20.0 },  // pts < 20  → res 12
    { 32, 35.0 },  // pts < 35  → res 16
    { 64, 60.0 },  // pts < 60  → res 24
    { 128,  1e9 }   // default / max detail (increased from 24)
};
void Rock3DMgr::clearStats(){
	for(int i=0;i<MAX_ROCK_STATS;i++){
		stats[i][0]=stats[i][1]=0;
	}
}

void Rock3DMgr::printStats(){
    int rcnt = 0;
    int tcnt = 0;
    std::cout << "------- 3D Rocks stats --------" << std::endl;
    for (int i = 0; i < MAX_ROCK_STATS; ++i) {
#ifdef PRINT_STATS
        std::cout << "res:"       << kRockLodTable[i].res
                  << " cnt:"      << stats[i][0]
                  << " triangles:"<< stats[i][1]
                  << std::endl;
#endif
        rcnt  += stats[i][0];
        tcnt  += stats[i][1];
    }
    std::cout << "Totals rocks:"    << rcnt
              << " triangles:"      << tcnt
              << std::endl;
}

void Rock3DMgr::setStats(int res, int tris){
    for (int i = 0; i < MAX_ROCK_STATS; ++i) {
        if (kRockLodTable[i].res == res) {
            stats[i][0]++;           // count of rocks at this res
            stats[i][1] += tris;     // total triangles
            return;
        }
    }
}

int Rock3DMgr::getLODResolution(double pts) {
   extern int test2;
	if (test2)
		return 24;

	for (int i = 0; i < MAX_ROCK_STATS; ++i) {
		if (pts < kRockLodTable[i].maxPts) {
			return kRockLodTable[i].res;
		}
	}
	// Fallback (shouldn't hit if table is well-formed):
	return kRockLodTable[MAX_ROCK_STATS - 1].res;
}
//************************************************************
// Rock3DObjMgr class
//************************************************************
MCObjectManager Rock3DObjMgr::rocks;
bool Rock3DObjMgr::vbo_valid = false;
ValueList<PlaceData*> Rock3DObjMgr::data(10000, 5000);
std::map<int, MCObject*> Rock3DObjMgr::lodTemplates;
std::map<Rock3DObjMgr::RockCacheKey, Rock3DObjMgr::RockCacheEntry> Rock3DObjMgr::rockCache;
int Rock3DObjMgr::cacheHits = 0;
int Rock3DObjMgr::cacheMisses = 0;
int Rock3DObjMgr::cacheRegens = 0;

Rock3DObjMgr::~Rock3DObjMgr(){
	freeLODTemplates();
}

//#define TEST // use built in noise functions

MCObject* Rock3DObjMgr::getTemplateForLOD(int resolution, bool noisy, double noiseAmpl, int rval, TNode *tc, double comp) {
    // Key includes noise parameters AND compression
    int key = noisy ? (resolution * 1000 + (int)(noiseAmpl * 100) + (int)(comp * 10)) : (resolution + (int)(comp * 10));
    
    auto it = lodTemplates.find(key);
    if (it != lodTemplates.end()) {
        return it->second;
    }
    
    Point origin(0, 0, 0);
    MCObject* templateSphere = new MCObject(origin, 1.0);
    
    MCGenerator generator;
    
    double rx = 0.5;
    double ry = 0.5;
    double rz = 0.5 * (1.0 - comp);
    if (rz < 0.05) rz = 0.05;

    // For flatter rocks, expand X/Y slightly
    if (comp > 0.3) {
        double expand = 1.0 + comp * 0.5;
        rx *= expand;
        ry *= expand;
    }

    // Adjust bounds to match ellipsoid size
    // Add extra margin for noise (especially when noiseAmpl is high)
    double margin = 1.1 + noiseAmpl * 2.0;  // Increase margin with noise amplitude
    Point boundsMin(-rx * margin, -ry * margin, -rz * margin);
    Point boundsMax(rx * margin, ry * margin, rz * margin);    
   
    SurfaceFunction field;
    if (noisy) {
#ifdef TEST
        field = makeNoisyRockField(origin, 0.5, 0, noiseAmpl);
#else
        field = makeRockField(origin, rx, ry, rz, rval, noiseAmpl, tc);
#endif
    } else {
        field = makeCenteredEllipsoid(origin, rx, ry, rz);
    }
    
    templateSphere->mesh = generator.generateMesh(field, boundsMin, boundsMax, resolution, 0.0);
    templateSphere->meshValid = true;
    
    lodTemplates[key] = templateSphere;
#ifdef PRINT_STATS    
    cout << "Created LOD template: resolution=" << resolution 
         << " noisy=" << noisy << " comp=" << comp
         << " tris=" << templateSphere->mesh.size() << endl;
#endif    
    return templateSphere;
}
void Rock3DObjMgr::freeLODTemplates() {
    for (auto& pair : lodTemplates) {
        delete pair.second;
    }
    lodTemplates.clear();
}
//-------------------------------------------------------------
// Rock3DObjMgr::setProgram() initialize shader
//-------------------------------------------------------------
bool Rock3DObjMgr::setProgram() {
    if (!data.size || !objs.size)
        return false;

	if(PlaceObjMgr::shadow_mode)
		return false;

    char defs[1024] = "";
    sprintf(defs, "#define NLIGHTS %d\n", Lights.size);

    GLSLMgr::setDefString(defs);
    GLSLMgr::loadProgram("rocks3d.vert", "rocks3d.frag");

    GLhandleARB program = GLSLMgr::programHandle();
    if (!program)
        return false;

    GLSLVarMgr vars;

    Planetoid *orb = (Planetoid*)TheScene->viewobj;
    Color diffuse = orb->diffuse;
    Color ambient = orb->ambient;
     
    vars.newFloatVec("Diffuse", diffuse.red(), diffuse.green(), diffuse.blue(), diffuse.alpha());
    vars.newFloatVec("Ambient", ambient.red(), ambient.green(), ambient.blue(), ambient.alpha());

    vars.setProgram(program);
    vars.loadVars();

    GLSLMgr::setProgram();
    GLSLMgr::loadVars();

    return true;
}
void Rock3DObjMgr::free() { 
	data.free();
    rocks.clear(); 
    vbo_valid = false; 
}

//-------------------------------------------------------------
// Rock3DObjMgr::collect() generate array of placements (data)
//-------------------------------------------------------------
void Rock3DObjMgr::collect() {
    data.free();
    for (int i = 0; i < objs.size; i++) {
        PlaceObj *obj = objs[i];
        obj->mgr()->collect(data);
    }
    if (data.size)
        data.sort();
    vbo_valid = false;
}

void Rock3DObjMgr::render_shadows(){
	if(objs.size==0)
		return;
	shadow_mode=true;
    
	// not really needed - these are the defaults for shadow mode
	Raster.setShadowProgram("shadows.vert",0,0);
	Raster.setProgram(Raster.PLACE_SHADOWS);
	render();
	shadow_mode=false;
}
//-------------------------------------------------------------
// Rock3DObjMgr::render() create and render the 3d rocks
//-------------------------------------------------------------
void Rock3DObjMgr::render() {
    int n = data.size;
    if (n == 0)
        return;
    
    bool wireframe = test7;
    bool smooth = test8;

    bool moved = TheScene->moved();
	bool changed=TheScene->changed_detail();

    bool update_needed = moved || changed || !vbo_valid;
  
    if (update_needed) {
        if (changed) {
            std::cout << "Settings changed - clearing cache" << std::endl;
            rockCache.clear();
            rocks.clear();
            vbo_valid = false;
        }

   		Point xpoint = TheScene->xpoint;
        rocks.clear();
        Rock3DMgr::clearStats();
        
        // Mark all cached as not used
        for (auto& pair : rockCache) {
            pair.second.framesSinceUsed++;
        }
        
        int hits = 0, misses = 0, regens = 0;

        for (int i = n - 1; i >= 0; i--) {
            PlaceData *s = data[i];
            
            Rock3DMgr *pmgr = (Rock3DMgr*)s->mgr;
            TNode *tc = pmgr->rnoise;
            
            char tmp[1024];
            tmp[0] = 0;
            if (tc)
                tc->valueString(tmp);
            TNode *tv = pmgr->vnoise;
  
            if (tv)
                tv->valueString(tmp);
            std::string estr = tmp;
            
            double isoNoiseAmpl = pmgr->noise_amp;
            
            bool useNoisyIsoSurface = isoNoiseAmpl > 0;
            bool useVertexDisplacement = (tv != nullptr && tv->isEnabled());
            
            double vertexNoiseAmpl = useVertexDisplacement ? 0.5 * pmgr->noise_amp : 0;

            Point eyePos = s->vertex - xpoint;
            double size = PSCALE * s->radius;
            double pts = s->pts;
            double dist = s->dist;
            int rval = s->rval;
            double comp = pmgr->comp;
            double drop = pmgr->drop;
             
            int resolution = Rock3DMgr::getLODResolution(pts);
            
            // Create cache key from world position
            RockCacheKey key(s->vertex);
            
            // Use frame-unique name (not cache key based)
            char name[64];
            sprintf(name, "rock_%d", i);
            
            // Check cache
            auto it = rockCache.find(key);
            
            bool needsGeneration = true;
            std::vector<MCTriangle> templateMesh;
            
            if (it != rockCache.end()) {
                RockCacheEntry& entry = it->second;
                entry.framesSinceUsed = 0;
                
                // Check what's different
                bool resMatch = (entry.resolution == resolution);
                bool seedMatch = (entry.seed == rval);
                bool isoNoiseExprMatch = (estr == entry.estr);
                 
                if (!resMatch || !seedMatch || !isoNoiseExprMatch) {
 #ifdef PRINT_CACHE_STATS
                    if (regens < 5) {
                        std::cout << "Regen rock " << i << ": ";
                        if (!resMatch) std::cout << "res " << entry.resolution << "->" << resolution << " ";
                        if (!seedMatch) std::cout << "seed " << entry.seed << "->" << rval << " ";
                        std::cout << std::endl;
                    }
#endif
                    regens++;
                    rockCache.erase(it);
                    needsGeneration = true;
                } else {
                    // CACHE HIT
                    templateMesh = entry.mesh;
                    needsGeneration = false;
                    hits++;
                }
            } else {
                misses++;
            }           
            // Generate if needed
            if (needsGeneration) {
                MCObject* templateSphere = getTemplateForLOD(resolution, useNoisyIsoSurface, isoNoiseAmpl, rval, tc, comp);                
                if (!templateSphere || templateSphere->mesh.empty())
                    continue;               
                // Copy mesh without modification
                for (const auto& tri : templateSphere->mesh) {
                    MCTriangle newTri;
                    for (int v = 0; v < 3; v++) {
                        newTri.vertices[v] = tri.vertices[v];
                    }
                    newTri.normal = Point(-tri.normal.x, -tri.normal.y, -tri.normal.z);
                    templateMesh.push_back(newTri);
                }               
                // Apply vertex displacement in template space
                if (useVertexDisplacement) {
                    MCObject tempRock(Point(0,0,0), 1.0);
                    tempRock.mesh = templateMesh;
#ifdef TEST
                    applyVertexDisplacement(&tempRock, rval, vertexNoiseAmpl);     // use built-in perlin noise
#else
                    applyVertexDisplacement(&tempRock, rval, vertexNoiseAmpl, tv); // use standard TNoise function
#endif
                    templateMesh = tempRock.mesh;  // COPY THE DISPLACED MESH BACK!
                 }
                                
                // Store in cache
                RockCacheEntry entry;
                entry.mesh = templateMesh;
                entry.worldVertex = s->vertex;
                entry.resolution = resolution;
                entry.estr = estr;
                entry.seed = rval;
                entry.framesSinceUsed = 0;
                rockCache[key] = entry;
            }
            
            // Create rock and transform to eye space
            MCObject *rock = rocks.addObject(name, eyePos, size);
            if (rock) {
                rock->setDistanceInfo(dist, pts);
                rock->mesh.clear();
                
                // Get world position and surface normal
                Point worldPos = s->vertex;
                Point up = s->normal;
                
                double dscale=PSCALE*drop*(1-0.5*comp)*0.5;
                
                // Apply drop: lower the rock center along surface normal
                Point rockCenter = worldPos - up*(s->radius * dscale);
                
                Point right, forward;
				if (fabs(up.z) < 0.9)
					right = Point(up.y, -up.x, 0).normalize();
				else
					right = Point(0, up.z, -up.y).normalize();
				
				forward = Point(up.y * right.z - up.z * right.y,
						up.z * right.x - up.x * right.z,
						up.x * right.y - up.y * right.x);
 
               // Transform each vertex from world space to eye space
      
                for (const auto& tri : templateMesh) {
                    MCTriangle newTri;                  
                    for (int v = 0; v < 3; v++) {
                        Point tv = tri.vertices[v];                       
                        Point rotated = Point(
								tv.x * right.x + tv.y * forward.x + tv.z * up.x,
								tv.x * right.y + tv.y * forward.y + tv.z * up.y,
								tv.x * right.z + tv.y * forward.z + tv.z * up.z);						
                        Point worldVertex = rockCenter+rotated*size;
						newTri.vertices[v] = worldVertex - xpoint; // Convert to eye space
                    }                   
                    // Calculate normal in eye space from the 3 eye-space vertices
                    Point edge1 = newTri.vertices[1] - newTri.vertices[0];
                    Point edge2 = newTri.vertices[2] - newTri.vertices[0];
                    Point eyeNormal = edge2.cross(edge1);
                    newTri.normal = eyeNormal.normalize();
                    rock->mesh.push_back(newTri);
                }
                rock->meshValid = true;
                rock->worldPosition = eyePos;
                
                // Upload VBO
                if (smooth && useVertexDisplacement)
                     rock->uploadToVBODisplaced();
                else if (smooth) 
                    rock->uploadToVBOSmooth();
                else 
                    rock->uploadToVBO();                
                Rock3DMgr::setStats(resolution, rock->mesh.size());
            }
        }
        
        // Cull old cache entries
        for (auto it = rockCache.begin(); it != rockCache.end(); ) {
            if (it->second.framesSinceUsed > 5) {
                it = rockCache.erase(it);
            } else {
                ++it;
            }
        }
        
        // Cache size limit
        const size_t MAX_CACHE_SIZE = 5000;
        if (rockCache.size() > MAX_CACHE_SIZE) {
            std::vector<std::pair<RockCacheKey, int>> ages;
            for (auto& pair : rockCache) {
                ages.push_back({pair.first, pair.second.framesSinceUsed});
            }
            std::sort(ages.begin(), ages.end(), 
                      [](auto& a, auto& b) { return a.second > b.second; });
            
            size_t toRemove = rockCache.size() - MAX_CACHE_SIZE;
            for (size_t i = 0; i < toRemove; i++) {
                rockCache.erase(ages[i].first);
            }
            std::cout << "Cache limit reached - removed " << toRemove << " oldest entries" << std::endl;
        }

        std::cout << "Rock cache: " << hits << " hits, " << misses << " misses, " 
                  << regens << " regens, " << rockCache.size() << " cached" << std::endl;
        
        Rock3DMgr::printStats();
        vbo_valid = true;
    }

    render_objects();
 
}
//-------------------------------------------------------------
// Rock3DObjMgr::render() create and render the 3d rocks
//-------------------------------------------------------------
void Rock3DObjMgr::render_objects() {
    bool wireframe = test7;

    if(!PlaceObjMgr::shadow_mode){
    	if (!setProgram()) {
    	  cout << "Rock3DObjMgr::setProgram FAILED" << endl;
    	  return;
    	}
    }
    if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	const std::vector<MCObject*>& rockList = rocks.getObjects();
	for (MCObject *rock : rockList) {
		if (rock->vboValid && rock->mesh.size() > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, rock->vboVertices);
			glVertexPointer(3, GL_FLOAT, 0, 0);
			glEnableClientState(GL_VERTEX_ARRAY);

			glBindBuffer(GL_ARRAY_BUFFER, rock->vboNormals);
			glNormalPointer(GL_FLOAT, 0, 0);
			glEnableClientState(GL_NORMAL_ARRAY);

			glDrawArrays(GL_TRIANGLES, 0, rock->mesh.size() * 3);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
		}
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//************************************************************
// TNrocks3D class
//************************************************************
TNrocks3D::TNrocks3D(int t,TNode *l, TNode *r, TNode *b) : TNrocks(t|MCROCKS,l,r,b)
{
	mgr=new Rock3DMgr(type);
	rock=0;
}

//-------------------------------------------------------------
// TNrocks3D::init() initialize the node
//-------------------------------------------------------------
void TNrocks3D::init()
{
	Rock3DMgr *rmgr=(Rock3DMgr*)mgr;
	rmgr->init();
	TNplacements::init();
	TNarg &args=*((TNarg *)left);

	if(args[7]){
		TNarg *tamp=args[6];
		tamp->eval();
		rmgr->rnoise=args[7];			
	}
	if(rock==0)
		rock=new Rock3D(type,this);	
	
	TNode *tc=findChild(ID_POINT);
	if(tc)
		rmgr->vnoise=tc;
	else
		rmgr->vnoise=0;
	mgr->set_first(1);

}

//-------------------------------------------------------------
// TNrocks3D::eval() evaluate the node
//-------------------------------------------------------------
void TNrocks3D::eval()
{
	TerrainData ground;
	static int cnt=0;
	
	if(!isEnabled() || TheScene->viewtype !=SURFACE){
		if(right)
			right->eval();
		return;
	}
	SINIT;
	if(CurrentScope->rpass()){
		int layer=inLayer()?Td.tp->type():0; // layer id
		int instance=Td.tp->Rocks.objects();
		mgr->instance=instance;
		mgr->layer=layer;
		if(rock){
			rock->set_id(instance);
			rock->layer=layer;
		}
		Td.tp->Rocks.addObject(rock);
		mgr->setHashcode();
		if(right)
			right->eval();
		return;
	}
	if(right)
		right->eval();
	
	if(!CurrentScope->spass()){
		ground.copy(S0);
	}
	INIT;

	mgr->type=type;

	mgr->getArgs((TNarg *)left);
	
	MaxSize=mgr->maxsize;
	
	double density=mgr->density;

	if(density>0)
		mgr->eval();  // calls PlantPoint.set_terrain (need MapPt)
	
	if(!CurrentScope->spass()){ // adapt pass only
		S0.copy(ground); // restore S0.p.z etc
		mgr->setTests(); // set S0.c S0.s (density)
	}
}

//------------------- 2D Rocks ------------------------------

//************************************************************
// Rock class
//************************************************************
Rock::Rock(PlacementMgr&m, Point4DL&p,int n) : Placement(m,p,n)
{
}
//-------------------------------------------------------------
// Rock::set_terrain()	impact terrain 
//-------------------------------------------------------------
bool Rock::set_terrain(PlacementMgr &pmgr)
{
	double r,z,rm=0;
	RockMgr &mgr=(RockMgr&)pmgr;
	
	mgr.pdist=1;
	if(radius==0)
		return false;
	
	double d=pmgr.mpt.distance(center);
	
	double thresh=mgr.noise_ampl;
	double td=mgr.drop*mgr.maxsize;
	double t=1.75*radius;

	r=radius;

	if(d>t)
		return false;

	if(pmgr.testColor())
		Placement::set_terrain(pmgr);
	
	mgr.pdist=d/radius;
	mgr.pdist=clamp(mgr.pdist,0,1);

 	if(mgr.noise_ampl>0){
 		double nf=mgr.noise_ampl*radius/Hscale;
 		SPUSH;
		Point4D np;
		if(mgr.offset_valid())
		    np=(mgr.mpt-mgr.offset)*(1/radius);
		else
		    np=(mgr.mpt)*(1/radius);
 		np=np+1/radius;
 		if(TheNoise.noise3D())
 		    np.w=0;
 		TheNoise.push(np);
 		CurrentScope->revaluate();
 		double z=0;
 		mgr.rnoise->eval();
 		if(mgr.rnoise->typeValue()==ID_POINT){
 			z=S0.p.z;
 			mgr.rx=nf*S0.p.x;
 			mgr.ry=nf*S0.p.y; 	
  		}
 		else
 			z=S0.s;
 		TheNoise.pop();
 		rm=0.25*z*mgr.noise_ampl*radius;
 		SPOP;
		d+=rm;
		r-=rm;
 	}
	mgr.rdist=d/r;
	
	d=clamp(d,0,1);
	
	z=mgr.base;
	
	S0.set_flag(ROCKBODY);

	setActive(true);

    z-=0.5*mgr.zcomp*r/Hscale;
	if(r>d)
		z+=(1-mgr.zcomp)*sqrt(r*r-d*d)/Hscale;
    if(z>mgr.ht)
        mgr.ht=z;
    return true;
}

//************************************************************
// RockMgr class
//************************************************************
//	arg[0]  levels   		scale levels
//	arg[1]  maxsize			size of largest craters
//	arg[2]  mult			size multiplier per level
//	arg[3]  density			density or dexpr
//
//	arg[4]  zcomp			z compression factor
//	arg[5]  drop			z drop factor or function
//	arg[6]  noise		    noise amplitude
//	arg[7]  noise_expr		noise function
//-------------------------------------------------------------
TNode *RockMgr::default_noise=0;
RockMgr::RockMgr(int i) : PlacementMgr(i)
{
	MSK_SET(type,PLACETYPE,ROCKS);
	noise_ampl=1;
	zcomp=0.1;
	drop=0.1;
	rnoise=0;
	rdist=0;
	pdist=1;
	rx=ry=0;
#ifdef TEST_ROCKS
    set_testColor(true);
#endif

}
RockMgr::~RockMgr()
{
  	if(finalizer()){
#ifdef DEBUG_PMEM
  		printf("RockMgr::free()\n");
#endif
        DFREE(default_noise);
	}
}

//-------------------------------------------------------------
// RockMgr::make() factory method to make Placement
//-------------------------------------------------------------
Placement *RockMgr::make(Point4DL &p, int n)
{
    return new Rock(*this,p,n);
}

//-------------------------------------------------------------
// RockMgr::init()	initialize global objects
//-------------------------------------------------------------
void RockMgr::init()
{
	if(default_noise==0){
#ifdef DEBUG_PMEM
  		printf("RockMgr::init()\n");
#endif
	   default_noise=(TNode*)TheScene->parse_node((char*)def_rnoise_expr);
	}
	PlacementMgr::init();
}

//************************************************************
// TNrocks class
//************************************************************
TNrocks::TNrocks(int t, TNode *l, TNode *r, TNode *b) : TNplacements(t|ROCKS,l,r,b)
{
    mgr=new RockMgr(type);
	TNarg &args=*((TNarg *)left);
	TNode *arg=args[8];
	if(arg && (arg->typeValue() != ID_CONST))
		mgr->dexpr=arg;
	set_collapsed();
}

//-------------------------------------------------------------
// TNrocks::applyExpr() apply expr value
//-------------------------------------------------------------
void TNrocks::applyExpr()
{
    if(expr){
 		TNrocks* rocks=(TNrocks*)expr;
        DFREE(left);
        left=rocks->left;
        left->setParent(this);
        rocks->init();
		delete mgr;
		type=rocks->type;
		mgr=rocks->mgr;
		rocks->left=0;
		rocks->mgr=0;
		rocks->base=0;
		eval();
		delete rocks;
        expr=0;
    }
    if(base)
        base->applyExpr();
}

//-------------------------------------------------------------
// TNrocks::replaceChild replace content
//-------------------------------------------------------------
NodeIF *TNrocks::replaceChild(NodeIF *c,NodeIF *n)
{
	return TNbase::replaceChild(c,n);
}

//-------------------------------------------------------------
// TNrocks::replaceChild replace content
//-------------------------------------------------------------
NodeIF *TNrocks::replaceNode(NodeIF *c)
{
	((TNrocks*)c)->right=right;
	((TNrocks*)c)->parent=parent;
	parent->replaceChild(this,c);
	delete base;
	base=0;
	TheScene->rebuild_all();
	return c;
}

//-------------------------------------------------------------
// TNrocks::addAfter append x after base if c==this
// - used when adding a TNnode object to a TerrainMgr stack
// - always want to append other objects after Rocks in tree
//-------------------------------------------------------------
NodeIF *TNrocks::addAfter(NodeIF *c,NodeIF *x){
	if(c==this){
		TNode *node=(TNode*)x;
		if(right){
			if(x->linkable()){
    			x->setParent(this);
    			x->addChild(right);
    			right=(TNode*)x;
			}
			else {
				TNadd *node=new TNadd((TNode*)x,right);
				node->setParent(this);
				right=node;
			}
		}
		else{
			right=node;
			right->setParent(this);
		}
	}
	else if(c)
		c->addChild(x);
	else
		addChild(x);
	return x;
}
//-------------------------------------------------------------
// TNrocks::addChild
//-------------------------------------------------------------
NodeIF *TNrocks::addChild(NodeIF *x){
	TNode *node=(TNode*)x;
	//cout<<"rocks::addChild "<<x->typeName()<<end;
	if(collapsed()){
		if(right){
			if(x->linkable()){
    			x->setParent(this);
    			x->addChild(right);
    			right=(TNode*)x;
			}
			else {
				TNadd *node=new TNadd((TNode*)x,right);
				node->setParent(this);
				right=node;
			}
		}
		else{
			right=node;
			right->setParent(this);
		}
	}
	else{
		if(base){
			node=new TNadd(node,base);
			node->setParent(this);
			base=node;
		}
		else{
			base=node;
			base->setParent(this);
		}
	}
	return x;
}

//-------------------------------------------------------------
// TNrocks::init() initialize the node
//-------------------------------------------------------------
void TNrocks::init()
{
	RockMgr *rmgr=(RockMgr*)mgr;
	rmgr->init();
	TNplacements::init();
	TNarg &args=*((TNarg *)left);

	if(args[7]){
		TNarg *tamp=args[6];
		tamp->eval();
		rmgr->noise_ampl=S0.s;
		rmgr->rnoise=args[7];			
	}
	mgr->set_first(1);
	TNplacements::init();
}

void TNrocks::setNoiseExpr(char *s){
//	if(noise){
//		noise->setExpr(s);
//		noise->applyExpr();
//	}
}
void TNrocks::setNoiseAmpl(double d){
}

TNnoise *TNrocks::getNoiseExpr(){
	TNarg &args=*((TNarg *)left);
	TNarg *a=args[7];
	if(a)
		return (TNnoise*)a;
	else 
		return nullptr;
}
double TNrocks::getNoiseAmpl(){
	TNarg &args=*((TNarg *)left);
	TNarg *a=args[7];
	
	if(a){
		a->eval();
		return S0.s;
	}
	return 0;
}
//-------------------------------------------------------------
// TNrocks::eval() evaluate the node
//-------------------------------------------------------------
void TNrocks::eval() {
	if (!isEnabled() || TheScene->viewtype != SURFACE) {
		if (right)
			right->eval();
		return;
	}

	TerrainData rock;
	TerrainData ground;
	int i;
	bool first = (right && right->typeValue() != ID_ROCKS);
	bool last = getParent()->typeValue() != ID_ROCKS;
	INIT;
	S0.set_flag(ROCKLAYER);
	int in_map = S0.get_flag(CLRTEXS);

	if (CurrentScope->rpass()) {
		INIT;
		if(right) // ground
			right->eval();
		INIT;
		Td.add_id();
		Td.tp->set_rock(true);
		Td.tp->ntexs=0;
		Td.tp->set_rock(true);
		if(!in_map)
			S0.set_flag(CLRTEXS);
		if(base)// rock texs
			base->eval();
		if(!in_map)// in case we were in another map on entry
			S0.clr_flag(CLRTEXS);
		mgr->setHashcode();
		return;
	}

	if (!in_map && first)
		Td.begin();
	ground.p.z = 0;

	INIT;
	bool other_rock = false;
	if (right)
		right->eval(); // ground
	other_rock = S0.get_flag(ROCKBODY);
	S0.p.z-=Drop;
	if (first) {
		S0.next_id();
		Td.insert_strata(S0);
	}
	ground.copy(S0);
	INIT;

	RockMgr *rmgr = (RockMgr*) mgr;
	
	mgr->getArgs((TNarg *)left);
	INIT;
	if (base)
		base->eval();
	//if (!S0.pvalid())
	S0.p.z=ground.p.z;
	rmgr->base = S0.p.z-rmgr->drop*rmgr->maxsize/Hscale;
	S0.next_id();
	rock.copy(S0);
	INIT;
	rmgr->ht = mgr->base;
	rmgr->eval();  // calls set_terrain sets mgr->ht

	if (rmgr->noise_ampl)
		CurrentScope->revaluate();
	rock.p.z = rmgr->ht;
	double delta = (rock.p.z - ground.p.z) / fabs(ground.p.z);
	if (delta > 0) {
		rock.p.x = rmgr->rx * (1 - rmgr->rdist);
		rock.p.y = rmgr->ry * (1 - rmgr->rdist);
		S0.copy(rock);
		Td.tp->set_rock(true);
		S0.set_flag(ROCKBODY);
	}
	else {
		Td.tp->set_rock(false);
		S0.copy(ground);
		if(!other_rock)
			S0.clr_flag(ROCKBODY);
	}
	Td.insert_strata(rock);
	if (!in_map && last)
		Td.end();
}

//-------------------------------------------------------------
// TNrocks::hasChild return true if child exists
//-------------------------------------------------------------
static int find_type=0;
static NodeIF *child;
static void findType(NodeIF *obj)
{
	if(obj->typeValue()==find_type){
		child=obj;
		obj->setFlag(NODE_STOP);
	}
}
NodeIF *TNrocks::findChild(int type){
	find_type=type;
	child=0;
	if(base)
		base->visitNode(findType);
	return child;
}

// called by VtxSceneDialog ->scene->makeObject
// called by Scene->makeObject
// this = prototype obj=parent(layer) m=GN_TYPE
// this->setParent(obj) already set
// VtxSceneDialog ->addtoTree(this)
NodeIF *TNrocks::getInstance(NodeIF *obj, int m){	
	return newInstance(m);
}
bool TNrocks::randomize(){
	double f=0.2;
	TNarg *arg=(TNarg*)left;
	char buff[1024];
	buff[0]=0;
	arg=arg->index(1);
	int i=0;
	while(arg){
		if(arg->left->typeValue()==ID_CONST){
			TNconst *val=(TNconst*)arg->left;
			//cout<<s[i]<<" before:"<<val->value;
			val->value*=(1+f*s[i]);
			//cout<<" after:"<<val->value<<endl;
		}
		else if(arg->left->typeValue()==ID_NOISE){
			TNnoise *val=(TNnoise*)arg->left;
			buff[0]=0;
			val->valueString(buff);
			//cout<<"before:"<<buff<<endl;
			std::string str=TNnoise::randomize(buff,f,1);
			TNnoise *newval=TheScene->parse_node(str.c_str());
			arg->left=newval;
			//cout<<"after:"<<str<<endl;
			delete val;
		}
		i++;
     	arg=arg->next();
	}
	return true;
}

// this=prototype, this->parent=layer
TNrocks *TNrocks::newInstance(int m){
	NodeIF::setRands();
	int gtype=m&GN_TYPES;
	Planetoid *orb=(Planetoid *)getOrbital(this);
	Planetoid::makeLists();
	std::string str=Planetoid::newRocks(orb,gtype);
	TNrocks *rocks=TheScene->parse_node(str.c_str());
	rocks->setParent(parent);
	rocks->randomize();
	return rocks;
}
