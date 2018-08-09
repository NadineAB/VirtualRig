/**************************************************************
 * This file is part of Deform Factors demo.                  *
 * Project web page:                                          *
 *    http://vcg.isti.cnr.it/deformfactors/                   *
 *                                                            *
 * Copyright (c) 2013 Marco Tarini <marco.tarini@isti.cnr.it> *
 *                                                            *
 * Deform Factors Demo is an implementation of                *
 * the algorithms and data structures described in            *
 * the Scientific Article:                                    *
 *    Accurate and Efficient Lighting for Skinned Models      *
 *    Marco Tarini, Daniele Panozzo, Olga Sorkine-Hornung     *
 *    Computer Graphic Forum, 2014                            *
 *    (presented at EUROGRAPHICS 2014)                        *
 *                                                            *
 * This Source Code is subject to the terms of                *
 * the Mozilla Public License v. 2.0.                         *
 * One copy of the license is available at                    *
 * http://mozilla.org/MPL/2.0/.                               *
 *                                                            *
 * Additionally, this Source Code is CITEWARE:                *
 * any derivative work must cite the                          *
 * above Scientific Article and include the same condition.   *
 *                                                            *
 **************************************************************/

#include "dual_quaternion.h"



/* convention clash! (sigh)
 * in vcg, real component of quaternion is the first one
 * anywhere else, it is the last one
 */
void putRealFirst( Quaternion &a ){
    a = Quaternion( a[3], a[0], a[1], a[2] );
}

void putRealLast( Quaternion &a ){
    a = Quaternion( a[1], a[2], a[3], a[0] );
}

void DualQuaternion::fromMatrix(const Mat4 &m)
{
    a.FromMatrix( m );
    b = m.GetColumn4(3)/2;
    b[3] = 0;

    putRealFirst( b );

    b = b*a;

    putRealLast( a );
    putRealLast( b );

}


float DualQuaternion::aW() { return  a.W(); }
float DualQuaternion::bW() { return  b.W(); }
Vec3 DualQuaternion::aXYZ() { return Vec3(a.X(),a.Y(),a.Z()); }
Vec3 DualQuaternion::bXYZ() { return Vec3(b.X(),b.Y(),b.Z()); }

Vec3 DualQuaternion::applyToPoint( Vec3 pos ){
    return pos + (
                cross( aXYZ(), cross( aXYZ(), pos) + pos*aW() + bXYZ() )
                + bXYZ() * aW()
                - aXYZ() * bW()
              )*2.0;
}


void DualQuaternion::normalize(){
    float len = a.Norm();
    a /= len;
    b /= len;
    b -= a*dot(a,b);
}

void DualQuaternion::multiplyAndAdd( DualQuaternion d, float scale ){
    if (dot(a,d.a)<0) {
        d.a = -d.a;
        d.b = -d.b;
    }
    a += d.a * scale;
    b += d.b * scale;
}

void DualQuaternion::mult( float scale ){
    a = a * scale;
    b = b * scale;
}


