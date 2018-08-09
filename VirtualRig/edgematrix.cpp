//
// originally implemented by Justin Legakis
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "matrix.h"
#include "vectors.h"


float det4x4(float a1, float a2, float a3, float a4, 
	     float b1, float b2, float b3, float b4, 
	     float c1, float c2, float c3, float c4, 
	     float d1, float d2, float d3, float d4);
float det3x3(float a1,float a2,float a3,
	     float b1,float b2,float b3,
	     float c1,float c2,float c3);
float det2x2(float a, float b,
	     float c, float d);

// ===================================================================
// ===================================================================
// COPY CONSTRUCTOR

Matrix::Matrix(const Matrix& m) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = m.data[y][x]; }
  }
}

Matrix::Matrix(const float *m) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = m[4*y+x]; }
  }
}

// ===================================================================
// ===================================================================
// MODIFIERS

void Matrix::SetToIdentity() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = (x == y); 
    }
  }
}

void Matrix::Clear() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      data[y][x] = 0; 
    }
  }
}

void Matrix::Transpose(Matrix &m) const {
  // be careful, <this> might be <m>
  Matrix tmp = Matrix(*this);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      m.data[y][x] = tmp.data[x][y];
    }
  }
}

// ===================================================================
// ===================================================================
// INVERSE

int Matrix::Inverse(Matrix &m, float epsilon) const {
  m = *this;

  float a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;
  a1 = m.data[0][0]; b1 = m.data[0][1]; c1 = m.data[0][2]; d1 = m.data[0][3];
  a2 = m.data[1][0]; b2 = m.data[1][1]; c2 = m.data[1][2]; d2 = m.data[1][3];
  a3 = m.data[2][0]; b3 = m.data[2][1]; c3 = m.data[2][2]; d3 = m.data[2][3];
  a4 = m.data[3][0]; b4 = m.data[3][1]; c4 = m.data[3][2]; d4 = m.data[3][3];

  float det = det4x4(a1,a2,a3,a4,b1,b2,b3,b4,c1,c2,c3,c4,d1,d2,d3,d4);

  if (fabs(det) < epsilon) {
    printf ("Matrix::Inverse --- singular matrix, can't invert!\n");
    assert(0);
    return 0;
  }

  m.data[0][0] =   det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
  m.data[1][0] = - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
  m.data[2][0] =   det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
  m.data[3][0] = - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);

  m.data[0][1] = - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
  m.data[1][1] =   det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
  m.data[2][1] = - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
  m.data[3][1] =   det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
  
  m.data[0][2] =   det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
  m.data[1][2] = - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
  m.data[2][2] =   det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
  m.data[3][2] = - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
  
  m.data[0][3] = - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
  m.data[1][3] =   det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
  m.data[2][3] = - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
  m.data[3][3] =   det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
  
  m *= 1/det;
  return 1;
}

float det4x4(float a1, float a2, float a3, float a4, 
	     float b1, float b2, float b3, float b4, 
	     float c1, float c2, float c3, float c4, 
	     float d1, float d2, float d3, float d4) {
  return 
      a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
    - b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
    + c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
    - d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
}

float det3x3(float a1,float a2,float a3,
	     float b1,float b2,float b3,
	     float c1,float c2,float c3) {
  return
      a1 * det2x2( b2, b3, c2, c3 )
    - b1 * det2x2( a2, a3, c2, c3 )
    + c1 * det2x2( a2, a3, b2, b3 );
}

float det2x2(float a, float b,
	     float c, float d) {
  return a * d - b * c;
}

// ===================================================================
// ===================================================================
// OVERLOADED OPERATORS

Matrix& Matrix::operator=(const Matrix& m) {
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      data[y][x] = m.data[y][x]; 
    }
  }
  return (*this); 
}

int Matrix::operator==(const Matrix& m) const {
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      if (this->data[y][x] != m.data[y][x]) {
	return 0; 
      }
    }
  }
  return 1; 
}

Matrix operator+(const Matrix& m1, const Matrix& m2) {
  Matrix answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      answer.data[y][x] = m1.data[y][x] + m2.data[y][x];
    }
  }
  return answer; 
}

Matrix operator-(const Matrix& m1, const Matrix& m2) {
  Matrix answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      answer.data[y][x] = m1.data[y][x] - m2.data[y][x];
    }
  }
  return answer; 
}

