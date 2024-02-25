#ifndef GEOTYPES_BASE_H
//=========================================================================
// geotypes-base.h
//
// Module with basic geometric objects and functions to manipulate them.
// All calculations done here are only as precise as the limited mantissa
// in 64-bits floating points allow for.
//=========================================================================
#define GEOTYPES_BASE_H

#include "tinybase-types.h"

#define PI  3.141592653589793
#define TAU 6.283185307179586
#define RADIAN(A) (A * 0.017453292519943295)
#define DEGREE(A) (A * 57.29577951308232)

//==================================
// 2D Vector
//==================================

struct v2
{
    f64 X, Y;
};

inline v2 V2(f64 X, f64 Y) { v2 Result = { X, Y }; return Result; }
#define INVALID_V2 V2(INF64, INF64)

f64 Cross(v2 A, v2 B);       // Cross product of [A] over [B].
f64 Dist(v2 A, v2 B);        // Euclidean distance.
f64 Dist2(v2 A, v2 B);       // Euclidean distance squared (no sqrt).
f64 Dot(v2 A, v2 B);         // Dot product.
v2  Lerp(v2 A, f64 t, v2 B); // Linear interpolation between [A] and [B] at [t] percent.
f64 Mag(v2 A);               // Magnitude.
f64 Mag2(v2 A);              // Magnitude squared (no sqrt).
v2  Rotate90CW(v2 A);        // Rotates 90 degrees Clockwise.
v2  Rotate90CCW(v2 A);       // Rotates 90 degrees Counter-Clockwise.
v2  Unit(v2 A);              // Unit vector.

inline v2  operator+(v2 A, v2 B) { return V2(A.X+B.X, A.Y+B.Y); }
inline v2& operator+=(v2& A, v2 B) { A = A + B; return A; }
inline v2  operator-(v2 A, v2 B) { return V2(A.X-B.X, A.Y-B.Y); }
inline v2& operator-=(v2& A, v2 B) { A = A - B; return A; }
inline v2  operator*(v2 A, f64 Scalar) { return V2(A.X*Scalar, A.Y*Scalar); }
inline v2  operator*(f64 Scalar, v2 A) { return A * Scalar; }
inline v2& operator*=(v2& A, f64 Scalar) { A = A * Scalar; return A; }
inline v2  operator/(v2 A, f64 Scalar) { return V2(A.X/Scalar, A.Y/Scalar); }
inline v2  operator/(f64 Scalar, v2 A) { return A / Scalar; }
inline v2& operator/=(v2& A, f64 Scalar) { A = A / Scalar; return A; }
inline bool operator==(v2 A, v2 B) { return A.X == B.X && A.Y == B.Y; }

//==================================
// 3D Vector
//==================================

struct v3
{
    union
    {
        v2 XY;
        struct { f64 X, Y; };
    };
    union { f64 M, Z; };
};

inline v3 V3(f64 X, f64 Y, f64 Z) { v3 Result = { X, Y, Z }; return Result; }
inline v3 V3(v2 XY, f64 Z) { return V3(XY.X, XY.Y, Z); }
#define INVALID_V3 V3(INF64, INF64, INF64)

v3  Cross(v3 A, v3 B);       // Cross product of [A] over [B].
f64 Dist(v3 A, v3 B);        // 3D euclidean distance.
f64 Dist2(v3 A, v3 B);       // 3D euclidean distance squared (no sqrt).
f64 Dot(v3 A, v3 B);         // Dot product.
v3  Lerp(v3 A, f64 t, v3 B); // Linear interpolation between [A] and [B] at [t] percent.
f64 Mag(v3 A);               // Magnitude.
f64 Mag2(v3 A);              // Magnitude squared (no sqrt).
v3  Rotate90XAxisCW(v3 A);   // Rotates 90 degrees around X-axis Clockwise.
v3  Rotate90XAxisCCW(v3 A);  // Rotates 90 degrees around X-axis Counter-Clockwise.
v3  Rotate90YAxisCW(v3 A);   // Rotates 90 degrees around Y-axis Clockwise.
v3  Rotate90YAxisCCW(v3 A);  // Rotates 90 degrees around Y-axis Counter-Clockwise.
v3  Rotate90ZAxisCW(v3 A);   // Rotates 90 degrees around Z-axis Clockwise.
v3  Rotate90ZAxisCCW(v3 A);  // Rotates 90 degrees around Z-axis Counter-Clockwise.
v3  Unit(v3 A);              // Unit vector.

