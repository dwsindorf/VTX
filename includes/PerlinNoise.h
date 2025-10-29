#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <vector>
#include <random>

class PerlinNoise {
private:
    std::vector<int> p;
    
    float fade(float t) const;
    float lerp(float t, float a, float b) const;
    float grad(int hash, float x, float y, float z) const;
    
public:
    PerlinNoise();
    PerlinNoise(unsigned int seed);
    float noise(float x, float y, float z) const;
    float octaveNoise(float x, float y, float z, int orders, float fstart, float fdelta,float astart, float adelta) const;

};

#endif
