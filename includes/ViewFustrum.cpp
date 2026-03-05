/*
 * ViewFustrum.cpp
 *
 *  Created on: Feb 22, 2026
 *      Author: deans
 */

#include "ViewFustrum.h"


void ViewFustrum::extractFromGL() {
    double mv[16], proj[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, mv);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);

    // Multiply proj * mv in column-major order
    // clip[c*4+r] = sum_k proj[k*4+r] * mv[c*4+k]
    double clip[16];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++) {
            clip[c*4+r] = 0;
            for (int k = 0; k < 4; k++)
                clip[c*4+r] += proj[k*4+r] * mv[c*4+k];
        }

    // Extract planes using Gribb/Hartmann method
    // In column-major: row r of clip matrix uses indices r, r+4, r+8, r+12
    // Plane extraction uses rows of the clip matrix:
    // Right:  row3 - row0
    // Left:   row3 + row0
    // Top:    row3 - row1
    // Bottom: row3 + row1
    // Far:    row3 - row2
    // Near:   row3 + row2

    auto row = [&](int r, int c) { return clip[c*4 + r]; };

    // Right
    planes[0].normal.x = row(3,0) - row(0,0);
    planes[0].normal.y = row(3,1) - row(0,1);
    planes[0].normal.z = row(3,2) - row(0,2);
    planes[0].d        = row(3,3) - row(0,3);
    planes[0].normalize();

    // Left
    planes[1].normal.x = row(3,0) + row(0,0);
    planes[1].normal.y = row(3,1) + row(0,1);
    planes[1].normal.z = row(3,2) + row(0,2);
    planes[1].d        = row(3,3) + row(0,3);
    planes[1].normalize();

    // Top
    planes[2].normal.x = row(3,0) - row(1,0);
    planes[2].normal.y = row(3,1) - row(1,1);
    planes[2].normal.z = row(3,2) - row(1,2);
    planes[2].d        = row(3,3) - row(1,3);
    planes[2].normalize();

    // Bottom
    planes[3].normal.x = row(3,0) + row(1,0);
    planes[3].normal.y = row(3,1) + row(1,1);
    planes[3].normal.z = row(3,2) + row(1,2);
    planes[3].d        = row(3,3) + row(1,3);
    planes[3].normalize();

    // Far
    planes[4].normal.x = row(3,0) - row(2,0);
    planes[4].normal.y = row(3,1) - row(2,1);
    planes[4].normal.z = row(3,2) - row(2,2);
    planes[4].d        = row(3,3) - row(2,3);
    planes[4].normalize();

    // Near
    planes[5].normal.x = row(3,0) + row(2,0);
    planes[5].normal.y = row(3,1) + row(2,1);
    planes[5].normal.z = row(3,2) + row(2,2);
    planes[5].d        = row(3,3) + row(2,3);
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
        if (planes[i].distanceToPoint(p) < -1e-6)  // small epsilon
            return false;
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