inline v3  operator+(v3 A, v3 B) { return V3(A.X+B.X, A.Y+B.Y, A.Z+B.Z); }
inline v3& operator+=(v3& A, v3 B) { A = A + B; return A; }
inline v3  operator-(v3 A, v3 B) { return V3(A.X-B.X, A.Y-B.Y, A.Z-B.Z); }
inline v3& operator-=(v3& A, v3 B) { A = A - B; return A; }
inline v3  operator*(v3 A, f64 Scalar) { return V3(A.X*Scalar, A.Y*Scalar, A.Z*Scalar); }
inline v3  operator*(f64 Scalar, v3 A) { return A * Scalar; }
inline v3& operator*=(v3& A, f64 Scalar) { A = A * Scalar; return A; }
inline v3  operator/(v3 A, f64 Scalar) { return V3(A.X/Scalar, A.Y/Scalar, A.Z/Scalar); }
inline v3  operator/(f64 Scalar, v3 A) { return A / Scalar; }
inline v3& operator/=(v3& A, f64 Scalar) { A = A / Scalar; return A; }
inline bool operator==(v3 A, v3 B) { return A.X == B.X && A.Y == B.Y && A.Z == B.Z; }

//==================================
// 4D Vector
//==================================

struct v4
{
    union
    {
        v3 XYZ;
        struct
        {
            v2 XY;
            f64 Z;
        };
        struct { f64 X, Y, _Padding; };
    };
    union { f64 M, W; };
};

inline v4 V4(f64 X, f64 Y, f64 Z, f64 M) { v4 Result = { X, Y, Z, M }; return Result; }
inline v4 V4(v2 XY, f64 Z, f64 M) { return V4(XY.X, XY.Y, Z, M); }
inline v4 V4(v3 XYZ, f64 M) { return V4(XYZ.X, XYZ.Y, XYZ.Z, M); }
#define INVALID_V4 V4(INF64, INF64, INF64, INF64)

v4  Cross(v4 A, v4 B);       // Cross product of [A] over [B].
f64 Dot(v4 A, v4 B);         // Dot product.
v4  Lerp(v4 A, f64 t, v4 B); // Linear interpolation between [A] and [B] at [t] percent.

inline v4  operator+(v4 A, v4 B) { return V4(A.X+B.X, A.Y+B.Y, A.Z+B.Z, A.M+B.M); }
inline v4& operator+=(v4& A, v4 B) { A = A + B; return A; }
inline v4  operator-(v4 A, v4 B) { return V4(A.X-B.X, A.Y-B.Y, A.Z-B.Z, A.M-B.M); }
inline v4& operator-=(v4& A, v4 B) { A = A - B; return A; }
inline v4  operator*(v4 A, f64 Scalar) { return V4(A.X*Scalar, A.Y*Scalar, A.Z*Scalar, A.M*Scalar); }
inline v4  operator*(f64 Scalar, v4 A) { return A * Scalar; }
inline v4& operator*=(v4& A, f64 Scalar) { A = A * Scalar; return A; }
inline v4  operator/(v4 A, f64 Scalar) { return V4(A.X/Scalar, A.Y/Scalar, A.Z/Scalar, A.M/Scalar); }
inline v4  operator/(f64 Scalar, v4 A) { return A / Scalar; }
inline v4& operator/=(v4& A, f64 Scalar) { A = A / Scalar; return A; }
inline bool operator==(v4 A, v4 B) { return A.X == B.X && A.Y == B.Y && A.Z == B.Z && A.M == B.M; }

