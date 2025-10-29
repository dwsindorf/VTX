#include "MarchingCubes.h"
#include <fstream>
#include <cmath>
#include <atomic>

//#define PRINT_LOD_STATS
//#define PRINT_OBJ_STATS
//#define SHOW_CULL_TIMING 
//#define TEST
#define ADD_FLOOR
// Cube constructor

static uint64_t hashVertex(const Point& v) {
    double snap = 0.001f;
    int ix = (int)(round(v.x / snap) /snap);
    int iy = (int)(round(v.y / snap) /snap); 
    int iz = (int)(round(v.z / snap) /snap);
    uint64_t key = ((uint64_t)(ix & 0x1FFFFF) << 42) |
                   ((uint64_t)(iy & 0x1FFFFF) << 21) |
                   ((uint64_t)(iz & 0x1FFFFF));
    return key;
}
Cube::Cube(Point pos, double s) : position(pos), size(s) {}


// Scalar field functions
double sphere(double x, double y, double z) {
    double radius = 1.0f;
    double objX = x - MarchingCubes::currentObjectCenter.x;
    double objY = y - MarchingCubes::currentObjectCenter.y;
    double objZ = z - MarchingCubes::currentObjectCenter.z;
    double distance = sqrt(objX*objX + objY*objY + objZ*objZ);
    return radius - distance;
}

double cube(double x, double y, double z) {
    double size = 1.0f;
    double objX = x - MarchingCubes::currentObjectCenter.x;
    double objY = y - MarchingCubes::currentObjectCenter.y;
    double objZ = z - MarchingCubes::currentObjectCenter.z;
    
    if (std::abs(objX) < size && std::abs(objY) < size && std::abs(objZ) < size) {
        return 1.0f;
    } else {
        return -1.0f;
    }
}

double noisySphere(double x, double y, double z) {
    int objectSeed = (int)(MarchingCubes::currentObjectCenter.x * 1000 + 
                          MarchingCubes::currentObjectCenter.y * 1000 + 
                          MarchingCubes::currentObjectCenter.z * 1000) + 42;

    static std::unordered_map<int, PerlinNoise> noiseGenerators;
    if (noiseGenerators.find(objectSeed) == noiseGenerators.end()) {
        noiseGenerators[objectSeed] = PerlinNoise(objectSeed);
    }
    
    PerlinNoise& noise = noiseGenerators[objectSeed];
    double objX = x - MarchingCubes::currentObjectCenter.x;
    double objY = y - MarchingCubes::currentObjectCenter.y;
    double objZ = z - MarchingCubes::currentObjectCenter.z;
    
    int orders = 4;
    double fstart = 1;
    double fdelta = 2.0;
    double adelta = 0.5;
    double f = fstart;
    double a = 0.5;
    
    double radius = 1.0f;
    double baseDistance = sqrt(objX*objX + objY*objY + objZ*objZ);
    double baseSphere = radius - baseDistance;
    
    double noiseValue=noise.octaveNoise(x, y, z, orders, fstart, fdelta, a, adelta);
    return baseSphere + noiseValue;
}


//#define TEST
double terrain3D(double x, double y, double z) {
#ifdef TEST
	return (1.0f - y) * 0.5f;
#else
    static PerlinNoise noise1(12345);
    static PerlinNoise noise2(67890);
#ifdef ADD_FLOOR   
    // Modulate floor height with 2D noise
    double baseFloorHeight = 0.5f;
    //double floorNoise = (noise1.octaveNoise(x * 30.0f, 0, z * 30.0f, 1, 0.5f));
    //double floorNoise = noise1.octaveNoise(x, y*0.1, z, 2, 10.0f,2.0,0.2f,0.5f);
    
    double floorHeight = baseFloorHeight;// + 1e-1*floorNoise;
    
    // Hard cutoff below modulated floor
    if (y < floorHeight) {
        return 1.0f;
    }
#endif   
    double scale1 = 0.3f;
    double scale2 = 0.8f;
     
    double terrain = noise1.octaveNoise(x, y * 0.2f, z, 2, scale1,2.0f,1.0f,0.5f);
    double caves = noise2.octaveNoise(x, y, z, 4, scale2,2.0,1.0,0.3f);

    double density = terrain + caves;
    density += (1.0f - y) * 0.5f;
    
    return density;
#endif
}

// Static variable definitions
SurfaceFunction MarchingCubes::field = &noisySphere;
MarchingCubes::FieldType MarchingCubes::field_type = MarchingCubes::TERRAIN;
MarchingCubes::SceneType MarchingCubes::scene_type = MarchingCubes::OBJECT_GRID;
Point MarchingCubes::currentObjectCenter;
bool MarchingCubes::useSmoothedNormals = false;
bool MarchingCubes::cullFloaters = true;
double MarchingCubes::baseResolution = 8.0f;
double MarchingCubes::isoLevel = 0.0f;
GenerationStats MarchingCubes::generationStats;

std::map<int, int> LODCalculator::s_lodStats;
double LODCalculator::s_terrainDetailScale = 1.0f;
double LODCalculator::s_fovHorizontal = 70.0f * M_PI / 180.0f;
double LODCalculator::s_nearDist = 2.0f;
double LODCalculator::s_farDist = 80.0f;

// Add after the constructor
MarchingCubesObject::~MarchingCubesObject() {
}

void MarchingCubes::setField(FieldType newtype) {
    switch(newtype) {
    case MarchingCubes::SPHERE:
        field = sphere;
        break;
    case MarchingCubes::CUBE:
        field = cube;
        break;
    case MarchingCubes::TERRAIN:
        field = terrain3D;
        break;
    default:
    case MarchingCubes::NOISY_SPHERE:
        field = noisySphere;
        break;
    }
}