Matrix operator*(const Matrix& m1, const Matrix& m2) {
  Matrix answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      for (int i=0; i<4; i++) {
	answer.data[y][x] 
	  += m1.data[y][i] * m2.data[i][x];
      }
    }
  }
  return answer;
}

Matrix operator*(const Matrix& m, float f) {
  Matrix answer;
  for (int y=0; y<4; y++) {
    for (int x=0; x<4; x++) {
      answer.data[y][x] = m.data[y][x] * f;
    }
  }
  return answer;
}

// ====================================================================
// ====================================================================
// TRANSFORMATIONS

Matrix Matrix::MakeTranslation(const Vec3f &v) {
  Matrix t;
  t.SetToIdentity();
  t.data[0][3] = v.x();
  t.data[1][3] = v.y();
  t.data[2][3] = v.z();
  return t;
}

Matrix Matrix::MakeScale(const Vec3f &v) {
  Matrix s; 
  s.SetToIdentity();
  s.data[0][0] = v.x();
  s.data[1][1] = v.y();;
  s.data[2][2] = v.z();
  s.data[3][3] = 1;
  return s;
}

Matrix Matrix::MakeXRotation(float theta) {
  Matrix rx;
  rx.SetToIdentity();
  rx.data[1][1]= (float)cos((float)theta);
  rx.data[1][2]=-(float)sin((float)theta);
  rx.data[2][1]= (float)sin((float)theta);
  rx.data[2][2]= (float)cos((float)theta);
  return rx;
}

Matrix Matrix::MakeYRotation(float theta) {
  Matrix ry;
  ry.SetToIdentity();
  ry.data[0][0]= (float)cos((float)theta);
  ry.data[0][2]= (float)sin((float)theta);
  ry.data[2][0]=-(float)sin((float)theta);
  ry.data[2][2]= (float)cos((float)theta);
  return ry;
}

Matrix Matrix::MakeZRotation(float theta) {
  Matrix rz;
  rz.SetToIdentity();
  rz.data[0][0]= (float)cos((float)theta);
  rz.data[0][1]=-(float)sin((float)theta);
  rz.data[1][0]= (float)sin((float)theta);
  rz.data[1][1]= (float)cos((float)theta);
  return rz;
}

Matrix Matrix::MakeAxisRotation(const Vec3f &v, float theta) {
  Matrix r;
  r.SetToIdentity();

  float x = v.x(); float y = v.y(); float z = v.z();

  float c = cosf(theta);
  float s = sinf(theta);
  float xx = x*x;
  float xy = x*y;
  float xz = x*z;
  float yy = y*y;
  float yz = y*z;
  float zz = z*z;

  r.Set(0,0, (1-c)*xx + c);
  r.Set(0,1, (1-c)*xy + z*s);
  r.Set(0,2, (1-c)*xz - y*s);
  r.Set(0,3, 0);

  r.Set(1,0, (1-c)*xy - z*s);
  r.Set(1,1, (1-c)*yy + c);
  r.Set(1,2, (1-c)*yz + x*s);
  r.Set(1,3, 0);

  r.Set(2,0, (1-c)*xz + y*s);
  r.Set(2,1, (1-c)*yz - x*s);
  r.Set(2,2, (1-c)*zz + c);
  r.Set(2,3, 0);

  r.Set(3,0, 0);
  r.Set(3,1, 0);
  r.Set(3,2, 0);
  r.Set(3,3, 1);

  return r;
}

// ====================================================================
// ====================================================================

void Matrix::Transform(Vec4f &v) const {
  Vec4f answer;
  for (int y=0; y<4; y++) {
    answer.data[y] = 0;
    for (int i=0; i<4; i++) {
      answer.data[y] += data[y][i] * v[i];
    }
  }
  v = answer;
}

// ====================================================================
// ====================================================================

void Matrix::Write(FILE *F) const {
  assert (F != NULL);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      float tmp = data[y][x];
      if (fabs(tmp) < 0.00001) tmp = 0;
      fprintf (F, "%12.6f ", tmp);
    }
    fprintf (F,"\n"); 
  } 
}

void Matrix::Read(FILE *F) {
  assert (F != NULL);
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      int scanned = fscanf (F,"%f",&data[y][x]);
      assert (scanned == 1); 
    }
  } 
}

// ====================================================================
// ====================================================================
