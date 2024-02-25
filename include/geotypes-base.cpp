
//==================================
// 2D Vector
//==================================

f64
Dot(v2 A, v2 B)
{ 
    f64 Result = A.X*B.X + A.Y*B.Y;
    return Result;
}

f64
Cross(v2 A, v2 B)
{
    f64 Result = A.X*B.Y - A.Y*B.X;
    return Result;
}

f64
Mag(v2 A)
{
    f64 Result = Sqrt(A.X*A.X + A.Y*A.Y);
    return Result;
}

f64
Mag2(v2 A)
{
    f64 Result = A.X*A.X + A.Y*A.Y;
    return Result;
}

v2
Unit(v2 A)
{
    v2 Result = A / Mag(A);
    return Result;
}

v2
Lerp(v2 A, f64 t, v2 B)
{
    v2 Result = A*(1.0 - t) + B*t;
    return Result;
}

v2
Rotate90CW(v2 A)
{
    v2 Result = V2(A.Y, -A.X);
    return Result;
}

v2
Rotate90CCW(v2 A)
{
    v2 Result = V2(-A.Y, A.X);
    return Result;
}

f64
Dist(v2 A, v2 B)
{
    f64 Result = Mag(A - B);
    return Result;
}

f64
Dist2(v2 A, v2 B)
{
    f64 Result = Mag2(A - B);
    return Result;
}

//==================================
// 3D Vector
//==================================

f64
Dot(v3 A, v3 B)
{ 
    f64 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return Result;
}

v3
Cross(v3 A, v3 B)
{ 
    v3 Result = V3(A.Y*B.Z - A.Z*B.Y,
                   A.Z*B.X - A.X*B.Z,
                   A.X*B.Y - A.Y*B.Z);
    return Result;
}

f64
Mag(v3 A)
{
    f64 Result = Sqrt(A.X*A.X + A.Y*A.Y + A.Z*A.Z);
    return Result;
}

f64
Mag2(v3 A)
{
    f64 Result = A.X*A.X + A.Y*A.Y + A.Z*A.Z;
    return Result;
}

v3
Unit(v3 A)
{
    v3 Result = A / Mag(A);
    return Result;
}

v3
Lerp(v3 A, f64 t, v3 B)
{
    v3 Result = A*(1.0 - t) + B*t;
    return Result;
}

v3
Rotate90XAxisCW(v3 A)
{
    v3 Result = V3(A.X, A.Z, -A.Y);
    return Result;
}

v3
Rotate90XAxisCCW(v3 A)
{
    v3 Result = V3(A.X, -A.Z, A.Y);
    return Result;
}

v3
Rotate90YAxisCW(v3 A)
{
    v3 Result = V3(-A.Z, A.Y, A.X);
    return Result;
}

v3
Rotate90YAxisCCW(v3 A)
{
    v3 Result = V3(A.Z, A.Y, -A.X);
    return Result;
}

v3
Rotate90ZAxisCW(v3 A)
{
    v3 Result = V3(A.Y, -A.X, A.Z);
    return Result;
}

v3
Rotate90ZAxisCCW(v3 A)
{
    v3 Result = V3(-A.Y, A.X, A.Z);
    return Result;
}

f64
Dist(v3 A, v3 B)
{
    f64 Result = Mag(A - B);
    return Result;
}

f64
Dist2(v3 A, v3 B)
{
    f64 Result = Mag2(A - B);
    return Result;
}

//==================================
// 4D Vector
//==================================

f64
Dot(v4 A, v4 B)
{ 
    f64 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z + A.W*B.W;
    return Result;
}

v4
Cross(v4 A, v4 B)
{
    v4 Result = {0};
    Result.XYZ = Cross(A.XYZ, B.XYZ);
    Result.M = A.M;
    return Result;
}

v4
Lerp(v4 A, f64 t, v4 B)
{
    v4 Result = A*(1.0 - t) + B*t;
    return Result;
}

//==================================
// 2x2 Matrix
//==================================

f64 Determinant(m22 M)
{
    f64 Result = M[0][0]*M[1][1] - M[0][1]*M[1][0];
    return Result;
}

