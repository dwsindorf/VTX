#include "MCObjects.h"
#include <algorithm>
#include <cstring>
#include <map>
#include "ViewFustrum.h"  // Add at top with other includes


static double maxpts=0;
static double minpts=1e6;
static double maxdist=0;
static double mindist=100;

//=============================================================================
// External lookup tables (defined in CubeTables.cpp)
//=============================================================================

extern const int MC_edgeTable[256];
extern const int MC_triTable[256][16];

//=============================================================================
// MCGenerator implementation
//=============================================================================

int MCGenerator::cells=0;

Point MCGenerator::interpolateVertex(const Point& p1, const Point& p2, 
                                     double val1, double val2, double isolevel) {
    if (std::abs(isolevel - val1) < 0.00001) return p1;
    if (std::abs(isolevel - val2) < 0.00001) return p2;
    if (std::abs(val1 - val2) < 0.00001) return p1;
    
    double mu = (isolevel - val1) / (val2 - val1);
    return Point(
        p1.x + mu * (p2.x - p1.x),
        p1.y + mu * (p2.y - p1.y),
        p1.z + mu * (p2.z - p1.z)
    );
}

void MCGenerator::addTriangle(const Point& v1, const Point& v2, const Point& v3, 
                              std::vector<MCTriangle>& triangles) {
    MCTriangle tri;
    tri.vertices[0] = v1;
    tri.vertices[1] = v2;
    tri.vertices[2] = v3;
    
    // SET templatePos for texture coordinates (needed for triplanar mapping)
    tri.templatePos[0] = v1;
    tri.templatePos[1] = v2;
    tri.templatePos[2] = v3;
    
    // Initialize colors to white (will be modulated later if color node exists)
    tri.colors[0] = Color(1, 1, 1);
    tri.colors[1] = Color(1, 1, 1);
    tri.colors[2] = Color(1, 1, 1);
    
    // Calculate face normal
    Point edge1 = tri.vertices[1] - tri.vertices[0];
    Point edge2 = tri.vertices[2] - tri.vertices[0];
    tri.normal = Point(
        edge1.y * edge2.z - edge1.z * edge2.y,
        edge1.z * edge2.x - edge1.x * edge2.z,
        edge1.x * edge2.y - edge1.y * edge2.x
    ).normalize();
    
    // Initialize faceNormal same as normal for now
    tri.faceNormal = tri.normal;
    
    triangles.push_back(tri);
}

void MCGenerator::generateTrianglesForCube(int cubeIndex, const Point vertList[12], 
                                           std::vector<MCTriangle>& triangles) {
    for (int i = 0; MC_triTable[cubeIndex][i] != -1; i += 3) {
        if (i + 2 < 16 && MC_triTable[cubeIndex][i+1] != -1 && MC_triTable[cubeIndex][i+2] != -1) {
            addTriangle(vertList[MC_triTable[cubeIndex][i]], 
                       vertList[MC_triTable[cubeIndex][i+1]], 
                       vertList[MC_triTable[cubeIndex][i+2]], 
                       triangles);
        }
    }
}

