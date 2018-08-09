#ifndef _MATRIX_H_
#define _MATRIX_H_

//
// originally implemented by Justin Legakis
//

#include <math.h>
#include <assert.h>

#include "vectors.h"

// ====================================================================
// ====================================================================

class Matrix {

public:

  // CONSTRUCTORS & DESTRUCTOR
  Matrix() { Clear(); }
  Matrix(const Matrix& m);
  Matrix(const float *m);
  ~Matrix() {}
  
  // ACCESSORS
  float* glGet(void) const {
    float *glMat = new float[16];
     glMat[0]=data[0][0];  glMat[1]=data[1][0];  glMat[2]=data[2][0];  glMat[3]=data[3][0];
     glMat[4]=data[0][1];  glMat[5]=data[1][1];  glMat[6]=data[2][1];  glMat[7]=data[3][1];
     glMat[8]=data[0][2];  glMat[9]=data[1][2]; glMat[10]=data[2][2]; glMat[11]=data[3][2];
    glMat[12]=data[0][3]; glMat[13]=data[1][3]; glMat[14]=data[2][3]; glMat[15]=data[3][3];
    return glMat; }
  float Get(int x, int y) const { 
    assert (x >= 0 && x < 4);
    assert (y >= 0 && y < 4);
    return data[y][x]; }
  
  // MODIFIERS
  void Set(int x, int y, float v) {
    assert (x >= 0 && x < 4);
    assert (y >= 0 && y < 4);
    data[y][x] = v; }
  void SetToIdentity();
  void Clear();

  void Transpose(Matrix &m) const;
  void Transpose() { Transpose(*this); }

  int Inverse(Matrix &m, float epsilon = 1e-08) const;
  int Inverse(float epsilon = 1e-08) { return Inverse(*this,epsilon); }

  // OVERLOADED OPERATORS
  Matrix& operator=(const Matrix& m);
  int operator==(const Matrix& m) const;
  int operator!=(const Matrix &m) const { return !(*this==m); }
  friend Matrix operator+(const Matrix &m1, const Matrix &m2);
  friend Matrix operator-(const Matrix &m1, const Matrix &m2);
  friend Matrix operator*(const Matrix &m1, const Matrix &m2);
  friend Matrix operator*(const Matrix &m1, float f);
  friend Matrix operator*(float f, const Matrix &m) { return m * f; }
  Matrix& operator+=(const Matrix& m) { *this = *this + m; return *this; }
  Matrix& operator-=(const Matrix& m) { *this = *this - m; return *this; }
  Matrix& operator*=(const float f)   { *this = *this * f; return *this; }
  Matrix& operator*=(const Matrix& m) { *this = *this * m; return *this; }

  // TRANSFORMATIONS
  static Matrix MakeTranslation(const Vec3f &v);
  static Matrix MakeScale(const Vec3f &v);
  static Matrix MakeScale(float s) { return MakeScale(Vec3f(s,s,s)); }
  static Matrix MakeXRotation(float theta);
  static Matrix MakeYRotation(float theta);
  static Matrix MakeZRotation(float theta);
  static Matrix MakeAxisRotation(const Vec3f &v, float theta);

  // Use to transform a point with a matrix
  // that may include translation
  void Transform(Vec4f &v) const;
  void Transform(Vec3f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),v.z(),1);
    Transform(v2);
    v.Set(v2.x(),v2.y(),v2.z()); }
  void Transform(Vec2f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),1,1);
    Transform(v2);
    v.Set(v2.x(),v2.y()); }

  // Use to transform the direction of the ray
  // (ignores any translation)
  void TransformDirection(Vec3f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),v.z(),0);
    Transform(v2);
    v.Set(v2.x(),v2.y(),v2.z()); }

  // INPUT / OUTPUT
  void Write(FILE *F = stdout) const;
  void Read(FILE *F);
  
private:

  // REPRESENTATION
  float	data[4][4];

};

// ====================================================================
// ====================================================================

#endif