//==================================
// 2x2 Matrix
//==================================

struct m22
{
    union
    {
        v2 V[2];
        f64 E[2][2];
    };
    
    f64* operator[](int Idx) const { return (f64*)&E[Idx]; }
};

inline m22 M22(v2 Row0, v2 Row1) { return { Row0, Row1 }; }
inline m22 M22(f64 a, f64 b, f64 c, f64 d) { return M22(V2(a, b), V2(c, d)); }
#define INVALID_M22 M22(INVALID_V2, INVALID_V2)

f64 Determinant(m22 M);
m22 Adjugate(m22 M);
m22 Inverse(m22 M);
m22 Transpose(m22 M);

m22 M22Identity(void);

inline m22  operator+(m22 A, m22 B) { return M22(A.V[0]+B.V[0], A.V[1]+B.V[1]); }
inline m22& operator+=(m22& A, m22 B) { A = A + B; return A; }
inline m22  operator-(m22 A, m22 B) { return M22(A.V[0]-B.V[0], A.V[1]-B.V[1]); }
inline m22& operator-=(m22& A, m22 B) { A = A - B; return A; }
inline m22  operator*(m22 M, f64 Scalar) { return M22(M.V[0]*Scalar, M.V[1]*Scalar); }
inline m22  operator*(f64 Scalar, m22 M) { return M * Scalar; }
inline m22& operator*=(m22& M, f64 Scalar) { M = M * Scalar; return M; }
inline v2   operator*(m22 M, v2 V) { return V2(Dot(V, M.V[0]), Dot(V, M.V[1])); }
inline v2   operator*(v2 V, m22 M) { return M * V; }
inline v2&  operator*=(v2& V, m22 M) { V = V * M; return V; }
inline m22  operator*(m22 A, m22 B) { B = Transpose(B); return M22(B*A.V[0], B*A.V[1]); }
inline m22& operator*=(m22& A, m22 B) { A = A * B; return A; }
inline bool  operator==(m22 A, m22 B) { return A.V[0] == B.V[0] && A.V[1] == B.V[1]; }

//==================================
// 3x3 Matrix
//==================================

struct m33
{
    union
    {
        struct v3 V[3];
        f64 E[3][3];
    };
    
    f64* operator[](int Idx) const { return (f64*)&E[Idx]; }
};

inline m33 M33(v3 Row0, v3 Row1, v3 Row2) { return { Row0, Row1, Row2 }; }
inline m33 M33(f64 a, f64 b, f64 c, f64 d, f64 e, f64 f, f64 g, f64 h, f64 i)
{ return M33(V3(a, b, c), V3(d, e, f), V3(g, h, i)); }
#define INVALID_M33 M33(INVALID_V3, INVALID_V3, INVALID_V3)

f64 Determinant(m33 M);
m33 Adjugate(m33 M);
m33 Inverse(m33 M);
m33 Transpose(m33 M);

m33 M33Identity(void);
m33 M33Rotate(f64 Angle);
m33 M33Scale(v2 ScaleVector);
m33 M33Translate(v2 TranslationVector);
m33 M33Shear(v2 ShearVector);
m33 M33FlipX(void);
m33 M33FlipY(void);