Point MarchingCubes::interpolateVertex(const Point& p1, const Point& p2, double val1, double val2, double isolevel) {
    if (std::abs(isolevel - val1) < 0.00001f) return p1;
    if (std::abs(isolevel - val2) < 0.00001f) return p2;
    if (std::abs(val1 - val2) < 0.00001f) return p1;
    
    double mu = (isolevel - val1) / (val2 - val1);
    return Point(
        p1.x + mu * (p2.x - p1.x),
        p1.y + mu * (p2.y - p1.y),
        p1.z + mu * (p2.z - p1.z)
    );
}

void MarchingCubes::addTriangle(const Point& v1, const Point& v2, const Point& v3, std::vector<Triangle>& triangles) {
    Triangle tri;
    tri.vertices[0] = v1;
    tri.vertices[1] = v2;
    tri.vertices[2] = v3;
    
    Point edge1 = v2 - v1;
    Point edge2 = v3 - v1;
    tri.normal = Point(
        edge1.y * edge2.z - edge1.z * edge2.y,
        edge1.z * edge2.x - edge1.x * edge2.z,
        edge1.x * edge2.y - edge1.y * edge2.x
    ).normalize();
    
    triangles.push_back(tri);
}

void MarchingCubes::generateTrianglesForCube(int cubeIndex, const Point vertList[12], std::vector<Triangle>& triangles) {
    for (int i = 0; triTable[cubeIndex][i] != -1; i += 3) {
        if (i + 2 < 16 && triTable[cubeIndex][i+1] != -1 && triTable[cubeIndex][i+2] != -1) {
            addTriangle(vertList[triTable[cubeIndex][i]], 
                       vertList[triTable[cubeIndex][i+1]], 
                       vertList[triTable[cubeIndex][i+2]], 
                       triangles);
        }
    }
}

std::vector<Triangle> MarchingCubes::generateMesh(
    MarchingCubesObject& obj,
    double minX, double maxX, double minY, double maxY, double minZ, double maxZ,
    int resolutionX, int resolutionY, int resolutionZ, double cubesize,
    double isolevel) {

    std::vector<Triangle> triangles;
    
    double stepX = (maxX - minX) / resolutionX;
    double stepY = (maxY - minY) / resolutionY;
    double stepZ = (maxZ - minZ) / resolutionZ;
    
    int edgeVertices[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };
    
    SurfaceFunction fieldFunc = MarchingCubes::field;
    
    auto t1 = std::chrono::high_resolution_clock::now();
    
    int gridSizeX = resolutionX + 1;
    int gridSizeY = resolutionY + 1;
    int gridSizeZ = resolutionZ + 1;
    
    std::vector<double> cornerGrid;
    cornerGrid.resize(gridSizeX * gridSizeY * gridSizeZ);
        
#pragma omp parallel
	{
		SurfaceFunction surface = fieldFunc;
		
		#pragma omp for collapse(3) schedule(dynamic, 8)
		for (int x = 0; x <= resolutionX; x++) {
			for (int y = 0; y <= resolutionY; y++) {
				for (int z = 0; z <= resolutionZ; z++) {
					double px = minX + x * stepX;
					double py = minY + y * stepY;
					double pz = minZ + z * stepZ;
					
					int idx = x * gridSizeY * gridSizeZ + y * gridSizeZ + z;
					cornerGrid[idx] = surface(px, py, pz);
				}
			}
		}
	}
    
    auto t1_5 = std::chrono::high_resolution_clock::now();
    
    // Stage 2: Build cache-friendly per-cube corner array
    int totalCubes = resolutionX * resolutionY * resolutionZ;
    std::vector<double> allCornerValues(totalCubes * 8);
    
    #pragma omp parallel for collapse(3) schedule(static)
    for (int x = 0; x < resolutionX; x++) {
        for (int y = 0; y < resolutionY; y++) {
            for (int z = 0; z < resolutionZ; z++) {
                int cubeIdx = (x * resolutionY * resolutionZ) + (y * resolutionZ) + z;
                int base = cubeIdx * 8;
                
                // Copy 8 corners for this cube
                allCornerValues[base+0] = cornerGrid[x * gridSizeY * gridSizeZ + y * gridSizeZ + z];
                allCornerValues[base+1] = cornerGrid[(x+1) * gridSizeY * gridSizeZ + y * gridSizeZ + z];
                allCornerValues[base+2] = cornerGrid[(x+1) * gridSizeY * gridSizeZ + (y+1) * gridSizeZ + z];
                allCornerValues[base+3] = cornerGrid[x * gridSizeY * gridSizeZ + (y+1) * gridSizeZ + z];
                allCornerValues[base+4] = cornerGrid[x * gridSizeY * gridSizeZ + y * gridSizeZ + (z+1)];
                allCornerValues[base+5] = cornerGrid[(x+1) * gridSizeY * gridSizeZ + y * gridSizeZ + (z+1)];
                allCornerValues[base+6] = cornerGrid[(x+1) * gridSizeY * gridSizeZ + (y+1) * gridSizeZ + (z+1)];
                allCornerValues[base+7] = cornerGrid[x * gridSizeY * gridSizeZ + (y+1) * gridSizeZ + (z+1)];
            }
        }
    }
    
    auto t2 = std::chrono::high_resolution_clock::now();
    
    // Stage 3: Marching cubes processing (unchanged)
#pragma omp parallel
    {
        std::vector<Triangle> localTriangles;
        
        #pragma omp for collapse(3) schedule(dynamic, 2)
        for (int x = 0; x < resolutionX; x++) {
            for (int y = 0; y < resolutionY; y++) {
                for (int z = 0; z < resolutionZ; z++) {
                    int cubeIdx = (x * resolutionY * resolutionZ) + (y * resolutionZ) + z;
                    
                    double px = minX + x * stepX;
                    double py = minY + y * stepY;
                    double pz = minZ + z * stepZ;
                    
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
                    
                    double cubeValues[8];
                    for (int i = 0; i < 8; i++) {
                        cubeValues[i] = allCornerValues[cubeIdx * 8 + i];
                    }
                    
                    int cubeIndex = 0;
                    for (int i = 0; i < 8; i++) {
                        if (cubeValues[i] < isolevel) {
                            cubeIndex |= (1 << i);
                        }
                    }
                    
#ifdef CULL_CUBES
                    if (edgeTable[cubeIndex] == 0) {
                        obj.stats.culledVoxels++;
                        continue;
                    }
#endif
                    
                    Point vertList[12];
                    for (int i = 0; i < 12; i++) {
                        if (edgeTable[cubeIndex] & (1 << i)) {
                            int v1 = edgeVertices[i][0];
                            int v2 = edgeVertices[i][1];
                            vertList[i] = interpolateVertex(
                                cubeVertices[v1], cubeVertices[v2], 
                                cubeValues[v1], cubeValues[v2], isolevel
                            );
                        }
                    }
                    
                    generateTrianglesForCube(cubeIndex, vertList, localTriangles);
                }
            }
        }
        
        #pragma omp critical
        {
            triangles.insert(triangles.end(), localTriangles.begin(), localTriangles.end());
        }
    }
    
#ifdef PRINT_CALC_STATS    
    auto t3 = std::chrono::high_resolution_clock::now();
    double cornerTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0;
    double mcTime = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() / 1000.0;
    generationStats.add(cornerTime, mcTime);
#endif    
    return triangles;
}