m22
Adjugate(m22 M)
{
    m22 Result = M22(M[1][1], -M[1][0],
                     -M[0][1], M[0][0]);
    return Result;
}

m22
Inverse(m22 M)
{
    f64 Det = Determinant(M);
    m22 Adj = Adjugate(M);
    m22 Result = (1.0 / Det) * Adj;
    return Result;
}

m22
Transpose(m22 M)
{
    m22 Result = M22(M[0][0], M[1][0],
                     M[0][1], M[1][1]);
    return Result;
}

m22
M22Identity(void)
{
    m22 Result = M22(1, 0,
                     0, 1);
    return Result;
}
//==================================
// 3x3 Matrix
//==================================

f64
Determinant(m33 M)
{
    f64 a = M[0][0], b = M[0][1], c = M[0][2];
    m22 efhi = M22(V2(M[1][1], M[1][2]), V2(M[2][1], M[2][2]));
    m22 dfgi = M22(V2(M[1][0], M[1][2]), V2(M[2][0], M[2][2]));
    m22 degh = M22(V2(M[1][0], M[1][1]), V2(M[2][0], M[2][1]));
    
    f64 Result = a*Determinant(efhi) - b*Determinant(dfgi) + c*Determinant(degh);
    return Result;
}

m33
Adjugate(m33 M)
{
    // First row
    f64 a = Determinant(M22(V2(M[1][1], M[1][2]),
                            V2(M[2][1], M[2][2])));
    f64 b = Determinant(M22(V2(M[1][0], M[1][2]),
                            V2(M[2][0], M[2][2])));
    f64 c = Determinant(M22(V2(M[1][0], M[1][1]),
                            V2(M[2][0], M[2][1])));
    
    // Second Row
    f64 d = Determinant(M22(V2(M[0][1], M[0][2]),
                            V2(M[2][1], M[2][2])));
    f64 e = Determinant(M22(V2(M[0][0], M[0][2]),
                            V2(M[2][0], M[2][2])));
    f64 f = Determinant(M22(V2(M[0][0], M[0][1]),
                            V2(M[2][0], M[2][1])));
    
    // Third Row
    f64 g = Determinant(M22(V2(M[0][1], M[0][2]),
                            V2(M[1][1], M[1][2])));
    f64 h = Determinant(M22(V2(M[0][0], M[0][2]),
                            V2(M[1][0], M[1][2])));
    f64 i = Determinant(M22(V2(M[0][0], M[0][1]),
                            V2(M[1][0], M[1][1])));
    
    m33 Result = M33(V3(a, -b, c), V3(-d, e, -f), V3(g, -h, i));
    return Result;
}

m33
Inverse(m33 M)
{
    f64 Det = Determinant(M);
    m33 Adj = Adjugate(M);
    m33 Result = (1.0 / Det) * Adj;
    return Result;
}

m33
Transpose(m33 M)
{
    m33 Result = M33(M[0][0], M[1][0], M[2][0],
                     M[0][1], M[1][1], M[2][1],
                     M[0][2], M[1][2], M[2][2]);
    return Result;
}

m33
M33Identity(void)
{
    m33 Result = M33(1, 0, 0,
                     0, 1, 0,
                     0, 0, 1);
    return Result;
}

m33
M33Rotate(f64 Angle)
{
    f64 C = Cos(Angle); // Must be in radians.
    f64 S = Sin(Angle); // Must be in radians.
    m33 Result = M33(C, -S, 0,
                     S,  C, 0,
                     0,  0, 1);
    return Result;
}

m33
M33Scale(v2 V)
{
    f64 X = V.X, Y = V.Y;
    m33 Result = M33(X, 0, 0,
                     0, Y, 0,
                     0, 0, 1);
    return Result;
}

m33
M33Translate(v2 V)
{
    f64 X = V.X, Y = V.Y;
    m33 Result = M33(1, 0, X,
                     0, 1, Y,
                     0, 0, 1);
    return Result;
}

m33
M33Shear(v2 V)
{
    f64 X = V.X, Y = V.Y;
    m33 Result = M33(1, Y, 0,
                     X, 1, 0,
                     0, 0, 1);
    return Result;
}

m33
M33FlipX(void)
{
    m33 Result = M33(-1, 0, 0,
                     +0, 1, 0,
                     +0, 0, 1);
    return Result;
}