inline m33  operator+(m33 A, m33 B) { return M33(A.V[0]+B.V[0], A.V[1]+B.V[1], A.V[2]+B.V[2]); }
inline m33& operator+=(m33& A, m33 B) { A = A + B; return A; }
inline m33  operator-(m33 A, m33 B) { return M33(A.V[0]-B.V[0], A.V[1]-B.V[1], A.V[2]-B.V[2]); }
inline m33& operator-=(m33& A, m33 B) { A = A - B; return A; }
inline m33  operator*(m33 M, f64 Scalar) { return M33(M.V[0]*Scalar, M.V[1]*Scalar, M.V[2]*Scalar); }
inline m33  operator*(f64 Scalar, m33 M) { return M * Scalar; }
inline m33& operator*=(m33& M, f64 Scalar) { M = M * Scalar; return M; }
inline v3   operator*(m33 M, v3 V) { return V3(Dot(V, M.V[0]), Dot(V, M.V[1]), Dot(V, M.V[2])); }
inline v3   operator*(v3 V, m33 M) { return M * V; }
inline v3&  operator*=(v3 V, m33 M) { V = V * M; return V; }
inline v2   operator*(m33 M, v2 V) { return (M * V3(V.X, V.Y, 1.0)).XY; }
inline v2   operator*(v2 V, m33 M) { return M * V; }
inline v2&  operator*=(v2 V, m33 M) { V = V * M; return V; }
inline m33  operator*(m33 A, m33 B){ B = Transpose(B); return M33(B*A.V[0], B*A.V[1], B*A.V[2]); }
inline m33& operator*=(m33& A, m33 B) { A = A * B; return A; }
inline bool  operator==(m33 A, m33 B) { return A.V[0]==B.V[0] && A.V[1]==B.V[1] && A.V[2]==B.V[2]; }

//==================================
// 4x4 Matrix
//==================================

struct m44
{
    union
    {
        struct v4 V[4];
        f64 E[4][4];
    };
    
    f64* operator[](int Idx) const { return (f64*)&E[Idx]; }
};

inline m44 M44(v4 Row0, v4 Row1, v4 Row2, v4 Row3) { return { Row0, Row1, Row2, Row3 }; }
inline m44 M44(f64 a, f64 b, f64 c, f64 d, f64 e, f64 f, f64 g, f64 h,
               f64 i, f64 j, f64 k, f64 l, f64 m, f64 n, f64 o, f64 p)
{ return M44(V4(a, b, c, d), V4(e, f, g, h), V4(i, j, k, l), V4(m, n, o, p)); }
#define INVALID_M44 M44(INVALID_V4, INVALID_V4, INVALID_V4, INVALID_V4)

f64 Determinant(m44 M);
m44 Adjugate(m44 M);
m44 Inverse(m44 M);
m44 Transpose(m44 M);

m44 M44Identity(void);
m44 M44Rotate(f64 Angle, v3 RotationAxis);
m44 M44RotateX(f64 Angle);
m44 M44RotateY(f64 Angle);
m44 M44RotateZ(f64 Angle);
m44 M44Scale(v3 ScaleVector);
m44 M44Translate(v3 TranslationVector);
m44 M44FlipX(void);
m44 M44FlipY(void);
m44 M44FlipZ(void);

inline m44  operator+(m44 A, m44 B)
{ return M44(A.V[0]+B.V[0], A.V[1]+B.V[1], A.V[2]+B.V[2], A.V[3]+B.V[3]); }
inline m44& operator+=(m44& A, m44 B) { A = A + B; return A; }
inline m44  operator-(m44 A, m44 B)
{ return M44(A.V[0]-B.V[0], A.V[1]-B.V[1], A.V[2]-B.V[2], A.V[3]-B.V[3]); }
inline m44& operator-=(m44& A, m44 B) { A = A - B; return A; }
inline m44  operator*(m44 M, f64 Scalar)
{ return M44(M.V[0]*Scalar, M.V[1]*Scalar, M.V[2]*Scalar, M.V[3]*Scalar); }
inline m44  operator*(f64 Scalar, m44 M) { return M * Scalar; }
inline m44& operator*=(m44& M, f64 Scalar) { M = M * Scalar; return M; }
inline v4   operator*(m44 M, v4 V)
{ return V4(Dot(V, M.V[0]), Dot(V, M.V[1]), Dot(V, M.V[2]), Dot(V, M.V[3])); }
inline v4   operator*(v4 V, m44 M) { return M * V; }
inline v4&  operator*=(v4 V, m44 M) { V = V * M; return V; }
inline v3   operator*(m44 M, v3 V) { return (M * V4(V.X, V.Y, V.Z, 1.0)).XYZ; }
inline v3   operator*(v3 V, m44 M) { return M * V; }
inline v3&  operator*=(v3 V, m44 M) { V = V * M; return V; }
inline m44  operator*(m44 A, m44 B)
{ B = Transpose(B); return M44(B*A.V[0], B*A.V[1], B*A.V[2], B*A.V[3]); }
inline m44& operator*=(m44& A, m44 B) { A = A * B; return A; }
inline bool  operator==(m44 A, m44 B)
{ return A.V[0] == B.V[0] && A.V[1] == B.V[1] && A.V[2] == B.V[2] && A.V[3] == B.V[3]; }

