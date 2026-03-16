#include "MCObjects.h"
#include <algorithm>
#include <cstring>
#include <map>
#include "ViewClass.h"  // Add at top with other includes
#include "RenderOptions.h"

#define DEBUG_ADAPTIVE

//=============================================================================
// External lookup tables (defined in CubeTables.cpp)
//=============================================================================

extern const int MC_edgeTable[256];
extern const int MC_triTable[256][16];


Point MCObjAdaptFlags::rockOrigin;
Point MCObjAdaptFlags::rockRight;
Point MCObjAdaptFlags::rockForward;
Point MCObjAdaptFlags::rockUp;
Point MCObjAdaptFlags::camForward;
Point MCObjAdaptFlags::rotatedCam; 
double MCObjAdaptFlags::frustumPlanes[6][4];
void MCObjAdaptFlags::extractFrustumPlanes() {
    Point xp = TheView->xpoint;

    Point camFwd(TheView->viewMatrix[2], TheView->viewMatrix[6], TheView->viewMatrix[10]);
    auto ndcX = [&](Point worldPos) -> double {
        Point er = worldPos - TheView->xpoint;
        Point p = er.mm(TheView->viewMatrix);
        double z = -(p.x*TheView->lookMatrix[2]+p.y*TheView->lookMatrix[6]+p.z*TheView->lookMatrix[10]);
        if (z <= 0) return 0;
        return (p.x*TheView->projMatrix[0]+p.y*TheView->projMatrix[4]+p.z*TheView->projMatrix[8])/z;
    };
    auto ndcY = [&](Point worldPos) -> double {
        Point er = worldPos - TheView->xpoint;
        Point p = er.mm(TheView->viewMatrix);
        double z = -(p.x*TheView->lookMatrix[2]+p.y*TheView->lookMatrix[6]+p.z*TheView->lookMatrix[10]);
        if (z <= 0) return 0;
        return (p.x*TheView->projMatrix[1]+p.y*TheView->projMatrix[5]+p.z*TheView->projMatrix[9])/z;
    };

    double eps = 1000.0 * FEET;
    Point gx(
        ndcX(rockOrigin+Point(eps,0,0))-ndcX(rockOrigin-Point(eps,0,0)),
        ndcX(rockOrigin+Point(0,eps,0))-ndcX(rockOrigin-Point(0,eps,0)),
        ndcX(rockOrigin+Point(0,0,eps))-ndcX(rockOrigin-Point(0,0,eps))
    );
    Point gy(
        ndcY(rockOrigin+Point(eps,0,0))-ndcY(rockOrigin-Point(eps,0,0)),
        ndcY(rockOrigin+Point(0,eps,0))-ndcY(rockOrigin-Point(0,eps,0)),
        ndcY(rockOrigin+Point(0,0,eps))-ndcY(rockOrigin-Point(0,0,eps))
    );
    gx = gx.normalize();  // screen-right in world space
    gy = gy.normalize();  // screen-up in world space

    Point camRight = gx;
    Point camUp = gy;     
    // Get half-angles from projection matrix
    // For column-major proj: m[0] = 1/(aspect*tan(fovY/2)), m[5] = 1/tan(fovY/2)
    double projX = TheView->projMatrix[0];  // col0,row0
    double projY = TheView->projMatrix[5];  // col1,row1
    // projX = 1/(aspect*tanHalfFov), projY = 1/tanHalfFov
    double tanHalfFovX = fabs(1.0 / projX);
    double tanHalfFovY = fabs(1.0 / projY);

    Point nLeft  = (camFwd - camRight * tanHalfFovX).normalize();
    Point nRight = (camFwd + camRight * tanHalfFovX).normalize();
    Point nBot   = (camFwd + camUp * tanHalfFovY).normalize();
    Point nTop   = (camFwd - camUp * tanHalfFovY).normalize();
    
    // Plane equation: n.dot(p) - n.dot(xp) >= 0  for points inside
    // d = -n.dot(xp)
    frustumPlanes[0][0]=nLeft.x;  frustumPlanes[0][1]=nLeft.y;  frustumPlanes[0][2]=nLeft.z;
    frustumPlanes[0][3]=-nLeft.dot(xp);

    frustumPlanes[1][0]=nRight.x; frustumPlanes[1][1]=nRight.y; frustumPlanes[1][2]=nRight.z;
    frustumPlanes[1][3]=-nRight.dot(xp);

    frustumPlanes[2][0]=nBot.x;   frustumPlanes[2][1]=nBot.y;   frustumPlanes[2][2]=nBot.z;
    frustumPlanes[2][3]=-nBot.dot(xp);

    frustumPlanes[3][0]=nTop.x;   frustumPlanes[3][1]=nTop.y;   frustumPlanes[3][2]=nTop.z;
    frustumPlanes[3][3]=-nTop.dot(xp);
    
//    double d = frustumPlanes[0][0]*rockOrigin.x + frustumPlanes[0][1]*rockOrigin.y
//             + frustumPlanes[0][2]*rockOrigin.z + frustumPlanes[0][3];
//    printf("rockOrigin plane[0] dist=%.2f ft (positive=inside)\n", d/FEET);

}
bool MCObjAdaptFlags::inView(const Point &pos, const Point& camera, double esize = 0.0) {	
	Point eyePos = pos;//-camera;
	Point p = eyePos.mm(TheView->viewMatrix);
	
    double z = -p.mz(TheView->lookMatrix);

	//if (z < TheView->znear - esize)
	//	return false;

	double m = (z > 0) ? esize / z : 0.0;
	double t = 1.0 + m;

	double y = p.my(TheView->projMatrix) / z;
	if (y < -t || y > t)
		return false;

	double x = p.mx(TheView->projMatrix) / z;
	if (x < -t * TheView->aspect || x > t * TheView->aspect)
            return false;

    return true;
}
void MCObjAdaptFlags::setDirections(Point p, Point r, Point f, Point u) {
	rockOrigin = p; rockRight = r; rockForward = f; rockUp = u;

	camForward = Point(
		-TheView->viewMatrix[2],
		-TheView->viewMatrix[6],
		-TheView->viewMatrix[10]
	).normalize();
	extractFrustumPlanes();

}