std::vector<MCTriangle> MCGenerator::generateMesh(
    SurfaceFunction field,
    const Point& boundsMin, const Point& boundsMax,
    int resolution, double isolevel) {
    
    std::vector<MCTriangle> triangles;
    
    double stepX = (boundsMax.x - boundsMin.x) / resolution;
    double stepY = (boundsMax.y - boundsMin.y) / resolution;
    double stepZ = (boundsMax.z - boundsMin.z) / resolution;
    
    // Edge vertex indices for interpolation
    int edgeVertices[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };
    
    // Pre-compute corner values for efficiency
    int gridSize = resolution + 1;
    std::vector<double> cornerGrid(gridSize * gridSize * gridSize);
    
    for (int x = 0; x <= resolution; x++) {
        for (int y = 0; y <= resolution; y++) {
            for (int z = 0; z <= resolution; z++) {
                double px = boundsMin.x + x * stepX;
                double py = boundsMin.y + y * stepY;
                double pz = boundsMin.z + z * stepZ;
                int idx = x * gridSize * gridSize + y * gridSize + z;
                cornerGrid[idx] = field(px, py, pz);
            }
        }
    }
    
    // Process each cube
    for (int x = 0; x < resolution; x++) {
        for (int y = 0; y < resolution; y++) {
            for (int z = 0; z < resolution; z++) {
                double px = boundsMin.x + x * stepX;
                double py = boundsMin.y + y * stepY;
                double pz = boundsMin.z + z * stepZ;
                
                // 8 corners of this cube
                Point cubeVertices[8] = {
                    Point(px, py, pz),
                    Point(px + stepX, py, pz),
                    Point(px + stepX, py + stepY, pz),
                    Point(px, py + stepY, pz),
                    Point(px, py, pz + stepZ),
                    Point(px + stepX, py, pz + stepZ),
                    Point(px + stepX, py + stepY, pz + stepZ),
                    Point(px, py + stepY, pz + stepZ)
                };
                
                // Get corner values from pre-computed grid
                double cubeValues[8];
                cubeValues[0] = cornerGrid[x * gridSize * gridSize + y * gridSize + z];
                cubeValues[1] = cornerGrid[(x+1) * gridSize * gridSize + y * gridSize + z];
                cubeValues[2] = cornerGrid[(x+1) * gridSize * gridSize + (y+1) * gridSize + z];
                cubeValues[3] = cornerGrid[x * gridSize * gridSize + (y+1) * gridSize + z];
                cubeValues[4] = cornerGrid[x * gridSize * gridSize + y * gridSize + (z+1)];
                cubeValues[5] = cornerGrid[(x+1) * gridSize * gridSize + y * gridSize + (z+1)];
                cubeValues[6] = cornerGrid[(x+1) * gridSize * gridSize + (y+1) * gridSize + (z+1)];
                cubeValues[7] = cornerGrid[x * gridSize * gridSize + (y+1) * gridSize + (z+1)];
                
                // Determine cube index
                int cubeIndex = 0;
                for (int i = 0; i < 8; i++) {
                    if (cubeValues[i] < isolevel) {
                        cubeIndex |= (1 << i);
                    }
                }
                
                // Skip if cube is entirely inside or outside
                if (MC_edgeTable[cubeIndex] == 0) {
                    continue;
                }
                
                // Interpolate vertices on active edges
                Point vertList[12];
                for (int i = 0; i < 12; i++) {
                    if (MC_edgeTable[cubeIndex] & (1 << i)) {
                        int v1 = edgeVertices[i][0];
                        int v2 = edgeVertices[i][1];
                        vertList[i] = interpolateVertex(
                            cubeVertices[v1], cubeVertices[v2], 
                            cubeValues[v1], cubeValues[v2], isolevel
                        );
                    }
                }
                
                // Generate triangles for this cube
                generateTrianglesForCube(cubeIndex, vertList, triangles);
            }
        }
    }
    
    return triangles;
}

// Add these method implementations:

std::vector<MCTriangle> MCGenerator::generateAdaptiveMesh(
    SurfaceFunction field,
    const Point& rockCenter,
    double rockRadius,
    const Point& cameraPos,
    double wscale,
    double minPixels,
    double maxDepth,  
    double margin)
{
    std::vector<OctreeCell> leafCells;
    
    OctreeCell root;
    root.center = rockCenter;
    root.size = rockRadius * 2.0 * margin;
    root.depth = 0;

    cells=0;
    maxpts=0;
    minpts=1e10;
    maxdist=0;
    mindist=1e6;

    subdivideOctree(field, root, leafCells, rockCenter, rockRadius,
                   cameraPos, wscale, maxDepth, minPixels);

    double minCellSize = root.size;
    for (const auto& cell : leafCells)
        minCellSize = std::min(minCellSize, cell.size);
    double snapSize = (minCellSize / (rockRadius * 2.0)) * 0.01;  // 1% of finest cell in local space

    
    std::cout << "  leafs=" << leafCells.size() << " non-leafs=" << cells << std::endl;
    std::cout << "  minpts:" << minpts << " maxpts:" << maxpts 
              << " ratio:" << maxpts/minpts 
              << " mindist:" << mindist << " maxdist:" << maxdist 
              << " ratio:" << maxdist/mindist << std::endl;

 
    // ===== Generate mesh for each leaf cell =====
    std::vector<MCTriangle> mesh;
    const int LEAF_RESOLUTION = 1;
    const double overlapFactor = 1.0;

    for (const auto& cell : leafCells) {
        double expandedSize = cell.size * overlapFactor;
        double h = expandedSize / 2.0;
        
        Point expandedMin(cell.center.x - h, cell.center.y - h, cell.center.z - h);
        Point expandedMax(cell.center.x + h, cell.center.y + h, cell.center.z + h);
        
        Point localMin = (expandedMin - rockCenter) / (rockRadius * 2);
        Point localMax = (expandedMax - rockCenter) / (rockRadius * 2);
        
        auto cellMesh = generateMesh(field, localMin, localMax, LEAF_RESOLUTION);
        mesh.insert(mesh.end(), cellMesh.begin(), cellMesh.end());
    }
    std::cout << "  Total triangles: " << mesh.size() << std::endl;  
    
    return mesh;
}