//==================================
// Circle
//==================================

struct circle
{
    v2 Centre;
    f64 Radius;
};

inline circle Circle(f64 X, f64 Y, f64 Radius) { circle Result = { V2(X, Y), Radius }; return Result; }
inline circle Circle(v2 Centre, f64 Radius) { circle Result = { Centre, Radius }; return Result; }
circle Circle(v2 A, v2 B, f64 Radius); // [A] and [B] are points on the circle.

f64  Area(circle A);                       // Area of circle.
f64  Length(circle C);                     // Length of circle's perimetre.
f64  ArcLength(f64 Angle, circle C);       // Length of arc formed by [Angle].
f64  ArcLength(v2 A, v2 B, circle C);      // Length of arc between points [A] and [B] (on the circle).
f64  GetAngleOfPoint(v2 A, circle C);      // Angle in radians between point [A] and the right-most point of [C].
v2   GetPointOfAngle(f64 Angle, circle C); // Point on circle from right-most point of [C], angle in radians.
bool PointInCircle(v2 P, circle C);        // Checks if [P] lies inside or on the circle.

//==================================
// BBox 2D
//==================================

struct bbox2
{
    v2 Min, Max;
};

inline bbox2 BBox2(v2 Min, v2 Max) { bbox2 Result = { Min, Max }; return Result; }
inline bbox2 BBox2(f64 XMin, f64 YMin, f64 XMax, f64 YMax) { return BBox2(V2(XMin, YMin), V2(XMax, YMax)); }
inline bbox2 BBox2(v2 Min, f64 Width, f64 Height) { return BBox2(Min, Min + V2(Width, Height)); }

inline bool operator==(bbox2 A, bbox2 B) { return A.Min == B.Min && A.Max == B.Max; }

f64   Area(bbox2 A);                 // Area of bbox2.
bbox2 Merge(bbox2 A, bbox2 B);       // Creates new bbox2 from the mins and maxs of two bbox2.
bool  Intersects(bbox2 A, bbox2 B);  // Check if two bbox2 overlap.
bool  Intersects(v2 P, bbox2 B);     // Check if point is inside or on bbox2.
bool  Intersects(bbox2 B, v2 P);     // Same as above.
bool  Intersects(circle C, bbox2 B); // Check if circle and bbox2 overlap.
bool  Intersects(bbox2 B, circle C); // Same as above.

//==================================
// BBox 3D
//==================================

struct bbox3
{
    v3 Min, Max;
};

inline bbox3 BBox3(v3 Min, v3 Max) { bbox3 Result = { Min, Max }; return Result; }
inline bbox3 BBox3(f64 XMin, f64 YMin, f64 ZMin, f64 XMax, f64 YMax, f64 ZMax) { return BBox3(V3(XMin, YMin, ZMin), V3(XMax, YMax, ZMax)); }
inline bbox3 BBox3(v3 Min, f64 Width, f64 Height, f64 Depth) { return BBox3(Min, Min + V3(Width, Height, Depth)); }

inline bool operator==(bbox3 A, bbox3 B) { return A.Min == B.Min && A.Max == B.Max; }