// LODParameters implementation
void LODParameters::printStats() {
    std::cout << "  VoxelSize: " << effectiveDetail << "  Voxels per cube: " << resolution << "^3 = "
              << (resolution * resolution * resolution) << std::endl;
}

// LODCalculator implementation
void LODCalculator::resetStats() {
    s_lodStats.clear();
}

void LODCalculator::recordLOD(int resolution) {
    s_lodStats[resolution]++;
}

void LODCalculator::printStats(int totalPatches) {
#ifdef PRINT_LOD_STATS
    if (s_lodStats.empty()) return;
    
    std::cout << "LOD Distribution:" << std::endl;
    for (const auto& pair : s_lodStats) {
        std::cout << "  Resolution " << pair.first << ": " 
                  << pair.second << " patches ("
                  << (100.0f * pair.second / totalPatches) << "%)" 
                  << " voxels:"<<pair.second*pair.first*pair.first*pair.first/1000<<" k"<<std::endl;
    }
#endif
}

LODParameters LODCalculator::calculateLOD(const Point& objectCenter, 
                                         const Point& objectSize, 
                                         const Point& viewpoint,
                                         int currentResolution) {
    double distance = (objectCenter - viewpoint).length();
    
    LODParameters lod;
    
    // Use terrain LOD ONLY for terrain field type, regardless of scene type
    if (MarchingCubes::scene_type == MarchingCubes::OBJECT_GRID && 
        MarchingCubes::field_type == MarchingCubes::TERRAIN) {
        // Hysteresis: add 10% buffer if transitioning to lower detail
        double hysteresis = 1.0f;
        if (currentResolution > 0) {
            hysteresis = 1.1f;  // 10% buffer zone
        }
        
        if (distance < 3.0f) {
            lod.cubeSize = 2.5f;
            lod.resolution = 64;
        }
        else if (distance < 10.0f * hysteresis) {
            lod.cubeSize = 3.0f;
            lod.resolution = 32;
        } 
        else if (distance < 25.0f * hysteresis) {
            lod.cubeSize = 6.0f;
            lod.resolution = 16;
        } 
        else if (distance < 50.0f * hysteresis) {
            lod.cubeSize = 12.0f;
            lod.resolution = 8;
        } 
        else {
            lod.cubeSize = 24.0f;
            lod.resolution = 4;
        }

        // Allow detail scale to modulate resolution
        lod.resolution = (int)(lod.resolution * s_terrainDetailScale);
        lod.resolution = std::max(4, std::min(64, lod.resolution));

        lod.effectiveDetail = lod.cubeSize / lod.resolution;

        recordLOD(lod.resolution);
    } else {
        // Non-terrain: Use distance-based scaling with baseResolution
        double cubeSize = 0.25f;
        double referenceDistance = 5.0f;
        int resolution = (int)(MarchingCubes::baseResolution * referenceDistance / distance);
        resolution = std::max(2, std::min(32, resolution));

        lod.cubeSize = cubeSize;
        lod.resolution = resolution;
        lod.effectiveDetail = cubeSize / resolution;
    }
    
    return lod;
}

// MarchingCubesObject implementation

std::string MarchingCubesObject::getName() const { 
    return name; 
}

Point MarchingCubesObject::getCenter() const { 
    return center; 
}

Point MarchingCubesObject::getSize() const { 
    return size; 
}

std::vector<Cube> MarchingCubesObject::subdivideVolume(const Point& boundsMin, const Point& boundsMax, double cubeSize) {
    std::vector<Cube> cubes;
    
    Point center = (boundsMin + boundsMax) * 0.5f;
    Point totalSize = boundsMax - boundsMin;
    
    int cubesPerDim = (int)round(totalSize.x / cubeSize);
    double actualCubeSize = totalSize.x / cubesPerDim;
    
    Point startPos = boundsMin;
    
    for (int x = 0; x < cubesPerDim; x++) {
        for (int y = 0; y < cubesPerDim; y++) {
            for (int z = 0; z < cubesPerDim; z++) {
                Point cubePos = startPos + Point(x * actualCubeSize, y * actualCubeSize, z * actualCubeSize);
                cubes.push_back(Cube(cubePos, actualCubeSize));
            }
        }
    }
    
    return cubes;
}

