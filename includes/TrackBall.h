#ifndef TRACKBALL_H_
#define TRACKBALL_H_

/*
 *  Trackball.h
 *
 * - converted to c++ class from original trackball.h file
 * - see notices and copyrights below
 *
 * (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */
/*
 * trackball.h
 * A virtual trackball implementation
 * Written by Gavin Bell for Silicon Graphics, November 1988.
 */

/*
 * Pass the x and y coordinates of the last and current positions of
 * the mouse, scaled so they are from (-1.0 ... 1.0).
 *
 * The resulting rotation is returned as a quaternion rotation in the
 * first paramater.
 */

class TrackBall {
public:
    double beginx,beginy;  /* position of mouse */
    double quat[4];        /* orientation of object */
    int sx,sy,sz;
    double matrix[4][4];

    TrackBall();

    static double tb_project_to_sphere(double r, double x, double y);
    static void normalize_quat(double q[4]);
	static void set(double q[4], double p1x, double p1y, double p2x, double p2y);

    void reset(){
    	beginx = 0.0f;
    	beginy = 0.0f;
    	set(quat, 0.0f, 0.0f, 0.0f, 0.0f );
    }

    void setMouse(int x,int y){
    	beginx=x;beginy=y;
    }
	static void vzero(double *v) {
		v[0] = 0.0;
		v[1] = 0.0;
		v[2] = 0.0;
	}

	static void vset(double *v, double x, double y, double z) {
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}

	static void vsub(const double *src1, const double *src2, double *dst) {
		dst[0] = src1[0] - src2[0];
		dst[1] = src1[1] - src2[1];
		dst[2] = src1[2] - src2[2];
	}

	static void vcopy(const double *v1, double *v2) {
		register int i;
		for (i = 0; i < 3; i++)
			v2[i] = v1[i];
	}

	static void vcross(const double *v1, const double *v2, double *cross) {
		double temp[3];

		temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
		temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
		temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
		vcopy(temp, cross);
	}

	static double vlength(const double *v) {
		return (double) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	}

	static void vscale(double *v, double div) {
		v[0] *= div;
		v[1] *= div;
		v[2] *= div;
	}

	static void vnormal(double *v) {
		vscale(v, 1.0f/vlength(v));
	}

	static double vdot(const double *v1, const double *v2) {
		return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
	}

	static void vadd(const double *src1, const double *src2, double *dst) {
		dst[0] = src1[0] + src2[0];
		dst[1] = src1[1] + src2[1];
		dst[2] = src1[2] + src2[2];
	}


	// multipy a vector by a quaternion rotation matrix
	static void vmult(double v[4],double m[4][4],double r[4]){
		for(int i=0;i<4;i++){
			r[i]=0.0;
			for(int j=0;j<4;j++)
				r[i]+=v[j]*m[j][i];
		}
	}


	void vmult(double v[4], double r[4]){
		vmult(v,matrix,r);
	}

	/*
	 * Given two quaternions, add them together to get a third quaternion.
	 * Adding quaternions to get a compound rotation is analagous to adding
	 * translations to get a compound translation.  When incrementally
	 * adding rotations, the first argument here should be the new
	 * rotation, the second and third the total rotation (which will be
	 * over-written with the resulting new total rotation).
	 */

	static void add_quats(double *q1, double *q2, double *dest);

	/*
	 * A useful function, builds a rotation matrix in Matrix based on
	 * given quaternion.
	 */
	void build_rotmatrix(){
		build_rotmatrix(matrix, quat);
	}
	static void build_rotmatrix(double m[4][4], double q[4]);

	/*
	 * This function computes a quaternion based on an axis (defined by
	 * the given vector) and an angle about which to rotate.  The angle is
	 * expressed in radians.  The result is put into the third argument.
	 */
	static void axis_to_quat(double a[3], double phi, double q[4]);

	// spin trackball from mouse drag
	void spin(int x, int y, int sx, int sy, double ampl);
	void quat_to_euler(double e[4]);
	void euler_to_quat(double e[4]);
	static void quat_to_euler(double e[4],double q[4]);
	static void euler_to_quat(double e[4],double q[4]);
};
#endif