void MCGenerator::subdivideOctree(
    SurfaceFunction field,
    const OctreeCell& cell,
    std::vector<OctreeCell>& leafCells,
    const Point& rockCenter,
    double rockRadius,
    const Point& cameraPos,
    double wscale,
    double maxDepth,
	double minPixels
    )
{
    // Convert cell to local space for surface intersection test
    Point localCenter = (cell.center - rockCenter) / (rockRadius*2);
    double localSize = cell.size/(rockRadius*2);
    
    // Create local-space cell for surface check
    OctreeCell localCell;
    localCell.center = localCenter;
    localCell.size = localSize;
    localCell.depth = cell.depth;
    
    // Check if surface passes through this cell
    if (!checkSurfaceIntersection(field, localCell, 0.0)) {
        return;  // Skip empty cells
    }
    double rockCenterDist = rockCenter.distance(cameraPos);
    
    double distance = cell.center.distance(cameraPos);
    
    double projectedPixels = wscale * cell.size / (distance);
    maxpts=std::max(maxpts,projectedPixels);
    minpts=std::min(minpts,projectedPixels);
    maxdist=std::max(maxdist,distance);
    mindist=std::min(mindist,distance);
   
    // Should we subdivide?
    bool should_subdivide = (projectedPixels > minPixels*2) && (cell.depth < maxDepth);
  
    if (!should_subdivide) {
         // This is a leaf cell
        leafCells.push_back(cell);
        return;
    }
    cells++;
    // Subdivide into 8 children
    double childSize = cell.size / 2.0;
    double offset = childSize / 2.0;
    
    for (int i = 0; i < 8; i++) {
        OctreeCell child;
        child.size = childSize;
        child.depth = cell.depth + 1;
        
        // Calculate child center in world space (8 octants)
        child.center.x = cell.center.x + ((i & 1) ? offset : -offset);
        child.center.y = cell.center.y + ((i & 2) ? offset : -offset);
        child.center.z = cell.center.z + ((i & 4) ? offset : -offset);
        
        subdivideOctree(field, child, leafCells, rockCenter, rockRadius,
                       cameraPos, wscale, maxDepth, minPixels);
    }
}

bool MCGenerator::checkSurfaceIntersection(SurfaceFunction field, 
                                          const OctreeCell& cell, 
                                          double isolevel)
{
    // Sample 8 corners
    double h = cell.size / 2;
    double cx = cell.center.x, cy = cell.center.y, cz = cell.center.z;
    
    Point corners[8] = {
        Point(cx-h, cy-h, cz-h), Point(cx+h, cy-h, cz-h),
        Point(cx+h, cy+h, cz-h), Point(cx-h, cy+h, cz-h),
        Point(cx-h, cy-h, cz+h), Point(cx+h, cy-h, cz+h),
        Point(cx+h, cy+h, cz+h), Point(cx-h, cy+h, cz+h)
    };
    
    double values[8];
    double minVal = 1e10, maxVal = -1e10;
    for (int i = 0; i < 8; i++) {
        values[i] = field(corners[i].x, corners[i].y, corners[i].z);
        minVal = std::min(minVal, values[i]);
        maxVal = std::max(maxVal, values[i]);
    }
    if (minVal <= isolevel && maxVal >= isolevel) return true;
    
    // All corners on same side - check edges with bisection
    // to catch thin surface features between corners
    int edgePairs[12][2] = {
        {0,1},{1,2},{2,3},{3,0},  // bottom face
        {4,5},{5,6},{6,7},{7,4},  // top face
        {0,4},{1,5},{2,6},{3,7}   // vertical edges
    };
    
    const int BISECT_STEPS = 8;  // 2^4 = 16 samples per edge
    for (int e = 0; e < 12; e++) {
        Point p1 = corners[edgePairs[e][0]];
        Point p2 = corners[edgePairs[e][1]];
        double v1 = values[edgePairs[e][0]];
        double v2 = values[edgePairs[e][1]];
        
        // Sample along edge at multiple points
        for (int s = 1; s < BISECT_STEPS; s++) {
            double t = s / (double)BISECT_STEPS;
            Point p = p1 + (p2 - p1) * t;
            double v = field(p.x, p.y, p.z);
            minVal = std::min(minVal, v);
            maxVal = std::max(maxVal, v);
            if (minVal <= isolevel && maxVal >= isolevel) return true;
        }
    }
    
    // Also sample center
    double centerVal = field(cx, cy, cz);
    minVal = std::min(minVal, centerVal);
    maxVal = std::max(maxVal, centerVal);
    
    return (minVal <= isolevel && maxVal >= isolevel);
}
//=============================================================================
// MCObject implementation
//=============================================================================