//=============================================================================
// MCGenerator implementation
//=============================================================================
int MCGenerator::cells=0;
int MCGenerator::leaf_cells=0;
int MCGenerator::csi_calls = 0;
int MCGenerator::cells_deleted=0;
int MCGenerator::cells_created=0;
int MCGenerator::csi_early_exit = 0;
int MCGenerator::csi_edge_exits = 0;
int MCGenerator::csi_cull_calls=0;
int MCGenerator::csi_adapt_calls=0;
int MCGenerator::csi_false = 0;
int MCGenerator::csi_by_depth[32] = {};
int MCGenerator::csi_cull_by_depth[32] = {};
int MCGenerator::tm_field_calls=0; // templates
int MCGenerator::ad_field_calls=0; // adaptive
int MCGenerator::frame_field_calls=0;  // reset each frame
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

bool MCGenerator::smooth(){
	extern int test8;
	return Render.avenorms() && test8;
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
				tm_field_calls++;

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

void MCGenerator::resetStats() {
	cells = leaf_cells = cells_created=cells_deleted=0;
	// Reset stats before subdivideOctree
	csi_calls = csi_early_exit = csi_edge_exits = csi_false = csi_cull_calls=csi_adapt_calls=0;
	memset(csi_by_depth, 0, sizeof(csi_by_depth));  // ← ensure this is present
	memset(csi_cull_by_depth, 0, sizeof(csi_cull_by_depth));  // ← ensure this is present

}
void MCGenerator::printStats(){
	int total_cells=cells+leaf_cells;
	if(total_cells==0)
		return;

   std::cout  << " CSI: calls=" << csi_calls
			  << " EXITS early:" << csi_early_exit
			  << " edge:" << csi_edge_exits  
			  << " empty:" << csi_false
			  << std::endl;
	// Add depth distribution printout:
	cout << " CSI: by depth ";
	for (int d = 0; d <= 10; d++)
		if (csi_by_depth[d] > 0)
			std::cout << "d" << d << ":" << csi_by_depth[d] << " ";
	cout << endl;
	cout << " CULL: by depth ";
	for (int d = 0; d <= 10; d++)
		if (csi_cull_by_depth[d] > 0)
			std::cout << "d" << d << ":" << csi_cull_by_depth[d] << " ";
	cout << endl;
	cout<< " CELLS created:" << cells_created << " deleted:" << cells_deleted<<endl;
	cout<< " Adapt calls:"<<csi_adapt_calls<<" culls:"<<csi_cull_calls<<" "<<100.0*csi_cull_calls/csi_adapt_calls<<"%"<<endl;
}
// Add these method implementations:

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
    const double SMOOTH_ANGLE_COS = cos(60.0 * M_PI / 180.0);
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
            if (referenceNormal.dot(normal)> SMOOTH_ANGLE_COS) {
                avgNormal = avgNormal + normal;
                count++;
            }
        }        
        normalAccum[key] = avgNormal / count;
    }
    
    // Normalize accumulated normals
    for (auto& pair : normalAccum) {
    	static Point last_good;
        double len = pair.second.length();
        if (len > 1e-10) {
            pair.second = pair.second.normalize();
            last_good=pair.second;
        } else {
            pair.second = last_good;//Point(0, 1, 0);
        }
    }
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

