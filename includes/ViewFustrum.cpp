/*
 * ViewFustrum.cpp
 *
 *  Created on: Feb 22, 2026
 *      Author: deans
 */

#include "ViewFustrum.h"


void ViewFustrum::extractFromGL() {
    double modelview[16];
    double projection[16];
    
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    
    extractFromMatrices(modelview, projection);
}

void ViewFustrum::extractFromMatrices(const double mv[16], const double proj[16]) {
    // Combine modelview and projection: clip = proj * mv
    double clip[16];
    
    clip[0]  = mv[0]*proj[0]  + mv[1]*proj[4]  + mv[2]*proj[8]   + mv[3]*proj[12];
    clip[1]  = mv[0]*proj[1]  + mv[1]*proj[5]  + mv[2]*proj[9]   + mv[3]*proj[13];
    clip[2]  = mv[0]*proj[2]  + mv[1]*proj[6]  + mv[2]*proj[10]  + mv[3]*proj[14];
    clip[3]  = mv[0]*proj[3]  + mv[1]*proj[7]  + mv[2]*proj[11]  + mv[3]*proj[15];
    
    clip[4]  = mv[4]*proj[0]  + mv[5]*proj[4]  + mv[6]*proj[8]   + mv[7]*proj[12];
    clip[5]  = mv[4]*proj[1]  + mv[5]*proj[5]  + mv[6]*proj[9]   + mv[7]*proj[13];
    clip[6]  = mv[4]*proj[2]  + mv[5]*proj[6]  + mv[6]*proj[10]  + mv[7]*proj[14];
    clip[7]  = mv[4]*proj[3]  + mv[5]*proj[7]  + mv[6]*proj[11]  + mv[7]*proj[15];
    
    clip[8]  = mv[8]*proj[0]  + mv[9]*proj[4]  + mv[10]*proj[8]  + mv[11]*proj[12];
    clip[9]  = mv[8]*proj[1]  + mv[9]*proj[5]  + mv[10]*proj[9]  + mv[11]*proj[13];
    clip[10] = mv[8]*proj[2]  + mv[9]*proj[6]  + mv[10]*proj[10] + mv[11]*proj[14];
    clip[11] = mv[8]*proj[3]  + mv[9]*proj[7]  + mv[10]*proj[11] + mv[11]*proj[15];
    
    clip[12] = mv[12]*proj[0] + mv[13]*proj[4] + mv[14]*proj[8]  + mv[15]*proj[12];
    clip[13] = mv[12]*proj[1] + mv[13]*proj[5] + mv[14]*proj[9]  + mv[15]*proj[13];
    clip[14] = mv[12]*proj[2] + mv[13]*proj[6] + mv[14]*proj[10] + mv[15]*proj[14];
    clip[15] = mv[12]*proj[3] + mv[13]*proj[7] + mv[14]*proj[11] + mv[15]*proj[15];
    
    // Extract planes from combined matrix
    // Right plane
    planes[0].normal.x = clip[3]  - clip[0];
    planes[0].normal.y = clip[7]  - clip[4];
    planes[0].normal.z = clip[11] - clip[8];
    planes[0].d        = clip[15] - clip[12];
    planes[0].normalize();
    
    // Left plane
    planes[1].normal.x = clip[3]  + clip[0];
    planes[1].normal.y = clip[7]  + clip[4];
    planes[1].normal.z = clip[11] + clip[8];
    planes[1].d        = clip[15] + clip[12];
    planes[1].normalize();
    
    // Bottom plane
    planes[2].normal.x = clip[3]  + clip[1];
    planes[2].normal.y = clip[7]  + clip[5];
    planes[2].normal.z = clip[11] + clip[9];
    planes[2].d        = clip[15] + clip[13];
    planes[2].normalize();
    
    // Top plane
    planes[3].normal.x = clip[3]  - clip[1];
    planes[3].normal.y = clip[7]  - clip[5];
    planes[3].normal.z = clip[11] - clip[9];
    planes[3].d        = clip[15] - clip[13];
    planes[3].normalize();
    
    // Far plane
    planes[4].normal.x = clip[3]  - clip[2];
    planes[4].normal.y = clip[7]  - clip[6];
    planes[4].normal.z = clip[11] - clip[10];
    planes[4].d        = clip[15] - clip[14];
    planes[4].normalize();
    
    // Near plane
    planes[5].normal.x = clip[3]  + clip[2];
    planes[5].normal.y = clip[7]  + clip[6];
    planes[5].normal.z = clip[11] + clip[10];
    planes[5].d        = clip[15] + clip[14];
    planes[5].normalize();
}

bool ViewFustrum::intersectsAABB(const Point& center, double halfSize) const {
    // Test AABB against all 6 planes
    for (int i = 0; i < 6; i++) {
        // Find the vertex of the AABB most in the direction of the plane normal
        Point pVertex = center;
        pVertex.x += (planes[i].normal.x >= 0) ? halfSize : -halfSize;
        pVertex.y += (planes[i].normal.y >= 0) ? halfSize : -halfSize;
        pVertex.z += (planes[i].normal.z >= 0) ? halfSize : -halfSize;
        
        // If positive vertex is outside (behind) this plane, box is outside frustum
        if (planes[i].distanceToPoint(pVertex) < 0) {
            return false;
        }
    }
    return true;
}

bool ViewFustrum::containsPoint(const Point& p) const {
    for (int i = 0; i < 6; i++) {
        if (planes[i].distanceToPoint(p) < 0) {
            return false;
        }
    }
    return true;
}

bool ViewFustrum::intersectsSphere(const Point& center, double radius) const {
    for (int i = 0; i < 6; i++) {
        double distance = planes[i].distanceToPoint(center);
        if (distance < -radius) {
            return false;  // Sphere completely outside this plane
        }
    }
    return true;
}