MCObject::MCObject() 
    : worldPosition(0, 0, 0), baseSize(1.0), 
      distanceToViewer(0), screenProjectedSize(0),
      meshValid(false), lastResolution(-1),
      vboVertices(0), vboNormals(0), vboFaceNormals(0),vboTemplatePos(0), 
	  vboColors(0), vboValid(false) {
	instanceId=0;
	dataIndex=0;
}

MCObject::MCObject(const Point& pos, double size) 
    : worldPosition(pos), baseSize(size), 
      distanceToViewer(0), screenProjectedSize(0),
      meshValid(false), lastResolution(-1),
      vboVertices(0), vboNormals(0),vboTemplatePos(0), vboFaceNormals(0),
	  vboColors(0),vboValid(false) {
	instanceId=0;
	dataIndex=0;

}

MCObject::~MCObject() {
    deleteVBO();
}

void MCObject::setPosition(const Point& pos) {
    if ((worldPosition - pos).length() > baseSize * 0.1) {
        invalidate();  // Significant movement invalidates cache
    }
    worldPosition = pos;
}

void MCObject::setDistanceInfo(double distance, double projectedSize) {
    distanceToViewer = distance;
    screenProjectedSize = projectedSize;
}

void MCObject::invalidate() {
    meshValid = false;
    vboValid = false;
}

void MCObject::clearMesh() {
    mesh.clear();
    meshValid = false;
    vboValid = false;
}

void MCObject::uploadToVBO() {
    if (mesh.empty()) return;
    
    // Delete old VBOs
     if (vboVertices != 0) glDeleteBuffers(1, &vboVertices);
     if (vboNormals != 0) glDeleteBuffers(1, &vboNormals);
     if (vboFaceNormals != 0) glDeleteBuffers(1, &vboFaceNormals);
     if (vboColors != 0) glDeleteBuffers(1, &vboColors);
     if (vboTemplatePos != 0) glDeleteBuffers(1, &vboTemplatePos);
    
    // Prepare vertex and normal arrays
    size_t vertexCount = mesh.size() * 3;
    std::vector<float> vertices(vertexCount * 3);
    std::vector<float> normals(vertexCount * 3);
    std::vector<float> facenormals(vertexCount * 3);
    std::vector<float> colors(vertexCount * 3);
    std::vector<float> templatePositions(vertexCount * 3);  // ADD THIS
    
    for (size_t i = 0; i < mesh.size(); i++) {
        for (int v = 0; v < 3; v++) {
            size_t idx = (i * 3 + v) * 3;
            vertices[idx + 0] = (float)mesh[i].vertices[v].x;
            vertices[idx + 1] = (float)mesh[i].vertices[v].y;
            vertices[idx + 2] = (float)mesh[i].vertices[v].z;
            
            normals[idx + 0] = (float)mesh[i].normal.x;
            normals[idx + 1] = (float)mesh[i].normal.y;
            normals[idx + 2] = (float)mesh[i].normal.z;
 
            facenormals[idx + 0] = (float)mesh[i].faceNormal.x;
            facenormals[idx + 1] = (float)mesh[i].faceNormal.y;
            facenormals[idx + 2] = (float)mesh[i].faceNormal.z;

            colors[idx + 0] = (float)mesh[i].colors[v].red();
            colors[idx + 1] = (float)mesh[i].colors[v].green();
            colors[idx + 2] = (float)mesh[i].colors[v].blue();
            
            templatePositions[idx + 0] = (float)mesh[i].templatePos[v].x;
            templatePositions[idx + 1] = (float)mesh[i].templatePos[v].y;
            templatePositions[idx + 2] = (float)mesh[i].templatePos[v].z;
        }
    }
    
    // Create and upload VBOs
    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vboFaceNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboFaceNormals);
    glBufferData(GL_ARRAY_BUFFER, facenormals.size() * sizeof(float), facenormals.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vboColors);  // ADD THIS
    glBindBuffer(GL_ARRAY_BUFFER, vboColors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vboTemplatePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboTemplatePos);
    glBufferData(GL_ARRAY_BUFFER, templatePositions.size() * sizeof(float), 
                 templatePositions.data(), GL_STATIC_DRAW);
   
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    vboValid = true;
}

