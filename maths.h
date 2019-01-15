#pragma once
#include "objfilemodel.h"

struct Plane
{
	xyz normal;					// normal of the plane
	float d;					// the d constant in the equation for this plane
};

class maths
{
public:
	maths();
	~maths();
	static float MagnitudeSqr(xyz* v1);
	static float Dot(xyz* v1, xyz* v2);
	static xyz Cross(xyz* v1, xyz* v2);
	static xyz Normal(xyz* v1, xyz* v2, xyz* v3);
	static Plane CalcPlane(xyz* v1, xyz* v2, xyz* v3);
	static float CalcPlanePoint(Plane* p1, xyz* v1);
	static xyz PlaneIntersection(Plane* p, xyz* v1, xyz* v2);
	static bool InTriangle(xyz* t0, xyz* t1, xyz* t2, xyz* point);
	static int Sign(float number);

	static xyz SetXYZ(float x, float y, float z);
	static xyz SetXYZ(xyz* v);
	static xyz ScaleXYZ(xyz* v, float scalar);
	static xyz ScaleXYZ(xyz* v1, xyz* v2);
	static xyz AddXYZ(xyz* v1, xyz* v2);
	static xyz SubtractXYZ(xyz* v1, xyz* v2);
	static bool CompareXYZ(xyz* v1, xyz* v2);
	static xyz NormaliseXYZ(xyz* v1);
};
