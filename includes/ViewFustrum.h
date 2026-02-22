/*
 * ViewFustrum.h
 *
 *  Created on: Feb 22, 2026
 *      Author: deans
 */

#ifndef INCLUDES_VIEWFUSTRUM_H_
#define INCLUDES_VIEWFUSTRUM_H_

// CRITICAL: Include GLEW first, before any other GL headers!
#ifdef _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
//#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include "PointClass.h"

class ViewFustrum {
public:
    // Frustum planes (6 planes: near, far, left, right, top, bottom)
    struct Plane {
        Point normal;
        double d;  // Distance from origin along normal
        
        Plane() : normal(0,0,0), d(0) {}
        
        // Signed distance from point to plane (positive = in front)
        double distanceToPoint(const Point& p) const {
            return normal.dot(p) + d;
        }
        
        // Normalize plane equation
        void normalize() {
            double mag = normal.length();
            normal = normal / mag;
            d = d / mag;
        }
    };
    
    Plane planes[6];
    
    ViewFustrum() {}
    
    // Extract frustum from OpenGL matrices
    void extractFromGL();
    
    // Extract from explicit matrices
    void extractFromMatrices(const double modelview[16], const double projection[16]);
    
    // Test if AABB intersects frustum
    bool intersectsAABB(const Point& center, double halfSize) const;
    
    // Test if point is inside frustum
    bool containsPoint(const Point& p) const;
    
    // Test if sphere intersects frustum
    bool intersectsSphere(const Point& center, double radius) const;
};


#endif /* INCLUDES_VIEWFUSTRUM_H_ */