m33
M33FlipY(void)
{
    m33 Result = M33(1,  0, 0,
                     0, -1, 0,
                     0,  0, 1);
    return Result;
}

//==================================
// 4x4 Matrix
//==================================

f64
Determinant(m44 M)
{
    f64 a = M[0][0], b = M[0][1], c = M[0][2], d = M[0][3];
    m33 fghjklnop = M33(V3(M[1][1], M[1][2], M[1][3]),
                        V3(M[2][1], M[2][2], M[2][3]),
                        V3(M[3][1], M[3][2], M[3][3]));
    m33 eghiklmop = M33(V3(M[1][0], M[1][2], M[1][3]),
                        V3(M[2][0], M[2][2], M[2][3]),
                        V3(M[3][0], M[3][2], M[3][3]));
    m33 efhijlmnp = M33(V3(M[1][0], M[1][1], M[1][3]),
                        V3(M[2][0], M[2][1], M[2][3]),
                        V3(M[3][0], M[3][1], M[3][3]));
    m33 efgijkmno = M33(V3(M[1][0], M[1][1], M[1][2]),
                        V3(M[2][0], M[2][1], M[2][2]),
                        V3(M[3][0], M[3][1], M[3][2]));
    
    f64 Result = (a*Determinant(fghjklnop) - b*Determinant(eghiklmop)
                  + c*Determinant(efhijlmnp) - d*Determinant(efgijkmno));
    return Result;
}

m44
Adjugate(m44 M)
{
    // First row
    f64 a = Determinant(M33(V3(M[1][1], M[1][2], M[1][3]),
                            V3(M[2][1], M[2][2], M[2][3]),
                            V3(M[3][1], M[3][2], M[3][3])));
    f64 b = Determinant(M33(V3(M[1][0], M[1][2], M[1][3]),
                            V3(M[2][0], M[2][2], M[2][3]),
                            V3(M[3][0], M[3][2], M[3][3])));
    f64 c = Determinant(M33(V3(M[1][0], M[1][1], M[1][3]),
                            V3(M[2][0], M[2][1], M[2][3]),
                            V3(M[3][0], M[3][1], M[3][3])));
    f64 d = Determinant(M33(V3(M[1][0], M[1][1], M[1][2]),
                            V3(M[2][0], M[2][1], M[2][2]),
                            V3(M[3][0], M[3][1], M[3][2])));
    
    // Second Row
    f64 e = Determinant(M33(V3(M[0][1], M[0][2], M[0][3]),
                            V3(M[2][1], M[2][2], M[2][3]),
                            V3(M[3][1], M[3][2], M[3][3])));
    f64 f = Determinant(M33(V3(M[0][0], M[0][2], M[0][3]),
                            V3(M[2][0], M[2][2], M[2][3]),
                            V3(M[3][0], M[3][2], M[3][3])));
    f64 g = Determinant(M33(V3(M[0][0], M[0][1], M[0][3]),
                            V3(M[2][0], M[2][1], M[2][3]),
                            V3(M[3][0], M[3][1], M[3][3])));
    f64 h = Determinant(M33(V3(M[0][0], M[0][1], M[0][2]),
                            V3(M[2][0], M[2][1], M[2][2]),
                            V3(M[3][0], M[3][1], M[3][2])));
    
    // Third Row
    f64 i = Determinant(M33(V3(M[0][1], M[0][2], M[0][3]),
                            V3(M[1][1], M[1][2], M[1][3]),
                            V3(M[3][1], M[3][2], M[3][3])));
    f64 j = Determinant(M33(V3(M[0][0], M[0][2], M[0][3]),
                            V3(M[1][0], M[1][2], M[1][3]),
                            V3(M[3][0], M[3][2], M[3][3])));
    f64 k = Determinant(M33(V3(M[0][0], M[0][1], M[0][3]),
                            V3(M[1][0], M[1][1], M[1][3]),
                            V3(M[3][0], M[3][1], M[3][3])));
    f64 l = Determinant(M33(V3(M[0][0], M[0][1], M[0][2]),
                            V3(M[1][0], M[1][1], M[1][2]),
                            V3(M[3][0], M[3][1], M[3][2])));
    
    // Fourth Row
    f64 m = Determinant(M33(V3(M[0][1], M[0][2], M[0][3]),
                            V3(M[1][1], M[1][2], M[1][3]),
                            V3(M[2][1], M[2][2], M[2][3])));
    f64 n = Determinant(M33(V3(M[0][0], M[0][2], M[0][3]),
                            V3(M[1][0], M[1][2], M[1][3]),
                            V3(M[2][0], M[2][2], M[2][3])));
    f64 o = Determinant(M33(V3(M[0][0], M[0][1], M[0][3]),
                            V3(M[1][0], M[1][1], M[1][3]),
                            V3(M[2][0], M[2][1], M[2][3])));
    f64 p = Determinant(M33(V3(M[0][0], M[0][1], M[0][2]),
                            V3(M[1][0], M[1][1], M[1][2]),
                            V3(M[2][0], M[2][1], M[2][2])));
    
    m44 Result = M44(V4(a, -b, c, -d), V4(-e, f, -g, h), V4(i, -j, k, -l), V4(-m, n, -o, p));
    return Result;
}

