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
	float Dot(xyz* v1, xyz* v2);
	xyz Cross(xyz* v1, xyz* v2);
	xyz Normal(xyz* v1, xyz* v2, xyz* v3);
	Plane CalcPlane(xyz* v1, xyz* v2, xyz* v3);
	float CalcPlanePoint(Plane* p1, xyz* v1);
	xyz PlaneIntersection(Plane* p, xyz* v1, xyz* v2);
	bool InTriangle(xyz* t0, xyz* t1, xyz* t2, xyz* point);
	int Sign(float number);

};