std::vector<Cube> MarchingCubesObject::cullEmptyCubes(const std::vector<Cube>& cubes, double isolevel) {
    std::vector<Cube> activeCubes;
    stats.culledCubes = 0;
    stats.totalCubes = cubes.size();
    
    for (const Cube& cube : cubes) {
        if (cubeIntersectsSurface(cube, isolevel)) {
            activeCubes.push_back(cube);
        } else {
            stats.culledCubes++;
        }
    }
    return activeCubes;
}

bool MarchingCubesObject::cubeIntersectsSurface(const Cube& cube, double isolevel) {
    Point corners[8] = {
        Point(cube.position.x, cube.position.y, cube.position.z),
        Point(cube.position.x + cube.size, cube.position.y, cube.position.z),
        Point(cube.position.x + cube.size, cube.position.y + cube.size, cube.position.z),
        Point(cube.position.x, cube.position.y + cube.size, cube.position.z),
        Point(cube.position.x, cube.position.y, cube.position.z + cube.size),
        Point(cube.position.x + cube.size, cube.position.y, cube.position.z + cube.size),
        Point(cube.position.x + cube.size, cube.position.y + cube.size, cube.position.z + cube.size),
        Point(cube.position.x, cube.position.y + cube.size, cube.position.z + cube.size)
    };
    
    double tolerance = cube.size * 0.5f;
    bool hasInside = false, hasOutside = false;
    
    // Call field function directly - no cache!
    SurfaceFunction surface = MarchingCubes::field;
    
    for (int i = 0; i < 8; i++) {
        double value = surface(corners[i].x, corners[i].y, corners[i].z);  // ← DIRECT CALL
        if (value < isolevel + tolerance) hasInside = true;
        if (value >= isolevel - tolerance) hasOutside = true;
    }
    
    return hasInside && hasOutside;
}