m44
Inverse(m44 M)
{
    f64 Det = Determinant(M);
    m44 Adj = Adjugate(M);
    m44 Result = (1.0 / Det) * Adj;
    return Result;
}

m44
Transpose(m44 M)
{
    m44 Result = M44(M[0][0], M[1][0], M[2][0], M[3][0],
                     M[0][1], M[1][1], M[2][1], M[3][1],
                     M[0][2], M[1][2], M[2][2], M[3][2],
                     M[0][3], M[1][3], M[2][3], M[3][3]);
    return Result;
}

m44
M44Identity(void)
{
    m44 Result = M44(1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1);
    return Result;
}

m44
M44Rotate(f64 Angle, v3 RotationAxis)
{
    f64 C = Cos(Angle); // Must be in radians.
    f64 IC = 1 - C;
    f64 S = Sin(Angle); // Must be in radians.
    f64 X = RotationAxis.X, Y = RotationAxis.Y, Z = RotationAxis.Z;
    
    m44 Result = M44(C + X*X*IC   , X*Y*IC - Z*S , X*Z*IC + Y*S , 0,
                     Y*X*IC + Z*S , C + Y*Y*IC   , Y*Z*IC - X*S , 0,
                     Z*X*IC - Y*S , Z*Y*IC + X*S , C + Z*Z*IC   , 0,
                     0            , 0            , 0            , 1);
    return Result;
}

m44
M44RotateX(f64 Angle)
{
    f64 C = Cos(Angle); // Must be in radians.
    f64 S = Sin(Angle); // Must be in radians.
    
    m44 Result = M44(1, 0,  0, 0,
                     0, C, -S, 0,
                     0, S,  C, 0,
                     0, 0,  0, 1);
    return Result;
}

m44
M44RotateY(f64 Angle)
{
    f64 C = Cos(Angle); // Must be in radians.
    f64 S = Sin(Angle); // Must be in radians.
    
    m44 Result = M44(C,  0, S, 0,
                     0,  1, 0, 0,
                     -S, 0, C, 0,
                     0,  0, 0, 1);
    return Result;
}

m44
M44RotateZ(f64 Angle)
{
    f64 C = Cos(Angle); // Must be in radians.
    f64 S = Sin(Angle); // Must be in radians.
    
    m44 Result = M44(C, -S, 0, 0,
                     S,  C, 0, 0,
                     0,  0, 1, 0,
                     0,  0, 0, 1);
    return Result;
}

m44
M44Scale(v3 V)
{
    f64 X = V.X, Y = V.Y, Z = V.Z;
    m44 Result = M44(X, 0, 0, 0,
                     0, Y, 0, 0,
                     0, 0, Z, 0,
                     0, 0, 0, 1);
    return Result;
}

m44
M44Translate(v3 V)
{
    f64 X = V.X, Y = V.Y, Z = V.Z;
    m44 Result = M44(1, 0, 0, X,
                     0, 1, 0, Y,
                     0, 0, 1, Z,
                     0, 0, 0, 1);
    return Result;
}