//=============================================================================
// MCObjNode implementation
//=============================================================================
MCObjNode::MCObjNode()
    : parent(nullptr),
      center(0,0,0), size(0), depth(0),
      surfaceChecked(false), surfacePresent(false),
      meshValid(false),attributesApplied(false),
      inFrustum(false), projectedSize(0)
{
    memset(children,          0, sizeof(children));
    memset(neighbors,         0, sizeof(neighbors));
    memset(cornerValues,      0, sizeof(cornerValues));
    memset(cornersEvaluated, false, sizeof(cornersEvaluated));
	MCGenerator::cells_created++;
}

MCObjNode::~MCObjNode()
{
    MCGenerator::cells_deleted++;
    collapse();
}

void MCObjNode::collapse()
{
    for (int i = 0; i < 8; i++) {
        if (children[i]) {
            delete children[i];
            children[i] = nullptr;
        }
    }
    meshValid         = false;
    attributesApplied = false;  // ← add
    mesh.clear();
    mesh.shrink_to_fit();
}

void MCObjNode::invalidate()
{
    surfaceChecked  = false;
    meshValid       = false;
    attributesApplied = false;  // ← add
    memset(cornersEvaluated, false, sizeof(cornersEvaluated));
    mesh.clear();
    mesh.shrink_to_fit();
    for (int i = 0; i < 8; i++)
        if (children[i])
            children[i]->invalidate();
}

void MCObjNode::visit(void (MCObjNode::*func)())
{
    (this->*func)();
    if (!isLeaf())
        for (int i = 0; i < 8; i++)
            if (children[i])
                children[i]->visit(func);
}

void MCObjNode::visit_all(void (MCObjNode::*func)())
{
    // visit_all visits every node whether leaf or not
    // mirrors MapNode::visit_all
    (this->*func)();
    for (int i = 0; i < 8; i++)
        if (children[i])
            children[i]->visit_all(func);
}

void MCObjNode::collectLeaves(std::vector<MCObjNode*>& leaves)
{
	if (isLeaf()) {
		if (!mesh.empty()){  // only collect leaves with actual geometry
			if (!attributesApplied)      // only count newly generated leaves
			 	MCGenerator::leaf_cells++;
			leaves.push_back(this);
		}
		return;
	}
	for (int i = 0; i < 8; i++)
		if (children[i])
			children[i]->collectLeaves(leaves);
}