std::vector<Triangle> MarchingCubesObject::generateMeshWithSpatialCulling(
    const Point& boundsMin, const Point& boundsMax,
    double cubeSize, int resolution, double isolevel) {
    
    auto totalStart = std::chrono::high_resolution_clock::now();
    
    auto subdivStart = std::chrono::high_resolution_clock::now();
    std::vector<Cube> candidateCubes = subdivideVolume(boundsMin, boundsMax, cubeSize);
    auto subdivEnd = std::chrono::high_resolution_clock::now();
    double subdivTime = std::chrono::duration_cast<std::chrono::microseconds>(subdivEnd - subdivStart).count() / 1000.0;

#ifdef CULL_CUBES
    auto cullStart = std::chrono::high_resolution_clock::now();
    std::vector<Cube> activeCubes = cullEmptyCubes(candidateCubes, isolevel);
    auto cullEnd = std::chrono::high_resolution_clock::now();
    double cullTime = std::chrono::duration_cast<std::chrono::microseconds>(cullEnd - cullStart).count() / 1000.0;
#else
    std::vector<Cube> activeCubes = candidateCubes;
    double cullTime = 0;
#endif
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - totalStart);
    stats.treetime = duration.count();
    
    std::vector<Triangle> allTriangles;
    allTriangles.reserve(activeCubes.size() * 2000);
    
    MarchingCubes mc;
    
    for (const Cube& cube : activeCubes) {
        std::vector<Triangle> cubeTriangles = mc.generateMesh(
        	*this,
            cube.position.x, cube.position.x + cube.size,
            cube.position.y, cube.position.y + cube.size,
            cube.position.z, cube.position.z + cube.size,
            resolution, resolution, resolution, cube.size, isolevel);  // Pass GPU corners
            
        allTriangles.insert(allTriangles.end(), cubeTriangles.begin(), cubeTriangles.end());
    }
     
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - totalStart);
    stats.proctime = duration.count() - stats.treetime;
     
    return allTriangles;
}
const std::vector<Triangle>& MarchingCubesObject::generateMesh(const Point& viewpoint) {
	LODParameters newLod = LODCalculator::calculateLOD(center, size, viewpoint, lastResolution);
    
    bool lodChanged = (newLod.resolution != lastResolution);
    bool isoChanged = (MarchingCubes::isoLevel != lastIsoValue);
    bool fieldChanged = (MarchingCubes::field_type != lastFieldType);
    bool neverGenerated = cachedMesh.empty();
        
	// ADD neverGenerated to the condition
	if (meshCacheValid && !lodChanged && !isoChanged && !fieldChanged && !neverGenerated) {
		return cachedMesh;
	}    
    // Cache is invalid, regenerate mesh
    resetStats();
    auto startTime = std::chrono::high_resolution_clock::now();
       
    MarchingCubes::currentObjectCenter = center;

    lod = newLod;
	bool noisy = MarchingCubes::field_type == MarchingCubes::TERRAIN
			|| MarchingCubes::field_type == MarchingCubes::NOISY_SPHERE;
	bool surface = MarchingCubes::field_type == MarchingCubes::TERRAIN
			&& MarchingCubes::scene_type == MarchingCubes::OBJECT_GRID;
    
    double noisePadding = noisy?std::max(0.5, lod.cubeSize * 0.15):0;
    double overlapVoxels = surface?1.5f:0;
    double minOverlap=surface?0:overlapVoxels;
    double maxOverlap=surface?noisePadding:overlapVoxels;
    
    Point paddedSize = size + Point(noisePadding, noisePadding, noisePadding);    
    double overlapAmount = lod.effectiveDetail * overlapVoxels;
    
    Point boundsMin=center - paddedSize * 0.5f - Point(overlapAmount, minOverlap, overlapAmount);
    Point boundsMax=center + paddedSize * 0.5f + Point(overlapAmount, maxOverlap, overlapAmount);
    
    auto triangles = generateMeshWithSpatialCulling(boundsMin, boundsMax, lod.cubeSize, lod.resolution, MarchingCubes::isoLevel);

#ifdef CULL_FLOATERS
    if(MarchingCubes::cullFloaters && noisy)
    	keepLargestComponent(triangles);
#endif

    if (MarchingCubes::useSmoothedNormals) {
        generateSmoothNormals(triangles);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    stats.runtime = duration.count();
    stats.trianglesGenerated = triangles.size();
    stats.totalMemory = stats.trianglesGenerated*sizeof(Triangle);
    
    cachedMesh = std::move(triangles);
    meshCacheValid = true;
    lastResolution = lod.resolution;
    lastIsoValue = MarchingCubes::isoLevel;
    lastFieldType = MarchingCubes::field_type;
        
    return cachedMesh;
}
void MarchingCubesObject::keepLargestComponent(std::vector<Triangle>& triangles) {
	//std::cout<<"culling floaters"<<std::endl;

    if (triangles.empty()) 
    	return;

    std::unordered_map<uint64_t, std::vector<size_t>> vertexToTriangles;
    for (size_t i = 0; i < triangles.size(); i++) {
        for (int v = 0; v < 3; v++) {
            uint64_t key = hashVertex(triangles[i].vertices[v]);  // Use standalone function
            vertexToTriangles[key].push_back(i);
        }
    }

    std::vector<bool> visited(triangles.size(), false);
    std::vector<std::vector<size_t>> components;

    for (size_t i = 0; i < triangles.size(); i++) {
        if (!visited[i]) {
            std::vector<size_t> component;
            floodFillComponent(i, triangles, vertexToTriangles, visited, component);
            components.push_back(component);
        }
    }

    if (components.empty()) return;

    size_t largestIdx = 0;
    stats.removed = 0;
    for (size_t i = 1; i < components.size(); i++) {
        stats.removed += components[i].size();
        if (components[i].size() > components[largestIdx].size()) {
            largestIdx = i;
        }
    }
    stats.components = components.size();
    stats.floaters = components.size() - 1;
    stats.largest = components[largestIdx].size();

    std::vector<Triangle> filteredTriangles;
    for (size_t triIdx : components[largestIdx]) {
        filteredTriangles.push_back(triangles[triIdx]);
    }

    triangles = std::move(filteredTriangles);
}

void MarchingCubesObject::floodFillComponent(
    size_t startTriangle, 
    std::vector<Triangle>& triangles,
    std::unordered_map<uint64_t, std::vector<size_t>>& vertexToTriangles,
    std::vector<bool>& visited, 
    std::vector<size_t>& component) {
    
    std::stack<size_t> stack;
    stack.push(startTriangle);
    visited[startTriangle] = true;

    while (!stack.empty()) {
        size_t current = stack.top();
        stack.pop();
        component.push_back(current);

        for (int v = 0; v < 3; v++) {
            uint64_t key = hashVertex(triangles[current].vertices[v]);
            auto it = vertexToTriangles.find(key);
            if (it != vertexToTriangles.end()) {
                for (size_t neighborIdx : it->second) {
                    if (!visited[neighborIdx]) {
                        visited[neighborIdx] = true;
                        stack.push(neighborIdx);
                    }
                }
            }
        }
    }
}

void MarchingCubesObject::generateSmoothNormals(std::vector<Triangle>& triangles) {
    if (triangles.empty()) return;
    
    std::unordered_map<uint64_t, Point> normalAccumulator;
    
    for (size_t triIdx = 0; triIdx < triangles.size(); triIdx++) {
        Triangle& tri = triangles[triIdx];
        
        Point edge1 = tri.vertices[1] - tri.vertices[0];
        Point edge2 = tri.vertices[2] - tri.vertices[0];
        Point faceNormal = Point(
            edge1.y * edge2.z - edge1.z * edge2.y,
            edge1.z * edge2.x - edge1.x * edge2.z,
            edge1.x * edge2.y - edge1.y * edge2.x
        ).normalize();
        
        for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
        	uint64_t key = hashVertex(tri.vertices[vertIdx]);
            normalAccumulator[key] = normalAccumulator[key] + faceNormal;
        }
    }
    
    for (auto& pair : normalAccumulator) {
        Point& normal = pair.second;
        if (normal.length() > 0.001f) {
            normal = normal.normalize();
        } else {
            normal = Point(0, 1, 0);
        }
    }
    
    for (size_t triIdx = 0; triIdx < triangles.size(); triIdx++) {
        Triangle& tri = triangles[triIdx];
        Point smoothNormal(0, 0, 0);
        
        for (int vertIdx = 0; vertIdx < 3; vertIdx++) {
            uint64_t key = hashVertex(tri.vertices[vertIdx]);
            smoothNormal = smoothNormal + normalAccumulator[key];
        }
        
        tri.normal = smoothNormal.normalize();
    }
}

void MarchingCubesObject::saveOBJWithNormals(std::vector<Triangle>& triangles, const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "w");
    
    if (!file) {
        std::cout << "ERROR: Could not open file: " << filename << std::endl;
        return;
    }
    
    fprintf(file, "# OBJ file with smooth normals\n");
    fprintf(file, "# Generated by Marching Cubes Object\n");
    fprintf(file, "# Triangles: %zu\n", triangles.size());
    
    std::unordered_map<uint64_t, int> vertexIndexMap;
    std::vector<Point> uniqueVertices;
    std::vector<Point> uniqueNormals;
    
    for (size_t t = 0; t < triangles.size(); t++) {
        Triangle& tri = triangles[t];
        for (int v = 0; v < 3; v++) {
        	uint64_t key = hashVertex(tri.vertices[v]);
            if (vertexIndexMap.find(key) == vertexIndexMap.end()) {
                vertexIndexMap[key] = uniqueVertices.size();
                uniqueVertices.push_back(tri.vertices[v]);
                uniqueNormals.push_back(tri.normal);
            }
        }
    }
    
    for (size_t i = 0; i < uniqueVertices.size(); i++) {
        const Point& v = uniqueVertices[i];
        fprintf(file, "v %f %f %f\n", v.x, v.y, v.z);
    }
    
    for (size_t i = 0; i < uniqueNormals.size(); i++) {
        const Point& n = uniqueNormals[i];
        fprintf(file, "vn %f %f %f\n", n.x, n.y, n.z);
    }
    
    for (size_t t = 0; t < triangles.size(); t++) {
        Triangle& tri = triangles[t];
        int idx[3];
        for (int v = 0; v < 3; v++) {
        	uint64_t key = hashVertex(tri.vertices[v]);
            idx[v] = vertexIndexMap[key] + 1;
        }
        
        fprintf(file, "f %d//%d %d//%d %d//%d\n", 
                idx[0], idx[0], idx[1], idx[1], idx[2], idx[2]);
    }
    
    fclose(file);
    
    std::cout << "Saved OBJ with smooth normals: " << uniqueVertices.size() 
              << " vertices, " << triangles.size() << " faces to " << filename << std::endl;
}