m44
M44FlipX(void)
{
    m44 Result = M44(-1, 0, 0, 0,
                     +0, 1, 0, 0,
                     +0, 0, 1, 0,
                     +0, 0, 0, 1);
    return Result;
}

m44
M44FlipY(void)
{
    m44 Result = M44(1,  0, 0, 0,
                     0, -1, 0, 0,
                     0,  0, 1, 0,
                     0,  0, 0, 1);
    return Result;
}

m44
M44FlipZ(void)
{
    m44 Result = M44(1, 0,  0, 0,
                     0, 1,  0, 0,
                     0, 0, -1, 0,
                     0, 0,  0, 1);
    return Result;
}


//==================================
// Circle
//==================================

circle
Circle(v2 A, v2 B, f64 Radius)
{
    v2 MidPoint = A + ((B-A) * 0.5);
    v2 HalfBaseUnit = Unit(B - MidPoint);
    v2 HeightUnit = Radius > 0 ? Rotate90CCW(HalfBaseUnit) : Rotate90CW(HalfBaseUnit);
    f64 HalfBaseLen = Dist(A, B) / 2;
    f64 HeightLen = Sqrt(Radius*Radius - HalfBaseLen*HalfBaseLen);
    v2 Height = HeightUnit * HeightLen;
    v2 Centre = MidPoint + Height;
    
    return Circle(Centre, Abs(Radius));
}

f64
Length(circle C)
{
    f64 Result = TAU * C.Radius;
    return Result;
}

f64
Area(circle C)
{
    f64 Result = PI * C.Radius * C.Radius;
    return Result;
}

f64
ArcLength(f64 Angle, circle C)
{
    f64 Result = Angle * C.Radius;
    return Result;
}

f64
ArcLength(v2 A, v2 B, circle C)
{
    f64 Base = Dist(A, B);
    f64 Circ2 = 2 * C.Radius * C.Radius;
    f64 Angle = ACos((Circ2 - Base*Base) / Circ2); // Law of cosines.
    f64 Result = ArcLength(Angle, C);
    return Result;
}

v2
GetPointOfAngle(f64 Angle, circle C)
{
    v2 Result = C.Centre + (C.Radius * V2(cos(Angle), sin(Angle)));
    return Result;
}

f64
GetAngleOfPoint(v2 A, circle C)
{
    v2 CAUnit = Unit(A - C.Centre);
    f64 Result = ACos(CAUnit.X) * (CAUnit.Y > 0 ? 1 : -1);
    return Result;
}

bool
PointInCircle(v2 P, circle C)
{
    v2 Delta = C.Centre - P;
    f64 A = Dot(Delta, Delta);
    f64 B = C.Radius * C.Radius;
    bool Result = A <= B;
    return Result;
}

//==================================
// BBox 2D
//==================================

f64
Area(bbox2 A)
{
    f64 Result = (A.Max.X - A.Min.X) * (A.Max.Y - A.Min.Y);
    return Result;
}

bbox2
Merge(bbox2 A, bbox2 B)
{
    bbox2 Result = BBox2(Min(A.Min.X, B.Min.X), Min(A.Min.Y, B.Min.Y),
                         Max(A.Max.X, B.Max.X), Max(A.Max.Y, B.Max.Y));
    return Result;
}

bool
Intersects(bbox2 A, bbox2 B)
{
    bool Result = (A.Min.X <= B.Max.X && A.Max.X >= B.Min.X && A.Min.Y <= B.Max.Y && A.Max.Y >= B.Min.Y);
    return Result;
}

bool
Intersects(v2 P, bbox2 B)
{
    bool Result = (P.X <= B.Max.X && P.X >= B.Min.X && P.Y <= B.Max.Y && P.Y >= B.Min.Y);
    return Result;
}

bool
Intersects(bbox2 B, v2 P) 
{
    return Intersects(P, B);
}

bool
Intersects(circle C, bbox2 B)
{
    f64 NearestX = Max(B.Min.X, Min(C.Centre.X, B.Max.X));
    f64 NearestY = Max(B.Min.Y, Min(C.Centre.Y, B.Max.Y));
    bool Result = PointInCircle(V2(NearestX, NearestY), C);
    return Result;
}