void MCObjNode::split()
{
    if (!isLeaf()) return;

    double childSize = size / 2.0;
    double offset    = childSize / 2.0;

    Point right   = MCObjAdaptFlags::rockRight;
    Point forward = MCObjAdaptFlags::rockForward;
    Point up      = MCObjAdaptFlags::rockUp;

    for (int i = 0; i < 8; i++) {
        MCObjNode* child = new MCObjNode();
        child->parent = this;
        child->depth  = depth + 1;
        child->size   = childSize;
        child->center = Point(
            center.x + ((i & 1) ? offset : -offset),
            center.y + ((i & 2) ? offset : -offset),
            center.z + ((i & 4) ? offset : -offset)
        );

        if (cornersEvaluated[i]) {
            child->cornerValues[i]     = cornerValues[i];
            child->cornersEvaluated[i] = true;
        }

        children[i] = child;
    }

    meshValid = false;
    mesh.clear();
    mesh.shrink_to_fit();
}
bool MCObjNode::checkSurface(SurfaceFunction field,
                              const Point& objCenter, double objRadius, int maxDepth,
                              double isolevel)
{
    if (surfaceChecked)
        return surfacePresent;

    surfaceChecked = true;
    MCGenerator::csi_calls++;
    MCGenerator::csi_by_depth[std::min(depth, 31)]++;

    Point worldCorners[8], corners[8];
    getCorners(worldCorners, corners, objCenter, objRadius);

    double values[8];
    double minVal =  1e10, maxVal = -1e10;
    for (int i = 0; i < 8; i++) {
        if (!cornersEvaluated[i]) {
            cornerValues[i]     = field(corners[i].x, corners[i].y, corners[i].z);
            cornersEvaluated[i] = true;
        }
        values[i] = cornerValues[i];
        minVal = std::min(minVal, values[i]);
        maxVal = std::max(maxVal, values[i]);
    }

    if (minVal <= isolevel && maxVal >= isolevel) {
        surfacePresent = true;
        MCGenerator::csi_early_exit++;
        return true;
    }

    static const int edgePairs[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    double depthFraction = (double)depth / (double)maxDepth;
    int BISECT_STEPS = std::max((int)round(8.0 * (1.0 - depthFraction)), 1);

    for (int e = 0; e < 12; e++) {
        Point p1 = corners[edgePairs[e][0]];
        Point p2 = corners[edgePairs[e][1]];
        for (int s = 1; s < BISECT_STEPS; s++) {
            double t = s / (double)BISECT_STEPS;
            Point  p = p1 + (p2 - p1) * t;
            double v = field(p.x, p.y, p.z);
            minVal = std::min(minVal, v);
            maxVal = std::max(maxVal, v);
            if (minVal <= isolevel && maxVal >= isolevel) {
                surfacePresent = true;
                MCGenerator::csi_edge_exits++;
                return true;
            }
        }
    }

    // Check center
    Point co = center - objCenter;
    Point localCenter = co / objRadius;
    double cv = field(localCenter.x, localCenter.y, localCenter.z);
    minVal = std::min(minVal, cv);
    maxVal = std::max(maxVal, cv);
    if (minVal <= isolevel && maxVal >= isolevel) {
        surfacePresent = true;
        MCGenerator::csi_edge_exits++;
        return true;
    }
    MCGenerator::csi_false++;
    surfacePresent = false;
    return false;
}

void MCObjNode::generateMesh(SurfaceFunction field,
                              const Point& objCenter, double objRadius)
{
    if (meshValid) return;

    Point worldCorners[8], localCorners[8];
    getCorners(worldCorners, localCorners, objCenter, objRadius);

    Point localMin = localCorners[0];
    Point localMax = localCorners[0];
    for (int i = 1; i < 8; i++) {
        localMin.x = std::min(localMin.x, localCorners[i].x);
        localMin.y = std::min(localMin.y, localCorners[i].y);
        localMin.z = std::min(localMin.z, localCorners[i].z);
        localMax.x = std::max(localMax.x, localCorners[i].x);
        localMax.y = std::max(localMax.y, localCorners[i].y);
        localMax.z = std::max(localMax.z, localCorners[i].z);
    }

    MCGenerator gen;
    mesh = gen.generateMesh(field, localMin, localMax, 1);

    meshValid = true;
    if (mesh.empty()) {
        surfacePresent = false;
        surfaceChecked = true;
    }
}
static int cnt=0;
void MCObjNode::adapt(SurfaceFunction field,
                      const Point& objCenter, double objRadius,
                      const Point& cameraPos, double wscale,
                      double minPixels, int maxDepth, const MCObjAdaptFlags& flags)
{
    MCGenerator::csi_adapt_calls++;
 
    
     // ── Projected screen size — use world-space xpoint for correct distance ──
    double distance   = center.distance(cameraPos);
    projectedSize     = wscale * size / distance;
    double effectiveMinPixels = minPixels;
    
    bool culled=false;
    
    bool skipSurfaceCheck = (depth < 2);  // cell larger than rock
    // ── Burial coarsening — use rock-local up axis ───────────────────────
    if (flags.burialCoarsening) {
        double localUp = (center.z - objCenter.z) / objRadius;
        if (localUp < 0) {
            effectiveMinPixels *= 1.0 + (-localUp) * 10.0;
            //culled=true;
        }
    }

    // ── Back-face coarsening — all in rock-local space ───────────────────
    if (flags.backfaceCoarsening) {
        Point localCenter = (center - objCenter) / objRadius;
        Point localCamNorm = (cameraPos - objCenter).normalize();
        double viewDot = localCenter.dot(localCamNorm);
        double thresh = 0.0;
        if (viewDot < thresh) {
            effectiveMinPixels = std::max(effectiveMinPixels,
                minPixels * (1.0 + (-viewDot + thresh) * 10.0));
            //culled=true;
        }
    }
#define USE_DP
#ifdef USE_DP
    if (flags.frustumCulling && depth > 5) {
  		Point camFwd = Point(-TheView->viewMatrix[2], -TheView->viewMatrix[6], -TheView->viewMatrix[10]);
			
		Point offset = center - objCenter;
		Point worldCenter = MCObjAdaptFlags::rockOrigin
						  + MCObjAdaptFlags::rockRight   * offset.x
						  + MCObjAdaptFlags::rockForward * offset.y
						  + MCObjAdaptFlags::rockUp      * offset.z;
		Point toCell = (TheView->xpoint-worldCenter).normalize();
                      
        double dotVal=camFwd.dot(toCell);
        dotVal*=fabs(dotVal);  
        double thresh = lerp(projectedSize, minPixels, 8, 0.99, 0.2);
        culled = (dotVal < thresh);  
        
        //if(culled && cnt%10000==0)
       //   cout<<"depth:"<<depth<<" dotVal:"<<dotVal<<" projected:"<<projectedSize<<" thresh:"<<thresh<<endl;

        if (culled) {
            effectiveMinPixels *= 8.0;
            skipSurfaceCheck = true;
        }
    }
#else
    if (flags.frustumCulling && depth > 5) {
     		Point camFwd = Point(-TheView->viewMatrix[2], -TheView->viewMatrix[6], -TheView->viewMatrix[10]);
       	
          	Point toRock = (TheView->xpoint - MCObjAdaptFlags::rockOrigin).normalize();
    		double dotVal = camFwd.dot(toRock);  // 1=looking directly at rock, 0=90deg away
    		dotVal*=fabs(dotVal);
    		
    		double angleFactor = 1.0 - std::max(0.0, dotVal);  // 0 when facing, 1 when 90deg away
    		double margin = angleFactor * 1.5;  // tunable
      
            Point worldCorners[8], localCorners[8];
            getCorners(worldCorners, localCorners, objCenter, objRadius);

            bool anyInside = false;
            for (int i = 0; i < 8; i++) {
                Point p = (worldCorners[i] - TheView->xpoint).mm(TheView->viewMatrix);
                double z = -p.mz(TheView->lookMatrix);
                if (z < TheView->znear) { anyInside = true; break; }
                double x = p.mx(TheView->projMatrix) / z;
                double y = p.my(TheView->projMatrix) / z;
                if (x >= -(TheView->aspect + margin) && x <= (TheView->aspect + margin) &&
                    y >= -(1.0 + margin) && y <= (1.0 + margin)) {
                    anyInside = true;
                    break;
                }
            }

            culled = !anyInside;
            if (culled) {
            	skipSurfaceCheck=true;
                effectiveMinPixels *= 8;
             }
        }
#endif
    cnt++;
    
    if(culled){
        MCGenerator::csi_cull_calls++;
        MCGenerator::csi_cull_by_depth[depth]++;
    }
    // ── Should this node be a leaf? ──────────────────────────────────────
    bool size_check  = (projectedSize <= effectiveMinPixels * 2);
    bool depth_check = (depth >= maxDepth);
    bool shouldBeLeaf = size_check || depth_check;

    if (!skipSurfaceCheck && !checkSurface(field, objCenter, objRadius, maxDepth)) {
        collapse();
        return;
    }
    if (shouldBeLeaf) {
         if (!isLeaf())
            collapse();
        if (!meshValid)
            generateMesh(field, objCenter, objRadius);
        return;
    }

    // ── Split if currently a leaf ────────────────────────────────────────
    if (isLeaf()) {
        split();
        MCGenerator::cells++;
    }

    // ── Recurse into children ────────────────────────────────────────────
    for (int i = 0; i < 8; i++)
        if (children[i])
            children[i]->adapt(field, objCenter, objRadius,
                               cameraPos, wscale, minPixels, maxDepth, flags);
}
void MCObjNode::getCorners(Point worldCorners[8], Point localCorners[8],
                            const Point& objCenter, double objRadius) const
{
    double h = size / 2.0;
    for (int i = 0; i < 8; i++) {
        // Rotated-space corner
        Point rc(
            center.x + ((i & 1) ? h : -h),
            center.y + ((i & 2) ? h : -h),
            center.z + ((i & 4) ? h : -h)
        );
        // World-space corner
        Point co = rc - objCenter;
        worldCorners[i] = MCObjAdaptFlags::rockOrigin
                        + MCObjAdaptFlags::rockRight   * co.x
                        + MCObjAdaptFlags::rockForward * co.y
                        + MCObjAdaptFlags::rockUp      * co.z;
        // Local-space corner for field evaluation
        localCorners[i] = co / objRadius;
    }
}
//=============================================================================
// MCObjTree implementation
//=============================================================================

MCObjTree::MCObjTree()
    : root(nullptr), instance(0), rval(0),
      radius(0), margin(1.0),
      valid(false), framesSinceUsed(0)
{
}

MCObjTree::~MCObjTree()
{
    free();
}

void MCObjTree::free()
{
    if (root) {
        delete root;
        root = nullptr;
    }
    valid = false;
}

void MCObjTree::invalidate()
{
    // Field params changed — force full rebuild next adapt()
    free();
}

Point MCObjTree::rotateToLocal(const Point p) {
    return Point(
        p.dot(MCObjAdaptFlags::rockRight),
        p.dot(MCObjAdaptFlags::rockForward),
        p.dot(MCObjAdaptFlags::rockUp)
    );
}

void MCObjTree::init(const Point& c, double r, double m,
                     SurfaceFunction f, int inst, int rv)
{
    free();
    center   = c;
    radius   = r;
    margin   = m;
    field    = f;
    instance = inst;
    rval     = rv;

    root         = new MCObjNode();
    root->center = center;
    root->size   = radius * margin;  // full diameter
    root->depth  = 0;

    valid = true;
}

void MCObjTree::adapt(const Point& cameraPos, double wscale,
                      double minPixels, int maxDepth,const MCObjAdaptFlags& flags)
{
    if (!root || !valid) return;
    root->adapt(field, center, radius, rotateToLocal(cameraPos), wscale, minPixels, maxDepth,flags);
    framesSinceUsed = 0;
}

void MCObjTree::collectLeaves(std::vector<MCObjNode*>& leaves)
{
    if (root)
        root->collectLeaves(leaves);
}

void MCObjTree::visit(void (MCObjNode::*func)())
{
    if (root)
        root->visit(func);
}

void MCObjTree::visit_all(void (MCObjNode::*func)())
{
    if (root)
        root->visit_all(func);
}

//=============================================================================
// MCObjTreeMgr implementation
//=============================================================================
MCObjTreeMgr::MCObjTreeMgr(int max)
    : maxTrees(max)
{}

MCObjTreeMgr::~MCObjTreeMgr()
{
    clear();
}

void MCObjTreeMgr::clear()
{
    for (auto& pair : trees) {
        delete pair.second;
    }
    trees.clear();
 
}

void MCObjTreeMgr::invalidateAll()
{
    for (auto& pair : trees)
        pair.second->invalidate();
}

uint64_t MCObjTreeMgr::makeKey(const Point& center, int instance, int rval) const
{
	// Snap to 1/1000th of internal unit — preserves distinctness for small coords
	    const double snap = 1e-10;
	    int64_t ix = (int64_t)round(center.x / snap);
	    int64_t iy = (int64_t)round(center.y / snap);
	    int64_t iz = (int64_t)round(center.z / snap);

	    uint64_t h = 14695981039346656037ULL;
	    auto mix = [&](uint64_t v) { h ^= v; h *= 1099511628211ULL; };

	    mix((uint64_t)ix);
	    mix((uint64_t)iy);
	    mix((uint64_t)iz);
	    mix((uint64_t)(uint32_t)instance);
	    mix((uint64_t)(uint32_t)rval);
	    return h;}

MCObjTree* MCObjTreeMgr::getOrCreate(const Point& center, double radius,
                                      double margin, SurfaceFunction field,
                                      int instance, int rval)
{
    uint64_t key = makeKey(center, instance, rval);
    
    Point rotatedCenter = MCObjTree::rotateToLocal(center);

    auto it = trees.find(key);
    if (it != trees.end()) {
        MCObjTree* tree = it->second;
        // Reinitialize if invalidated (e.g. after mesh_needs_rebuild)
        if (!tree->valid || !tree->root)
            tree->init(rotatedCenter, radius, margin, field, instance, rval);
        else
            tree->framesSinceUsed = 0;
        return tree;
    }

    // Evict before adding if at capacity
    if ((int)trees.size() >= maxTrees)
        evict();

    MCObjTree* tree = new MCObjTree();
    tree->init(rotatedCenter, radius, margin, field, instance, rval);
    trees[key] = tree;
    return tree;
}

void MCObjTreeMgr::evict()
{
    // Age all trees and remove the oldest half
    // Simple strategy: remove any tree not used this frame
    // More sophisticated LRU can be added later

    // First increment age of all trees
    for (auto& pair : trees)
        pair.second->framesSinceUsed++;

    // Remove trees unused for more than N frames
    const int MAX_IDLE_FRAMES = 5;
    auto it = trees.begin();
    while (it != trees.end()) {
        if (it->second->framesSinceUsed > MAX_IDLE_FRAMES) {
            delete it->second;
            it = trees.erase(it);
        } else {
            ++it;
        }
    }
}
