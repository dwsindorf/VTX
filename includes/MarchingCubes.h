#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H

#ifdef _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "PointClass.h"
#include "PerlinNoise.h"

#include <vector>
#include <functional>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <stack>
#include <chrono>
#include <cmath>
#include <map>
#include <set>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CULL_CUBES
#define CULL_FLOATERS
#define PRINT_CALC_STATS

class MarchingCubes;
class MarchingCubesObject;
struct Cube {
    Point position;
    double size;
    Cube(Point pos, double s);
};

struct Triangle {
    Point vertices[3];
    Point normal;
};
typedef std::function<double(double, double, double)> SurfaceFunction;

// Add near the top with other structs
struct GenerationStats {
    double totalCornerTime = 0;
    double totalMCTime = 0;
    int patchCount = 0;
    
    void reset() {
        totalCornerTime = 0;
        totalMCTime = 0;
        patchCount = 0;
    }
    
    void add(double cornerTime, double mcTime) {
        totalCornerTime += cornerTime;
        totalMCTime += mcTime;
        patchCount++;
    }
    
    void print() {
#ifdef PRINT_CALC_STATS
        double totalTime = totalCornerTime + totalMCTime;
        if (totalTime == 0) return;
        
        std::cout << "\n=== Generation Summary (all patches) ===" << std::endl;
        std::cout << "Patches processed:          " << patchCount << std::endl;
        std::cout << "Corner evaluation (Perlin): " << totalCornerTime << " ms (" 
                  << (totalCornerTime/totalTime*100) << "%)" << std::endl;
        std::cout << "MC processing (geometry):   " << totalMCTime << " ms (" 
                  << (totalMCTime/totalTime*100) << "%)" << std::endl;
        std::cout << "TOTAL:                      " << totalTime << " ms" << std::endl;
        std::cout << "Average per patch:          " << (totalTime/patchCount) << " ms" << std::endl;
#endif
    }
};
class MarchingCubes {
protected:
    static const int edgeTable[256];
    static const int triTable[256][16];

    Point interpolateVertex(const Point& p1, const Point& p2, double val1, double val2, double isolevel);
    void addTriangle(const Point& v1, const Point& v2, const Point& v3, std::vector<Triangle>& triangles);
    void generateTrianglesForCube(int cubeIndex, const Point vertList[12], std::vector<Triangle>& triangles);
public:
 
    static GenerationStats generationStats;
    
    static Point currentObjectCenter;
    static double isoLevel;
  
    enum FieldType {
        SPHERE, NOISY_SPHERE, CUBE, TERRAIN, INIT
    };
    enum SceneType {
        SINGLE_OBJECT,
        MULTIPLE_OBJECTS,
        OBJECT_GRID
    };

    static FieldType field_type;
    static SceneType scene_type;

    static SurfaceFunction field;
    static bool useSmoothedNormals;
    static bool cullFloaters;
    static double baseResolution;
    static void setField(FieldType);

    std::vector<Triangle> generateMesh(MarchingCubesObject&, double minX, double maxX, double minY, double maxY, double minZ, double maxZ,
                                      int resolutionX, int resolutionY, int resolutionZ, double cubesize, 
                                      double isolevel = 0.0f);
};

class LODParameters {
public:
    double cubeSize;
    int resolution;
    double effectiveDetail;
    void printStats();
};

class LODCalculator {
private:
    static std::map<int, int> s_lodStats;
    
public:
    static double s_terrainDetailScale;
    static double s_fovHorizontal;
    static double s_nearDist;
    static double s_farDist;
   
    static void resetStats();
    static void recordLOD(int resolution);
    static void printStats(int totalPatches);
    static LODParameters calculateLOD(const Point& objectCenter, 
                                         const Point& objectSize, 
                                         const Point& viewpoint,
                                         int currentResolution = -1); 
};

class MarchingCubesObject {
private:
    Point center;
    Point size;
    std::string name;
  
    struct ObjectStats {
        int totalMemory = 0;
        int totalCubes = 0;
        int culledCubes = 0;
        int totalVoxels = 0;
        int culledVoxels = 0;
        int components = 0;
        int largest = 0;
        int floaters = 0;
        int removed = 0;
        int trianglesGenerated = 0;
        int treetime = 0;
        int proctime = 0;
        int runtime = 0;
        int savetime = 0;
    };
    std::vector<Cube> subdivideVolume(const Point& boundsMin, const Point& boundsMax, double cubeSize);
    std::vector<Cube> cullEmptyCubes(const std::vector<Cube>& cubes, double isolevel);
    bool cubeIntersectsSurface(const Cube& cube, double isolevel);
    std::vector<Triangle> generateMeshWithSpatialCulling(const Point& boundsMin, const Point& boundsMax,
                                                        double cubeSize, int resolution, double isolevel);

public:
    ObjectStats stats;
    LODParameters lod;
	bool meshCacheValid;
	int lastResolution;
	double lastIsoValue;
    MarchingCubes::FieldType lastFieldType;
    std::vector<Triangle> cachedMesh;

    MarchingCubesObject::MarchingCubesObject(Point pos, Point sz, std::string nm) 
        : center(pos), size(sz), name(nm), 
          meshCacheValid(false), lastResolution(-1), 
          lastIsoValue(0.0f), lastFieldType(MarchingCubes::INIT){}  // Add this
    
    ~MarchingCubesObject(); 
    std::string getName() const;
    size_t getCacheSize() const;
    Point getCenter() const;
    Point getSize() const;
    const std::vector<Triangle>& generateMesh(const Point& viewpoint);
    void keepLargestComponent(std::vector<Triangle>& triangles);
    void floodFillComponent(size_t startTriangle, std::vector<Triangle>& triangles,
                           std::unordered_map<uint64_t, std::vector<size_t>>& vertexToTriangles,
                           std::vector<bool>& visited, std::vector<size_t>& component);
    void generateSmoothNormals(std::vector<Triangle>& triangles);
    void saveOBJWithNormals(std::vector<Triangle>& triangles, const std::string& filename);
    void printStats();
    void resetStats();
    void clearCache();
    void invalidateMeshCache();

};

class MarchingCubesScene {
private:
    struct ScenePerformanceStats {
        double totalBuildTime = 0.0;
        size_t totalMemoryMB = 0;
        int totalTriangles = 0;
        int totalObjects = 0;
    };
    
    struct PatchInfo {
        Point position;
        int lodLevel;
        MarchingCubesObject* object;
    };
    
    std::map<std::string, MarchingCubesObject*> objectMap;
    Point viewpoint;
    std::map<std::pair<int,int>, std::string> m_activePatches;
    size_t m_totalSceneMemory;
    
public:
    std::vector<MarchingCubesObject*> objects;
    MarchingCubesScene() : m_totalSceneMemory(0){}
    ~MarchingCubesScene();
    
    void invalidate();
   
    void setViewpoint(Point vp);
    Point getViewpoint() const;
    void addObject(Point center, Point size, std::string name);
    std::vector<Triangle> generateScene();
    void saveScene(const std::string& filename, bool includeMarkers = true);
    ScenePerformanceStats getPerformanceStats();
    void removeObject(const std::string &name);
    bool hasObject(const std::string &name) const;
    std::vector<std::string> getObjectNames() const;
    void generateTerrainPatches(const Point& viewPosition, double yaw, double pitch,
                               double fovHorizontal, double nearDist, double farDist);

    void updateTerrainIncremental(const Point& currentPos, double currentYaw, double currentPitch,
                                  const Point& lastPos, double lastYaw, double lastPitch,
                                  double fovHorizontal, double nearDist, double farDist);
};

#endif
