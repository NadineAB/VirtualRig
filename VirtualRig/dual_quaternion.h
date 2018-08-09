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

#ifndef DUALQUATERNION_H
#define DUALQUATERNION_H

/* Classes to represent quaternions and dual quaterions.
 *
 * Those classes uses a GLSL-like interface.
 *
 * For Quaternion, we rely on the implementation of VCG, which
 * partially relies on Eigen Lib.
 *
 * Ideally, you should be able to substitute
 * these classes with whtever you use for quaterions.
 */


#include "base_types.h"


/*
 * For quaternions, we use vcg ones
 */
#include <vcg/math/quaternion.h>
typedef vcg::Quaternionf Quaternion;


struct DualQuaternion
{
    Quaternion a,b; // primal and dual parts

    void fromMatrix( const Mat4 &m);

    Vec3 applyToPoint( Vec3 );
    void normalize();

    void multiplyAndAdd( DualQuaternion d, float scale );
    void mult( float scale );

    // inner helper functions
    float aW();  // real part of primal
    float bW();  // real part of dual
    Vec3 aXYZ(); // imaginary part of primal
    Vec3 bXYZ(); // imaginary part of dual
};

#endif // DUALQUATERNION_H