void MarchingCubesObject::printStats() {
    std::cout << "=== Object '" << name << "' Stats ===" << std::endl;
    lod.printStats();
    //cache.printStats();
     
    if (stats.totalCubes > 0) {
        std::cout << "  Cubes:     active " << (stats.totalCubes - stats.culledCubes)
                  << " culled:" << stats.culledCubes 
                  << "/" << stats.totalCubes
                  << " (" << 100.0 * stats.culledCubes / stats.totalCubes << " %)" << std::endl;
        int voxels=stats.totalCubes*lod.resolution*lod.resolution*lod.resolution;
        std::cout << "  Voxels:    active " << (voxels - stats.culledVoxels)
                          << " culled:" << stats.culledVoxels 
                          << "/" << voxels
                          << " (" << 100.0 * stats.culledVoxels / voxels << " %)" << std::endl;
    }
    if (stats.floaters > 0) {
        std::cout << "  Floaters:  " << stats.floaters << " Triangles kept: " 
                  << stats.largest << " removed: " << stats.removed << std::endl;
    }        
    std::cout << "  Triangles: " << stats.trianglesGenerated << std::endl;
    std::cout << "  Tm Cubes:  " << stats.treetime / 1e3 << " ms" << std::endl;
    std::cout << "  Tm Voxels: " << stats.proctime / 1e6 << " s" << std::endl;
    std::cout << "  Tm Total:  " << stats.runtime / 1e6 << " s" << std::endl;
}

void MarchingCubesObject::resetStats() { 
    stats = {}; 
}

void MarchingCubesObject::invalidateMeshCache() {
    meshCacheValid = false;
    cachedMesh.clear();
}
// MarchingCubesScene implementation
MarchingCubesScene::~MarchingCubesScene() {
    for (auto obj : objects) {
        delete obj;
    }
    objects.clear();
}

void MarchingCubesScene::setViewpoint(Point vp) { 
    viewpoint = vp; 
}

Point MarchingCubesScene::getViewpoint() const { 
    return viewpoint; 
}

void MarchingCubesScene::addObject(Point center, Point size, std::string name) {
    if (objectMap.find(name) == objectMap.end()) {
        MarchingCubesObject* obj = new MarchingCubesObject(center, size, name);
        objects.push_back(obj);
        objectMap[name] = obj;
    }
}

std::vector<Triangle> MarchingCubesScene::generateScene() {
    auto sceneStart = std::chrono::high_resolution_clock::now();
    
    std::vector<Triangle> allTriangles;
    
    int cachedCount = 0;
    int regeneratedCount = 0;
    double cacheCheckTime = 0;
    double triangleInsertTime = 0;
    double lodCalculationTime = 0;

    // Stage 1: process CPU patches
    for (auto obj : objects) {
        auto objStart = std::chrono::high_resolution_clock::now();
        
        auto lodStart = std::chrono::high_resolution_clock::now();
        LODParameters newLod = LODCalculator::calculateLOD(obj->getCenter(), obj->getSize(), viewpoint);
        auto lodEnd = std::chrono::high_resolution_clock::now();
        lodCalculationTime += std::chrono::duration_cast<std::chrono::microseconds>(lodEnd - lodStart).count() / 1000.0;
        
        bool wasCached = obj->meshCacheValid;
        
        // Check if this object needs regeneration
        bool lodChanged = (newLod.resolution != obj->lastResolution);
        bool isoChanged = (MarchingCubes::isoLevel != obj->lastIsoValue);
        bool fieldChanged = (MarchingCubes::field_type != obj->lastFieldType);
        bool needsRegeneration = !wasCached || lodChanged || isoChanged || fieldChanged;
        
        if (!needsRegeneration) {
            // Use cached mesh
            cachedCount++;
            auto objEnd = std::chrono::high_resolution_clock::now();
            cacheCheckTime += std::chrono::duration_cast<std::chrono::microseconds>(objEnd - objStart).count() / 1000.0;
            
            const std::vector<Triangle>& objTriangles = obj->generateMesh(viewpoint);
            
            auto insertStart = std::chrono::high_resolution_clock::now();
            allTriangles.insert(allTriangles.end(), objTriangles.begin(), objTriangles.end());
            auto insertEnd = std::chrono::high_resolution_clock::now();
            triangleInsertTime += std::chrono::duration_cast<std::chrono::microseconds>(insertEnd - insertStart).count() / 1000.0;
        }
		else {
            regeneratedCount++;
        }
    }
    
    // Stage 3: Generate meshes remaining CPU patches
    for (auto obj : objects) {
        bool wasCached = obj->meshCacheValid;
        
        // Skip if already processed (cached)
        if (wasCached)
        	continue;
        
        const std::vector<Triangle>& objTriangles = obj->generateMesh(viewpoint);
        
#ifdef PRINT_OBJ_STATS
        obj->printStats();
#endif

        auto insertStart = std::chrono::high_resolution_clock::now();
        allTriangles.insert(allTriangles.end(), objTriangles.begin(), objTriangles.end());
        auto insertEnd = std::chrono::high_resolution_clock::now();
        triangleInsertTime += std::chrono::duration_cast<std::chrono::microseconds>(insertEnd - insertStart).count() / 1000.0;
    }
  
    auto sceneEnd = std::chrono::high_resolution_clock::now();
    double totalSceneTime = std::chrono::duration_cast<std::chrono::microseconds>(sceneEnd - sceneStart).count() / 1000.0;
    
    std::cout << "\n=== Scene-Level Breakdown ===" << std::endl;
    std::cout << "Total patches: " << objects.size() << std::endl;
    std::cout << "Cached: " << cachedCount << " patches, Regenerated: " << regeneratedCount<< std::endl; 
    std::cout << "LOD calculations: " << lodCalculationTime << " ms" << std::endl;
    std::cout << "Triangle insertion: " << triangleInsertTime << " ms" << std::endl;
    std::cout << "Total scene time: " << totalSceneTime << " ms" << std::endl;
    
    return allTriangles;
}

