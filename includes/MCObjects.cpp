#include "MCObjects.h"
#include <algorithm>
#include <cstring>
#include <map>

//=============================================================================
// External lookup tables (defined in CubeTables.cpp)
//=============================================================================

extern const int MC_edgeTable[256];
extern const int MC_triTable[256][16];

//=============================================================================
// MCGenerator implementation
//=============================================================================
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
    tri.vertices[1] = v3;
    tri.vertices[2] = v2;
    
    // Calculate face normal
    Point edge1 = tri.vertices[1] - tri.vertices[0];
    Point edge2 = tri.vertices[2] - tri.vertices[0];
    tri.normal = Point(
        edge1.y * edge2.z - edge1.z * edge2.y,
        edge1.z * edge2.x - edge1.x * edge2.z,
        edge1.x * edge2.y - edge1.y * edge2.x
    ).normalize();
    
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
void MCObject::uploadToVBODisplaced(bool computeSmoothNormals) {
    if (mesh.empty()) return;
            
    // Delete old VBOs
    if (vboVertices != 0) glDeleteBuffers(1, &vboVertices);
    if (vboNormals != 0) glDeleteBuffers(1, &vboNormals);
    if (vboFaceNormals != 0) glDeleteBuffers(1, &vboFaceNormals);
    if (vboColors != 0) glDeleteBuffers(1, &vboColors);
    if (vboTemplatePos != 0) glDeleteBuffers(1, &vboTemplatePos);
    
    std::unordered_map<uint64_t, Point> normalAccum;
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
    if (computeSmoothNormals) {
        // Build vertex-to-triangles mapping
        std::unordered_map<uint64_t, std::vector<int>> vertexToTriangles;      
        for (size_t i = 0; i < mesh.size(); i++) {
            for (int v = 0; v < 3; v++) {
                uint64_t key = hashVertex(mesh[i].templatePos[v]);
                vertexToTriangles[key].push_back(i);
            }
        }  
#ifdef DEBUG_AVERAGING
        int minTris = 1000, maxTris = 0;
        double totalTris = 0;
        for (auto& pair : vertexToTriangles) {
            int count = pair.second.size();
            minTris = std::min(minTris, count);
            maxTris = std::max(maxTris, count);
            totalTris += count;
        }
        std::cout << "Vertex sharing: avg=" << (totalTris / vertexToTriangles.size())
                  << " min=" << minTris << " max=" << maxTris << std::endl;
#endif
        // Compute smoothed normals with angle threshold
        const double SMOOTH_ANGLE_COS = cos(80.0 * M_PI / 180.0);
        for (auto& pair : vertexToTriangles) {
            uint64_t key = pair.first;
            std::vector<int>& tris = pair.second;           
            Point referenceNormal = mesh[tris[0]].normal;           
            Point avgNormal = referenceNormal;
            int count = 1;          
            for (size_t i = 1; i < tris.size(); i++) {
                Point normal = mesh[tris[i]].normal;
                if (referenceNormal.dot(normal) > SMOOTH_ANGLE_COS) {
                    avgNormal = avgNormal + normal;
                    count++;
                }
            }
            normalAccum[key] = avgNormal/count;
        } 
        static Point lastGood=Point(0, 1, 0);
        // Normalize accumulated normals
        for (auto& pair : normalAccum) {
            double len = pair.second.length();
            if (len > 1e-8)
            	lastGood=pair.second.normalize();
            else 
            	cout<<"degenerate normal detected len="<<len<<" x:"<<pair.second.x<<endl;
            pair.second = lastGood;  
        }
    }
    // Build VBO arrays
    size_t vertexCount = mesh.size() * 3;
    std::vector<float> vertices(vertexCount * 3);
    std::vector<float> normals(vertexCount * 3);
    std::vector<float> facenormals(vertexCount * 3);
    std::vector<float> colors(vertexCount * 3);
    std::vector<float> templatePositions(vertexCount * 3);
    
    for (size_t i = 0; i < mesh.size(); i++) {
        for (int v = 0; v < 3; v++) {
            size_t idx = (i * 3 + v) * 3;
            
            vertices[idx + 0] = (float)mesh[i].vertices[v].x;
            vertices[idx + 1] = (float)mesh[i].vertices[v].y;
            vertices[idx + 2] = (float)mesh[i].vertices[v].z;
            
            // Use smooth normals if computed, otherwise use face normals
            Point normal;
            if (computeSmoothNormals) {
                uint64_t key = hashVertex(mesh[i].templatePos[v]);
                auto it = normalAccum.find(key);
                if (it != normalAccum.end()) {
                    normal = it->second;
                } else {
                    // Fallback to face normal if hash lookup fails
                    normal = mesh[i].normal;
                }
            } else {
                normal = mesh[i].normal;
            }
            
            normals[idx + 0] = (float)normal.x;
            normals[idx + 1] = (float)normal.y;
            normals[idx + 2] = (float)normal.z;
            
            colors[idx + 0] = (float)mesh[i].colors[v].red();
            colors[idx + 1] = (float)mesh[i].colors[v].green();
            colors[idx + 2] = (float)mesh[i].colors[v].blue();
            
            templatePositions[idx + 0] = (float)mesh[i].templatePos[v].x;
            templatePositions[idx + 1] = (float)mesh[i].templatePos[v].y;
            templatePositions[idx + 2] = (float)mesh[i].templatePos[v].z;
            
            facenormals[idx + 0] = (float)mesh[i].faceNormal.x;
            facenormals[idx + 1] = (float)mesh[i].faceNormal.y;
            facenormals[idx + 2] = (float)mesh[i].faceNormal.z;

        }
    }
    // Upload VBOs
    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
 
    glGenBuffers(1, &vboFaceNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboFaceNormals);
    glBufferData(GL_ARRAY_BUFFER, facenormals.size() * sizeof(float), facenormals.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vboColors);
    glBindBuffer(GL_ARRAY_BUFFER, vboColors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &vboTemplatePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboTemplatePos);
    glBufferData(GL_ARRAY_BUFFER, templatePositions.size() * sizeof(float), 
                 templatePositions.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    vboValid = true;
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

// Add to MCObject
void MCObject::generateSphereNormals() {
    if (mesh.empty()) return;
    
    for (auto& tri : mesh) {
        Point centroid = (tri.vertices[0] + tri.vertices[1] + tri.vertices[2]) * (1.0/3.0);
        tri.normal = (centroid - worldPosition).normalize();
    }
    
    vboValid = false;
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

