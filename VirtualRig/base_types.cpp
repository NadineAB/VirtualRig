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

#include "base_types.h"


/* implementation of basic oprations for points, vectors
 * Here, we rely on the  VCG lib implementation.
 * It should be easy to convert these.
 */

Vec3 cross( Vec3 a, Vec3 b) {
    return a^b;
}

float dot( Vec4 a, Vec4 b) {
    return a*b;
}

Mat3 inverse( Mat3 m) {
  return m;
}


float angle( Vec3 a, Vec3 b ){
    return vcg::Angle(a,b);
}


Mat4 inverseOfIsometry(Mat4 m) {
    Mat4 res = m;

    Vec4 tra = res.GetColumn4(3);
    res.SetColumn(3, Vec4(0,0,0,1) );

    res.transposeInPlace();

    tra = -(res*tra);
    tra[3] = 1;
    res.SetColumn(3, tra);

    return res;
}


