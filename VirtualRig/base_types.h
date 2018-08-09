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

#ifndef BASE_TYPES_H
#define BASE_TYPES_H

/* Classes to represent vectors matrices.
 *
 * Those classes uses a GLSL-like interface.
 *
 * Here, we rely on the implementations of VCG, which
 * partially relies on Eigen Lib.
 *
 * Ideally, you should be able to substitute
 * these classes with whtever you use for vectors and matrices.
 */



#include <vcg/space/point2.h>
#include <vcg/space/point3.h>
#include <vcg/math/matrix44.h>
#include <vcg/math/matrix33.h>

typedef vcg::Point4f Vec4;
typedef vcg::Point3f Vec3;
typedef vcg::Point2f Vec2;

typedef vcg::Matrix44f Mat4;
typedef vcg::Matrix33f Mat3;

/* (global) functions */
Vec3 cross( Vec3 a, Vec3 b );
float dot( Vec4 a, Vec4 b );

float angle( Vec3 a, Vec3 b );

Mat3 inverse( Mat3 m );
Mat4 inverseOfIsometry( Mat4 m ); // optimized for roto-translations



#endif // BASE_TYPES_H