f64   AreaXY(bbox3 A);         // Area of XY plane from bbox3.
f64   AreaXZ(bbox3 A);         // Area of XZ plane from bbox3.
f64   AreaYZ(bbox3 A);         // Area of YZ plane from bbox3.
f64   Volume(bbox3 A);         // Volume of bbox3.
bbox3 Merge(bbox3 A, bbox3 B); // Creates new bbox3 from the mins and maxs of two bbox3.

//==================================
// BBox 4D
//==================================

struct bbox4
{
    v4 Min, Max;
};

inline bbox4 BBox4(v4 Min, v4 Max) { bbox4 Result = { Min, Max }; return Result; }
inline bbox4 BBox4(f64 XMin, f64 YMin, f64 ZMin, f64 MMin, f64 XMax, f64 YMax, f64 ZMax, f64 MMax) { return BBox4(V4(XMin, YMin, ZMin, MMin), V4(XMax, YMax, ZMax, MMax)); }
inline bbox4 BBox4(v4 Min, f64 Width, f64 Height, f64 Depth, f64 Extent) { return BBox4(Min, Min + V4(Width, Height, Depth, Extent)); }

inline bool operator==(bbox4 A, bbox4 B) { return A.Min == B.Min && A.Max == B.Max; }

f64   AreaXY(bbox4 A);         // Area of XY plane from bbox4.
f64   AreaXZ(bbox4 A);         // Area of XZ plane from bbox4.
f64   AreaYZ(bbox4 A);         // Area of YZ plane from bbox4.
f64   Volume(bbox4 A);         // Volume of XYZ cube of bbox4.
bbox4 Merge(bbox4 A, bbox4 B); // Creates new bbox4 from the mins and maxs of two bbox4.

//==================================
// Line 2D
//==================================

struct line2
{
    v2 P0, P1;
};

inline line2 Line2(v2 P0, v2 P1) { line2 Result = { P0, P1 }; return Result; }
inline line2 Line2(f64 X0, f64 Y0, f64 X1, f64 Y1) { return Line2(V2(X0, Y0), V2(X1, Y1)); }

f64 Dist(v2 P, line2 L); // Distance of [P] to closest point in [L].
f64 Dist(line2 L, v2 P); // Same as above.

f64 LocationAlongLine(line2 L, v2 Target);

/* Percentage between 0 and 1 where [Target] lies along [L]. If [Target] does not lie over [L],
 |  it is projected over it (closest point). Return value is not clamped, so < 0 means [Target]
 |  is before [L], and > 1 means it's after. */

v2 PointAlongLine(line2 L, f64 LineLocation);

/* Given a [LineLocation] percentage between 0 and 1 along [L], returns the point on it.
|  Percentagens outside the [0,1] interval are clamped to it. */

bool IsPointLeftOfLine(v2 P, line2 L);

/* Returns true if [P] is to the left of [L], and false if not. Also returns false if point
|  is on the line. */

//==================================
// Line 3D
//==================================

struct line3
{
    v3 P0, P1;
};

inline line3 Line3(v3 P0, v3 P1) { line3 Result = { P0, P1 }; return Result; }
inline line3 Line3(f64 X0, f64 Y0, f64 Z0, f64 X1, f64 Y1, f64 Z1) { return Line3(V3(X0, Y0, Z0), V3(X1, Y1, Z1)); }

f64 Dist(v3 P, line3 L); // 3D distance of [P] to closest point in [L].
f64 Dist(line3 L, v3 P); // Same as above.

f64 LocationAlongLine(line3 L, v3 Target);

/* Percentage between 0 and 1 where [Target] lies along [L]. If [Target] does not lie over [L],
 |  it is projected over it (closest point). Return value is not clamped, so < 0 means [Target]
 |  is before [L], and > 1 means it's after. */

v3 PointAlongLine(line3 L, f64 LineLocation);

/* Given a [LineLocation] percentage between 0 and 1 along [L], returns the point on it.
|  Percentagens outside the [0,1] interval are clamped to it. */


#if !defined(GEOTYPES_STATIC_LINKING)
#include "geotypes-base.cpp"
#endif

#endif //GEOTYPES_BASE_H