void MCObject::deleteVBO() {
    if (vboVertices != 0) {
        glDeleteBuffers(1, &vboVertices);
        vboVertices = 0;
    }
    if (vboNormals != 0) {
        glDeleteBuffers(1, &vboNormals);
        vboNormals = 0;
    }
    if (vboFaceNormals != 0) {
        glDeleteBuffers(1, &vboFaceNormals);
        vboFaceNormals = 0;
    }
    if (vboColors != 0) {
        glDeleteBuffers(1, &vboColors);
        vboColors = 0;
    }
    if (vboTemplatePos != 0) {
         glDeleteBuffers(1, &vboTemplatePos);
         vboTemplatePos = 0;
    } 
    vboValid = false;
}

void MCObject::generateSmoothNormals() {
    if (mesh.empty()) return;
    
    // Hash vertices in TEMPLATE SPACE (not world space)
    double snapSize = 0.001;
    auto hashVertex = [snapSize](const Point& v) -> uint64_t {
        long long ix = (long long)round(v.x / snapSize);
        long long iy = (long long)round(v.y / snapSize);
        long long iz = (long long)round(v.z / snapSize);
        
        uint64_t hx = ((uint64_t)(ix & 0x1FFFFF));
        uint64_t hy = ((uint64_t)(iy & 0x1FFFFF));
        uint64_t hz = ((uint64_t)(iz & 0x1FFFFF));
        
        return (hx << 42) | (hy << 21) | hz;
    };
    
    // Build vertex-to-triangles mapping using templatePos
    std::unordered_map<uint64_t, std::vector<size_t>> vertexToTriangles;
    for (size_t i = 0; i < mesh.size(); i++) {
        for (int v = 0; v < 3; v++) {
            uint64_t key = hashVertex(mesh[i].templatePos[v]);  // Use templatePos!
            vertexToTriangles[key].push_back(i);
        }
    }
    
    // Compute smoothed normals with angle threshold
    const double SMOOTH_ANGLE_COS = cos(90.0 * M_PI / 180.0);
    std::unordered_map<uint64_t, Point> normalAccum;
    
    for (auto& pair : vertexToTriangles) {
        uint64_t key = pair.first;
        std::vector<size_t>& tris = pair.second;
        
        Point referenceNormal = mesh[tris[0]].normal;
        Point avgNormal = referenceNormal;
        int count = 1;
        
        for (size_t i = 1; i < tris.size(); i++) {
            Point normal = mesh[tris[i]].normal;
            // Only average if angle is within threshold
            if (referenceNormal.dot(normal)< SMOOTH_ANGLE_COS) {
                avgNormal = avgNormal + normal;
                count++;
            }
        }
        
        normalAccum[key] = avgNormal / count;
    }
    
    // Normalize accumulated normals
    for (auto& pair : normalAccum) {
        double len = pair.second.length();
        if (len > 1e-8) {
            pair.second = pair.second.normalize();
        } else {
            pair.second = Point(0, 1, 0);
        }
    }
    
    // Store per-vertex smooth normals in a temporary structure
    // We can't modify triangle normals directly since each vertex needs its own
    // Instead, store in a map that uploadToVBO will use
    std::unordered_map<uint64_t, Point> vertexNormals;
    for (auto& tri : mesh) {
        for (int v = 0; v < 3; v++) {
            uint64_t key = hashVertex(tri.templatePos[v]);
            auto it = normalAccum.find(key);
            if (it != normalAccum.end()) {
                vertexNormals[key] = it->second;
            }
        }
    }
    
    // Note: We need to modify uploadToVBO to use per-vertex normals
    // For now, just update the triangle normals with averaged value
    for (auto& tri : mesh) {
        Point smoothNormal(0, 0, 0);
        int count = 0;
        for (int v = 0; v < 3; v++) {
            uint64_t key = hashVertex(tri.templatePos[v]);
            auto it = normalAccum.find(key);
            if (it != normalAccum.end()) {
                smoothNormal = smoothNormal + it->second;
                count++;
            }
        }
        if (count > 0) {
            tri.normal = (smoothNormal / count).normalize();
        }
    }
    
    vboValid = false;
}

