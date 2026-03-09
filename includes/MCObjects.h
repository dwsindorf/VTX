#ifndef MCOBJECTS_H
#define MCOBJECTS_H

#include "ViewFustrum.h"
#include "PerlinNoise.h"
#include "ColorClass.h"

#include <vector>
#include <functional>
#include <cmath>
#include <unordered_map>
#include <map>
#include <string>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define USE_PERSISTENT_TREE

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
    static int tm_field_calls;
    static int ad_field_calls;
    static int frame_field_calls;  // reset each frame
    static int cells;
    static int cells_deleted;
    static int cells_created;
    static int leaf_cells;
    static int csi_calls;
    static int csi_early_exit;
    static int csi_edge_exits;
    static int csi_cull_calls;
    static int csi_adapt_calls;
    static int csi_false;
    static int csi_by_depth[32];
    static double csi_minCellSize;
    double maxdepth;

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

 
    Point interpolateVertex(const Point& p1, const Point& p2,
                            double val1, double val2, double isolevel);
    void addTriangle(const Point& v1, const Point& v2, const Point& v3,
                     std::vector<MCTriangle>& triangles);
    void generateTrianglesForCube(int cubeIndex, const Point vertList[12],
                                  std::vector<MCTriangle>& triangles);

    
    std::vector<MCTriangle> generateMesh(
        SurfaceFunction field,
        const Point& boundsMin, const Point& boundsMax,
        int resolution, double isolevel = 0.0);
#ifndef USE_PERSISTENT_TREE
    void subdivideOctree(
         SurfaceFunction field,
         const OctreeCell& cell,
         std::vector<OctreeCell>& leafCells,
         const Point& rockCenter,
         double rockRadius,
         const Point& cameraPos,
         double wscale,
         double maxDepth,
         double minPixels);

    bool checkSurfaceIntersection(SurfaceFunction field,
                                  const OctreeCell& cell,
                                  double isolevel);
    std::vector<MCTriangle> generateAdaptiveMesh(
        SurfaceFunction field,
        const Point& rockCenter,
        double rockRadius,
        const Point& cameraPos,
        double wscale,
        double minPixels = 50,
        double maxDepth = 8,
        double margin = 1);
#endif
    static void resetStats();
    static void printStats();
};

//=============================================================================
// MCObject - A single marching cubes object placed on terrain
//=============================================================================

class MCObject {
public:
    Point worldPosition;
    double baseSize;
    double distanceToViewer;
    double screenProjectedSize;
    std::vector<MCTriangle> mesh;
    bool meshValid;
    int lastResolution;
    GLuint vboVertices;
    GLuint vboNormals;
    GLuint vboFaceNormals;
    GLuint vboColors;
    GLuint vboTemplatePos;
    bool vboValid;
    int instanceId;
    int dataIndex;

    MCObject();
    MCObject(const Point& pos, double size);
    ~MCObject();

    void setPosition(const Point& pos);
    void setDistanceInfo(double distance, double projectedSize = 0.0);
    const std::vector<MCTriangle>& generateMesh(
        SurfaceFunction field, double isolevel = 0.0);
    int calculateResolution() const;
    void invalidate();
    void clearMesh();
    void uploadToVBO();
    void deleteVBO();
    void generateSmoothNormals();
    void generateSphereNormals();
    const std::vector<MCTriangle>& generateMeshAdaptive(
        SurfaceFunction field,
        const Point& rockCenter,
        double rockRadius,
        const Point& cameraPos,
        double wscale,
        double min_pts = 2,
        double max_depth = 10,
        double margin = 1.0);
};

//=============================================================================
// MCObjectManager - Manages collection of MC objects on terrain
//=============================================================================

class MCObjectManager {
private:
    std::vector<MCObject*> objects;
    SurfaceFunction currentField;
    double currentIsoLevel;

public:
    std::unordered_map<std::string, MCObject*> objectMap;
    MCObjectManager();
    ~MCObjectManager();
    void setField(SurfaceFunction field);
    void setIsoLevel(double iso);
    MCObject* addObject(const Point& position, double size);
    MCObject* addObject(const std::string& name, const Point& position, double size);
    MCObject* getObject(const std::string& name);
    void removeObject(const std::string& name);
    void clear();
    size_t getObjectCount() const { return objects.size(); }
    const std::vector<MCObject*>& getObjects() const { return objects; }
};

//=============================================================================
// Built-in scalar field functions for common shapes
//=============================================================================

namespace MCFields {
    double sphere(double x, double y, double z);
    SurfaceFunction makeNoisySphere(const Point& center, int seed = 42);
    double cube(double x, double y, double z);
}

// In MCObjects.h
struct MCObjAdaptFlags {
    bool burialCoarsening  = true;   // increase minPixels below terrain surface
    bool backfaceCoarsening = true;  // increase minPixels on far side of rock
    bool frustumCulling    = false;  // skip cells outside view frustum (future)
    bool curvatureAdapt    = false;  // finer subdivision at high curvature (future)

    static Point rockOrigin;
    static Point rockRight;
    static Point rockForward;
    static Point rockUp;
    static Point camForward; 
    
    static void setDirections(Point p, Point r, Point f, Point u);
    static bool inView(const Point& worldPos, double esize = 0.0);
    // Presets for common object types
    static MCObjAdaptFlags rock() {
        return MCObjAdaptFlags{false, false, true, false};
    }
    static MCObjAdaptFlags asteroid() {
        return MCObjAdaptFlags{false, true, false, false};  // no burial
    }
    static MCObjAdaptFlags cave() {
        return MCObjAdaptFlags{false, false, false, false}; // interior object
    }
    static MCObjAdaptFlags holes() {
        return MCObjAdaptFlags{true, false, false, false}; // pourous object
    }
};

