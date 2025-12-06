#ifndef MCOBJECTS_H
#define MCOBJECTS_H

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
#include <cmath>
#include <unordered_map>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//=============================================================================
// Core data structures
//=============================================================================

struct MCTriangle {
    Point vertices[3];
    Point normal;
};

// Scalar field function type - returns density at (x,y,z)
typedef std::function<double(double, double, double)> SurfaceFunction;

//=============================================================================
// MCGenerator - Core marching cubes algorithm
//=============================================================================

class MCGenerator {
private:
    Point interpolateVertex(const Point& p1, const Point& p2, 
                           double val1, double val2, double isolevel);
    void addTriangle(const Point& v1, const Point& v2, const Point& v3, 
                    std::vector<MCTriangle>& triangles);
    void generateTrianglesForCube(int cubeIndex, const Point vertList[12], 
                                  std::vector<MCTriangle>& triangles);

public:
    // Generate mesh for a bounding volume
    // resolution = number of voxels per axis
    std::vector<MCTriangle> generateMesh(
        SurfaceFunction field,
        const Point& boundsMin, const Point& boundsMax,
        int resolution, double isolevel = 0.0);
};

//=============================================================================
// MCObject - A single marching cubes object placed on terrain
//=============================================================================

class MCObject {
public:
    // Object placement info (provided by your terrain system)
    Point worldPosition;      // Center position on terrain
    double baseSize;          // Object's base size in world units
    
    // Distance-based LOD info (computed or provided)
    double distanceToViewer;
    double screenProjectedSize;  // Optional: if your system provides this
    
    // Generated mesh data
    std::vector<MCTriangle> mesh;
    bool meshValid;
    int lastResolution;
    
    // Optional: OpenGL VBO handles for rendering
    GLuint vboVertices;
    GLuint vboNormals;
    bool vboValid;
    
    MCObject();
    MCObject(const Point& pos, double size);
    ~MCObject();
    
    // Core interface
    void setPosition(const Point& pos);
    void setDistanceInfo(double distance, double projectedSize = 0.0);
    
    // Generate mesh with LOD based on distance
    const std::vector<MCTriangle>& generateMesh(
        SurfaceFunction field, 
        double isolevel = 0.0);
    
    // Calculate appropriate resolution based on distance
    int calculateResolution() const;
    
    // Invalidate cached mesh (call when object moves significantly)
    void invalidate();
    
    // Clear mesh data
    void clearMesh();
    
    // Upload mesh to VBOs for rendering
    void uploadToVBO();
    void deleteVBO();
    
    // Optional: smooth normals across shared vertices
    void generateSmoothNormals();
    void MCObject::generateSphereNormals();
};

//=============================================================================
// MCObjectManager - Manages collection of MC objects on terrain
//=============================================================================

class MCObjectManager {
private:
    std::vector<MCObject*> objects;
    std::unordered_map<std::string, MCObject*> objectMap;
    
    // Current field function and iso level
    SurfaceFunction currentField;
    double currentIsoLevel;
    
    // LOD configuration
    struct LODConfig {
        double nearDistance;      // Distance for max detail
        double farDistance;       // Distance for min detail
        int maxResolution;        // Resolution at near distance
        int minResolution;        // Resolution at far distance
        double hysteresis;        // Prevents LOD thrashing (e.g., 1.1 = 10%)
    } lodConfig;

public:
    MCObjectManager();
    ~MCObjectManager();
    
    // Configuration
    void setField(SurfaceFunction field);
    void setIsoLevel(double iso);
    void setLODConfig(double nearDist, double farDist, 
                      int maxRes = 32, int minRes = 4);
    
    // Object management
    MCObject* addObject(const std::string& name, const Point& position, double size);
    MCObject* getObject(const std::string& name);
    void removeObject(const std::string& name);
    void clear();
    
    // Bulk operations
    size_t getObjectCount() const { return objects.size(); }
    const std::vector<MCObject*>& getObjects() const { return objects; }
    
    // Update all objects based on viewer position
    void updateFromViewpoint(const Point& viewpoint);
    
    // Generate/update meshes for all objects
    void generateAllMeshes();
    
    // Upload all meshes to VBOs
    void uploadAllToVBO();
    
    // Get LOD config for external use
    const LODConfig& getLODConfig() const { return lodConfig; }
};

//=============================================================================
// Built-in scalar field functions for common shapes
//=============================================================================

namespace MCFields {
    // Sphere centered at origin with radius 1
    double sphere(double x, double y, double z);
    
    // Noisy sphere (asteroid/rock shape) - creates unique shape per center/seed
    SurfaceFunction makeNoisySphere(const Point& center, int seed = 42);
    
    // Cube (sharp edges)
    double cube(double x, double y, double z);
}

#endif // MCOBJECTS_H