void MCObject::generateSphereNormals()
{
    // Generate radial normals for sphere-like objects
    for (auto& tri : mesh) {
        // Calculate geometric face normal
        Point edge1 = tri.vertices[1] - tri.vertices[0];
        Point edge2 = tri.vertices[2] - tri.vertices[0];
        tri.faceNormal = edge1.cross(edge2).normalize();
        
        // For sphere normals: average position of triangle vertices
        Point triCenter = (tri.vertices[0] + tri.vertices[1] + tri.vertices[2]) / 3.0;
        
        // Normal points radially outward from object center
        tri.normal = (triCenter - worldPosition).normalize();
    }
}

const std::vector<MCTriangle>& MCObject::generateMeshAdaptive(
    SurfaceFunction field,
    const Point& rockCenter,
    double rockRadius,
    const Point& cameraPos,
    double wscale,
	double min_pts,
	double max_depth,
	double margin)
{
    MCGenerator generator;
    
    mesh = generator.generateAdaptiveMesh(
        field,
        rockCenter,
        rockRadius,
        cameraPos,
        wscale,
        min_pts,            // Stop when cells < 2 pixels
        max_depth,              // Max depth
        margin
    );
    
    meshValid = true;
    lastResolution = -1;
    
    generateSphereNormals();
    
    return mesh;
}
//=============================================================================
// MCObjectManager implementation
//=============================================================================

MCObjectManager::MCObjectManager() 
    : currentIsoLevel(0.0) {
    // Default field: simple sphere
    currentField = MCFields::sphere;
}

MCObjectManager::~MCObjectManager() {
    clear();
}

void MCObjectManager::setField(SurfaceFunction field) {
    currentField = field;
    // Invalidate all meshes since field changed
    for (auto* obj : objects) {
        obj->invalidate();
    }
}

void MCObjectManager::setIsoLevel(double iso) {
    if (std::abs(iso - currentIsoLevel) > 0.001) {
        currentIsoLevel = iso;
        // Invalidate all meshes since iso level changed
        for (auto* obj : objects) {
            obj->invalidate();
        }
    }
}

MCObject* MCObjectManager::addObject(const Point& position, double size) {
    // Check if already exists
    MCObject* obj = new MCObject(position, size);
    objects.push_back(obj);
    return obj;
}
MCObject* MCObjectManager::addObject(const std::string& name, const Point& position, double size) {
    // Check if already exists
    auto it = objectMap.find(name);
    if (it != objectMap.end()) {
        return it->second;
    }
  
    MCObject* obj = new MCObject(position, size);
    objects.push_back(obj);
    objectMap[name] = obj;
    return obj;
}

MCObject* MCObjectManager::getObject(const std::string& name) {
    auto it = objectMap.find(name);
    return (it != objectMap.end()) ? it->second : nullptr;
}

void MCObjectManager::removeObject(const std::string& name) {
    auto it = objectMap.find(name);
    if (it != objectMap.end()) {
        MCObject* obj = it->second;
        objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
        delete obj;
        objectMap.erase(it);
    }
}

void MCObjectManager::clear() {
    for (auto* obj : objects) {
        delete obj;
    }
    objects.clear();
    objectMap.clear();
}

//=============================================================================
// Built-in field functions
//=============================================================================

namespace MCFields {

double sphere(double x, double y, double z) {
    double distance = sqrt(x*x + y*y + z*z);
    return 1.0 - distance;  // Positive inside unit sphere
}

double cube(double x, double y, double z) {
    double size = 1.0;
    if (std::abs(x) < size && std::abs(y) < size && std::abs(z) < size) {
        return 1.0;
    }
    return -1.0;
}

SurfaceFunction makeNoisySphere(const Point& center, int seed) {
    // Capture center and seed by value
    return [center, seed](double x, double y, double z) -> double {
        // Static noise generator per seed
        static std::unordered_map<int, PerlinNoise> noiseGenerators;
        if (noiseGenerators.find(seed) == noiseGenerators.end()) {
            noiseGenerators[seed] = PerlinNoise(seed);
        }
        PerlinNoise& noise = noiseGenerators[seed];
        
        // Transform to object-local coordinates
        double objX = x - center.x;
        double objY = y - center.y;
        double objZ = z - center.z;
        
        // Base sphere
        double radius = 1.0;
        double distance = sqrt(objX*objX + objY*objY + objZ*objZ);
        double baseSphere = radius - distance;
        
        // Add noise displacement
        double noiseValue = noise.octaveNoise(x, y, z, 4, 1.0, 2.0, 0.5, 0.5);
        
        return baseSphere + noiseValue;
    };
}

} // namespace MCFields

