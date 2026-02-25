#ifndef MCOBJECTS_H
#define MCOBJECTS_H

#include "ViewFustrum.h"
#include "PerlinNoise.h"
#include "ColorClass.h"

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
    Point faceNormal;
    Color colors[3];
    Point templatePos[3]; 
};

// Scalar field function type - returns density at (x,y,z)
typedef std::function<double(double, double, double)> SurfaceFunction;

//=============================================================================
// MCGenerator - Core marching cubes algorithm
//=============================================================================

class MCGenerator {
public:
	static int cells;
	
	struct OctreeCell {
		Point center;
		double size;
		int depth;

		Point getMin() const {
			double h = size / 2;
			return Point(center.x - h, center.y - h, center.z - h);
		}

		Point getMax() const {
			double h = size / 2;
			return Point(center.x + h, center.y + h, center.z + h);
		}
	};
	void subdivideOctree(
	        SurfaceFunction field,
	        const OctreeCell& cell,
	        std::vector<OctreeCell>& leafCells,
	        const Point& rockCenter,
	        double rockRadius,
	        const Point& cameraPos,
	        double wscale,
	        int maxDepth,
	        double isolevel,
	        double minPixels);
	
    
    bool checkSurfaceIntersection(SurfaceFunction field, 
                                  const OctreeCell& cell, 
                                  double isolevel);
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
    
    std::vector<MCTriangle> generateAdaptiveMesh(
            SurfaceFunction field,
            const Point& rockCenter,
            double rockRadius,
            const Point& cameraPos,
            double wscale,
            int maxDepth = 8,
            double isolevel = 0.0,
            double minPixels = 2.0);
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
    GLuint vboFaceNormals;
    GLuint vboColors;
    GLuint vboTemplatePos;
    bool vboValid;
    int instanceId;  // Which instance/type this belongs to
    int dataIndex;   // Index into the data array (for texture mapping)
    
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
    void generateSphereNormals();
    const std::vector<MCTriangle>& generateMeshAdaptive(
            SurfaceFunction field,
            const Point& rockCenter,
            double rockRadius,
            const Point& cameraPos,
            double wscale,
			double min_pts=2,
			double max_depth=10,
            double isolevel = 0.0);
};

//=============================================================================
// MCObjectManager - Manages collection of MC objects on terrain
//=============================================================================

class MCObjectManager {
private:
    std::vector<MCObject*> objects;
    
    // Current field function and iso level
    SurfaceFunction currentField;
    double currentIsoLevel;
   
public:
    std::unordered_map<std::string, MCObject*> objectMap;
    MCObjectManager();
    ~MCObjectManager();
    
    // Configuration
    void setField(SurfaceFunction field);
    void setIsoLevel(double iso);
     
    // Object management
    MCObject* addObject(const Point& position, double size);
    MCObject* addObject(const std::string& name, const Point& position, double size);
    MCObject* getObject(const std::string& name);
    void removeObject(const std::string& name);
    void clear();
    
    // Bulk operations
    size_t getObjectCount() const { return objects.size(); }
    const std::vector<MCObject*>& getObjects() const { return objects; }
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