void MarchingCubesScene::saveScene(const std::string& filename, bool includeMarkers) {
    auto allTriangles = generateScene();
    
    FILE* file = fopen(filename.c_str(), "w");
    if (!file) {
        std::cout << "ERROR: Could not open file: " << filename << std::endl;
        return;
    }
    
    fprintf(file, "# Multi-object scene generated by Marching Cubes\n");
    fprintf(file, "# Total triangles: %zu\n", allTriangles.size());
    fprintf(file, "# Viewpoint: %f %f %f\n", viewpoint.x, viewpoint.y, viewpoint.z);
    
    fprintf(file, "# Mesh vertices\n");
    for (size_t t = 0; t < allTriangles.size(); t++) {
        Triangle& tri = allTriangles[t];
        for (int v = 0; v < 3; v++) {
            fprintf(file, "v %f %f %f\n", tri.vertices[v].x, tri.vertices[v].y, tri.vertices[v].z);
        }
    }
    
    int meshVertexCount = allTriangles.size() * 3;
    
    if (includeMarkers) {
        fprintf(file, "# Viewpoint marker vertices\n");
        Point vp = viewpoint;
        double size = 0.2f;
        
        fprintf(file, "v %f %f %f\n", vp.x - size, vp.y - size, vp.z - size);
        fprintf(file, "v %f %f %f\n", vp.x + size, vp.y - size, vp.z - size);
        fprintf(file, "v %f %f %f\n", vp.x + size, vp.y + size, vp.z - size);
        fprintf(file, "v %f %f %f\n", vp.x - size, vp.y + size, vp.z - size);
        fprintf(file, "v %f %f %f\n", vp.x - size, vp.y - size, vp.z + size);
        fprintf(file, "v %f %f %f\n", vp.x + size, vp.y - size, vp.z + size);
        fprintf(file, "v %f %f %f\n", vp.x + size, vp.y + size, vp.z + size);
        fprintf(file, "v %f %f %f\n", vp.x - size, vp.y + size, vp.z + size);
    }
    
    if (MarchingCubes::useSmoothedNormals) {
        fprintf(file, "# Normals\n");
        for (size_t t = 0; t < allTriangles.size(); t++) {
            Triangle& tri = allTriangles[t];
            for (int v = 0; v < 3; v++) {
                fprintf(file, "vn %f %f %f\n", tri.normal.x, tri.normal.y, tri.normal.z);
            }
        }
    }
    
    fprintf(file, "# Main mesh faces (%zu triangles)\n", allTriangles.size());
    for (size_t t = 0; t < allTriangles.size(); t++) {
        int base = t * 3 + 1;
        
        if (MarchingCubes::useSmoothedNormals) {
            fprintf(file, "f %d//%d %d//%d %d//%d\n", base, base, base+1, base+1, base+2, base+2);
        } else {
            fprintf(file, "f %d %d %d\n", base, base+1, base+2);
        }
    }
    
    if (includeMarkers) {
        fprintf(file, "# Viewpoint marker faces\n");
        int base = meshVertexCount + 1;
        
        fprintf(file, "f %d %d %d\n", base+0, base+1, base+2);
        fprintf(file, "f %d %d %d\n", base+0, base+2, base+3);
        fprintf(file, "f %d %d %d\n", base+4, base+6, base+5);
        fprintf(file, "f %d %d %d\n", base+4, base+7, base+6);
        fprintf(file, "f %d %d %d\n", base+0, base+4, base+5);
        fprintf(file, "f %d %d %d\n", base+0, base+5, base+1);
        fprintf(file, "f %d %d %d\n", base+2, base+6, base+7);
        fprintf(file, "f %d %d %d\n", base+2, base+7, base+3);
        fprintf(file, "f %d %d %d\n", base+0, base+3, base+7);
        fprintf(file, "f %d %d %d\n", base+0, base+7, base+4);
        fprintf(file, "f %d %d %d\n", base+1, base+5, base+6);
        fprintf(file, "f %d %d %d\n", base+1, base+6, base+2);
    }
    
    fclose(file);
    
    std::cout << "Saved scene with " << objects.size() << " Objects " << allTriangles.size() << " triangles";
    if (includeMarkers) {
        std::cout << " with marker";
    }
    if (MarchingCubes::useSmoothedNormals) {
        std::cout << " with smooth normals";
    }
    std::cout << " to " << filename << std::endl;
}

MarchingCubesScene::ScenePerformanceStats MarchingCubesScene::getPerformanceStats() {
    ScenePerformanceStats sceneStats;
    
    size_t totalPersistentMemory = 0;
    
    for (auto obj : objects) {
        sceneStats.totalBuildTime += obj->stats.runtime / 1e6;
        sceneStats.totalTriangles += obj->stats.trianglesGenerated;
        
        // Persistent memory = cached mesh + small corner cache
        size_t meshMemory = obj->stats.trianglesGenerated * sizeof(Triangle);
        
        totalPersistentMemory += meshMemory;
    }
    sceneStats.totalObjects = objects.size();
    
    sceneStats.totalMemoryMB = totalPersistentMemory;
    
    return sceneStats;
}

