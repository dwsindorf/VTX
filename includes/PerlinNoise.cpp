#include "PerlinNoise.h"
#include <algorithm>
#include <numeric>
#include <cmath>

PerlinNoise::PerlinNoise() {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);
    std::default_random_engine engine;
    std::shuffle(p.begin(), p.end(), engine);
    p.insert(p.end(), p.begin(), p.end());
}

PerlinNoise::PerlinNoise(unsigned int seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);
    std::default_random_engine engine(seed);
    std::shuffle(p.begin(), p.end(), engine);
    p.insert(p.end(), p.begin(), p.end());
}

float PerlinNoise::fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::lerp(float t, float a, float b) const{
    return a + t * (b - a);
}

float PerlinNoise::grad(int hash, float x, float y, float z) const {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float PerlinNoise::noise(float x, float y, float z) const {
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;
    
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);
    
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);
    
    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;
    
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
                                   grad(p[BA], x-1, y, z)),
                           lerp(u, grad(p[AB], x, y-1, z),
                                   grad(p[BB], x-1, y-1, z))),
                   lerp(v, lerp(u, grad(p[AA+1], x, y, z-1),
                                   grad(p[BA+1], x-1, y, z-1)),
                           lerp(u, grad(p[AB+1], x, y-1, z-1),
                                   grad(p[BB+1], x-1, y-1, z-1))));
}


float PerlinNoise::octaveNoise(float x, float y, float z, int orders, float fstart, float fdelta,float astart, float adelta) const {

    float freq = fstart;
    float ampl = astart;

    float noiseValue = 0;
    float maxValue = 0;
	for(int i = 0; i < orders; i++) {
		noiseValue += noise(x * freq, y * freq, z * freq) * ampl;
		maxValue+=ampl;
		ampl *= adelta;
		freq *= fdelta;
	}    
    return noiseValue/maxValue;
}