#ifdef USE_PERSISTENT_TREE
//=============================================================================
// MCObjNode - One cell in a persistent adaptive octree
// Equivalent to MapNode in the 2D terrain system.
// Used for rocks, asteroids, standalone MC objects.
//=============================================================================

class MCObjNode {
public:
    // Tree structure - mirrors MapNode neighbor pointers
    MCObjNode *children[8];  // null if leaf
    MCObjNode *parent;

    // Neighbor connectivity for T-junction stitching (future use)
    // mirrors MapNode lnode/rnode/unode/dnode
    MCObjNode *neighbors[6]; // +x,-x,+y,-y,+z,-z faces

    // Cell geometry (world space)
    Point center;
    double size;
    int depth;

    // Corner field values — shared with adjacent siblings on split.
    // Avoids re-evaluation when neighbors split or merge.
    double cornerValues[8];
    bool cornersEvaluated[8];

    // Cached result of surface intersection test
    // avoids re-running checkSurfaceIntersection when tree hasn't changed
    bool surfaceChecked;
    bool surfacePresent;

    // Generated mesh — only valid for leaf nodes
    std::vector<MCTriangle> mesh;
    bool meshValid;
    bool attributesApplied;  // ← add this

    // Per-frame LOD state — updated each adapt pass
    bool inFrustum;
    double projectedSize;

    MCObjNode();
    ~MCObjNode();

    bool isLeaf() const { return children[0] == nullptr; }

    // Tree operations — mirror MapNode/MapLink interface
    void split();                // create 8 children
    void collapse();             // destroy children, become leaf

    // Traversal — mirrors MapNode::visit()
    void visit(void (MCObjNode::*func)());
    void visit_all(void (MCObjNode::*func)());

    // Collect leaf nodes for mesh generation and rendering
    void collectLeaves(std::vector<MCObjNode*>& leaves);

    // Core per-frame adapt — mirrors MapNode::adapt()
    // Splits or collapses based on viewpoint, culling, LOD

    void adapt(SurfaceFunction field,
               const Point& objCenter, double objRadius,
               const Point& cameraPos, double wscale,
               double minPixels, int maxDepth,
               const MCObjAdaptFlags& flags = MCObjAdaptFlags{});
    
     // Surface intersection using cached corner values where available
    bool checkSurface(SurfaceFunction field,
                      const Point& objCenter, double objRadius,
                      int maxDepth,
                      double isolevel = 0.0);
    // Generate marching cubes mesh for this leaf cell
    void generateMesh(SurfaceFunction field,
                      const Point& objCenter, double objRadius);

    // Invalidate mesh and surface cache (call when field changes)
    void invalidate();
};

//=============================================================================
// MCObjTree - Persistent adaptive octree for one 3D object
// Equivalent to Map in the 2D terrain system.
// Owns the root MCObjNode and manages the tree lifetime.
// Used for: rocks, asteroids, standalone MC objects, future terrain.
//=============================================================================

class MCObjTree {
public:
    MCObjNode *root;

    // Object identity — determines field function and caching
    int instance;   // which manager/type owns this object
    int rval;       // noise seed

    // Cached field function — rebuilt only on invalidate()
    SurfaceFunction field;

    // Object geometry (world space)
    Point center;
    double radius;
    double margin;

    // Tree state
    bool valid;             // false if full rebuild needed
    int framesSinceUsed;    // for LRU eviction in MCObjMgr

    MCObjTree();
    ~MCObjTree();

    // Initialize or reinitialize root node
    void init(const Point& center, double radius, double margin,
              SurfaceFunction field, int instance, int rval);

    // Force full rebuild on next adapt() — call when field params change
    void invalidate();

    // Free entire tree
    void free();

    // Top-level adapt — mirrors Map::adapt()
    // Traverses tree top-down, splits/collapses nodes per viewpoint
    
    void adapt(const Point& cameraPos, double wscale,
               double minPixels, int maxDepth,
               const MCObjAdaptFlags& flags = MCObjAdaptFlags{});

    // Collect all leaf nodes for rendering
    void collectLeaves(std::vector<MCObjNode*>& leaves);

    // Traversal — mirrors Map::visit() / Map::visit_all()
    void visit(void (MCObjNode::*func)());
    void visit_all(void (MCObjNode::*func)());
};

//=============================================================================
// MCObjTreeMgr - Manages a collection of MCObjTree instances
// Equivalent to MCObjectManager but for persistent trees.
// Future: asteroid fields, cave systems, standalone objects.
//=============================================================================

class MCObjTreeMgr {
public:
    // Key: quantized world position + instance + rval
    // mirrors Rock3DObjMgr::rockCache but holds trees not meshes
    std::map<uint64_t, MCObjTree*> trees;

    int maxTrees;   // LRU eviction limit
    static int ad_field_calls;
    static int frame_field_calls;  // reset each frame

    MCObjTreeMgr(int maxTrees = 200);
    ~MCObjTreeMgr();

    // Get existing tree or create new one for this object
    MCObjTree* getOrCreate(const Point& center, double radius,
                           double margin, SurfaceFunction field,
                           int instance, int rval);

    // Invalidate all trees — call when settings change
    void invalidateAll();

    // Remove least recently used trees down to maxTrees
    void evict();

    void clear();

//private:
    // Generate a stable 64-bit key from position + instance
    uint64_t makeKey(const Point& center, int instance, int rval) const;
};
#endif
#endif // MCOBJECTS_H