void MarchingCubesScene::removeObject(const std::string &name) {
    auto it = objectMap.find(name);
    if (it != objectMap.end()) {
        objects.erase(
            std::remove(objects.begin(), objects.end(), it->second),
            objects.end());
        delete it->second;
        objectMap.erase(it);
    }
}

bool MarchingCubesScene::hasObject(const std::string &name) const {
    return objectMap.find(name) != objectMap.end();
}

std::vector<std::string> MarchingCubesScene::getObjectNames() const {
    std::vector<std::string> names;
    for (const auto& pair : objectMap) {
        names.push_back(pair.first);
    }
    return names;
}
void MarchingCubesScene::generateTerrainPatches(const Point& viewPosition, double yaw, double pitch,
        double fovHorizontal, double nearDist, double farDist) 
{
	double radYaw = yaw * M_PI / 180.0f;
	double radPitch = pitch * M_PI / 180.0f;
	
	std::set<std::string> desiredPatchNames;
	
	struct LODRing {
		double minDist;
		double maxDist;
		double patchSize;
	};
	
	LODRing rings[] = {
		{0.0f,   10.0f,  3.0f},
		{10.0f,  25.0f,  6.0f},
		{25.0f,  50.0f,  12.0f},
		{50.0f,  farDist, 24.0f}
	};
	
	int neighbors[][2] = {{0,0}, {-1,0}, {1,0}, {0,-1}, {0,1}};
	
	for (size_t ringIdx = 0; ringIdx < 4; ringIdx++) {
		const auto& ring = rings[ringIdx];
		
	    int neighborPattern = (ring.patchSize <= 3.0f) ? 1 : 5;  // 1 = just center, 5 = cross
	    
		int depthSteps = (int)((ring.maxDist - ring.minDist) / 3.0f);
		depthSteps = std::max(1, depthSteps);
		
		for (int d = 0; d < depthSteps; d++) {
			double t = (double)d / depthSteps;
			double dist = ring.minDist + t * (ring.maxDist - ring.minDist);
			
			// Adjust distance based on pitch - looking down sees farther ahead
			double pitchOffset = 0.0f;
			if (radPitch < 0) {  // Looking down
				// Shift the pattern forward proportional to how far down we're looking
				pitchOffset = -dist * sin(radPitch) * 0.5f;  // 50% compensation
			}
			double adjustedDist = dist + pitchOffset;
			
			double width = adjustedDist * tan(fovHorizontal / 2.0f) * 1.4f;
			int patchesAcross = (int)ceil(2.0f * width / ring.patchSize) + 4;
			
			for (int w = -patchesAcross/2; w <= patchesAcross/2; w++) {
				// Use adjusted distance for z-position
				Point viewPos(w * ring.patchSize, 0, -adjustedDist);
				
				double worldX = viewPosition.x + viewPos.x * cos(radYaw) - viewPos.z * sin(radYaw);
				double worldZ = viewPosition.z + viewPos.x * sin(radYaw) + viewPos.z * cos(radYaw);
				
				int gridX = (int)round(worldX / ring.patchSize);
				int gridZ = (int)round(worldZ / ring.patchSize);
				
				// Add cross-pattern neighbors
				for (int i = 0; i < neighborPattern; i++) {
					int nx = gridX + neighbors[i][0];
					int nz = gridZ + neighbors[i][1];
					
					std::string name = "patch_" + std::to_string((int)ring.patchSize) + "_" + 
									  std::to_string(nx) + "_" + std::to_string(nz);
					desiredPatchNames.insert(name);
				}
			}
		}
	}
    
    // Update patches (add/remove as needed)
    std::vector<std::string> currentPatchNames = getObjectNames();
    std::set<std::string> currentSet(currentPatchNames.begin(), currentPatchNames.end());
    
    int removed = 0;
    for (const std::string& name : currentPatchNames) {
        if (desiredPatchNames.find(name) == desiredPatchNames.end()) {
            removeObject(name);
            removed++;
        }
    }
    
    int added = 0;
    for (const std::string& name : desiredPatchNames) {
        if (currentSet.find(name) == currentSet.end()) {
            int patchSize, gridX, gridZ;
            if (sscanf(name.c_str(), "patch_%d_%d_%d", &patchSize, &gridX, &gridZ) == 3) {
                Point patchWorldPos(gridX * patchSize, 0, gridZ * patchSize);
                Point size(patchSize, patchSize * 0.5f, patchSize);
                addObject(patchWorldPos, size, name);
                added++;
            }
        }
    }
    
    std::cout << "Terrain: " << desiredPatchNames.size() << " patches (" 
              << added << " added, " << removed << " removed)" << std::endl;
}
void MarchingCubesScene::updateTerrainIncremental(const Point& currentPos, double currentYaw, double currentPitch,
                                                  const Point& lastPos, double lastYaw, double lastPitch,
                                                  double fovHorizontal, double nearDist, double farDist) {
    double yawChange = abs(currentYaw - lastYaw);
    double pitchChange = abs(currentPitch - lastPitch);
    Point posChange = currentPos - lastPos;
    double posDist = sqrt(posChange.x * posChange.x + posChange.z * posChange.z);
 
    if (yawChange < 5.0f && pitchChange < 5.0f && posDist < 0.3f) {
        return;  // No significant change
    }
     
    // Regenerate patches with new pitch
    generateTerrainPatches(currentPos, currentYaw, currentPitch, fovHorizontal, nearDist, farDist);
}

void MarchingCubesScene::invalidate(){
	 for (auto obj : objects) {
	    obj->invalidateMeshCache();
	 }
}