bool
Intersects(bbox2 B, circle C)
{
    return Intersects(C, B);
}

//==================================
// BBox 3D
//==================================

f64
AreaXY(bbox3 A)
{
    f64 Result = (A.Max.X - A.Min.X) * (A.Max.Y - A.Min.Y);
    return Result;
}

f64
AreaXZ(bbox3 A)
{
    f64 Result = (A.Max.X - A.Min.X) * (A.Max.Z - A.Min.Z);
    return Result;
}

f64
AreaYZ(bbox3 A)
{
    f64 Result = (A.Max.Y - A.Min.Y) * (A.Max.Z - A.Min.Z);
    return Result;
}

bbox3
Merge(bbox3 A, bbox3 B)
{
    bbox3 Result = BBox3(Min(A.Min.X, B.Min.X), Min(A.Min.Y, B.Min.Y), Min(A.Min.Z, B.Min.Z),
                         Max(A.Max.X, B.Max.X), Max(A.Max.Y, B.Max.Y), Max(A.Max.Z, B.Max.Z));
    return Result;
}

//==================================
// BBox 4D
//==================================

f64
AreaXY(bbox4 A)
{
    f64 Result = (A.Max.X - A.Min.X) * (A.Max.Y - A.Min.Y);
    return Result;
}

f64
AreaXZ(bbox4 A)
{
    f64 Result = (A.Max.X - A.Min.X) * (A.Max.Z - A.Min.Z);
    return Result;
}

f64
AreaYZ(bbox4 A)
{
    f64 Result = (A.Max.Y - A.Min.Y) * (A.Max.Z - A.Min.Z);
    return Result;
}

bbox4
Merge(bbox4 A, bbox4 B)
{
    bbox4 Result = BBox4(Min(A.Min.X, B.Min.X), Min(A.Min.Y, B.Min.Y), Min(A.Min.Z, B.Min.Z), Min(A.Min.M, B.Min.M),
                         Max(A.Max.X, B.Max.X), Max(A.Max.Y, B.Max.Y), Max(A.Max.Z, B.Max.Z), Max(A.Max.M, B.Max.M));
    return Result;
}

//==================================
// Line 2D
//==================================

bool
IsPointLeftOfLine(v2 P, line2 L)
{
    f64 Result = (L.P1.X-L.P0.X)*(P.Y-L.P0.Y) - (L.P1.Y-L.P0.Y)*(P.X-L.P0.X);
    return Result > 0;
}

f64
LocationAlongLine(line2 L, v2 Target)
{
    v2 LineDir = L.P1 - L.P0;
    v2 TargetDir = Target - L.P0;
    v2 LineDirUnit = Unit(LineDir);
    
    f64 Result = Dot(LineDirUnit, TargetDir) / Mag(LineDir);
    return Result;
}

v2
PointAlongLine(line2 L, f64 LineLocation)
{
    v2 Result = ((L.P1 - L.P0) * LineLocation) + L.P0;
    return Result;    
}

f64
Dist(v2 P, line2 L)
{
    f64 LineLocation = Clamp01(LocationAlongLine(L, P));
    v2 Projection = PointAlongLine(L, LineLocation);
    f64 Result = Dist(P, Projection);
    return Result;
}

f64
Dist(line2 L, v2 P)
{
    return Dist(P, L);
}

//==================================
// Line 3D
//==================================

f64
LocationAlongLine(line3 L, v3 Target)
{
    v3 LineDir = L.P1 - L.P0;
    v3 TargetDir = Target - L.P0;
    v3 LineDirUnit = Unit(LineDir);
    
    f64 Result = Dot(LineDirUnit, TargetDir) / Mag(LineDir);
    return Result;
}

v3
PointAlongLine(line3 L, f64 LineLocation)
{
    v3 Result = ((L.P1 - L.P0) * LineLocation) + L.P0;
    return Result;    
}

f64
Dist(v3 P, line3 L)
{
    f64 LineLocation = Clamp01(LocationAlongLine(L, P));
    v3 Projection = PointAlongLine(L, LineLocation);
    f64 Result = Dist(P, Projection);
    return Result;
}

f64
Dist(line3 L, v3 P)
{
    return Dist(P, L);
}
