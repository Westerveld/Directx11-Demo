#include "maths.h"



maths::maths()
{
}


maths::~maths()
{
}

float maths::Dot(xyz* v1, xyz* v2)
{
	float dot;
	
	dot = ((*v1).x * (*v2).x) + ((*v1).y * (*v2).y) + ((*v1).z * (*v2).z);
	return dot;
}

xyz maths::Cross(xyz* v1, xyz* v2)
{
	xyz newV;
	newV.x = ((*v1).y * (*v2).z) - ((*v1).z * (*v2).y);
	newV.y = ((*v1).z * (*v2).x) - ((*v1).x * (*v2).z);
	newV.z = ((*v1).x * (*v2).y) - ((*v1).y * (*v2).x);
	return newV;
}

xyz maths::Normal(xyz* v1, xyz* v2, xyz* v3)
{
	xyz newV1, newV2, newCross;

	//Calculate the first new vector
	newV1.x = (*v1).x - (*v2).x;
	newV1.y = (*v1).y - (*v2).y;
	newV1.z = (*v1).z - (*v2).z;

	//Calculate the second new vector
	newV2.x = (*v1).x - (*v3).x;
	newV2.y = (*v1).y - (*v3).y;
	newV2.z = (*v1).z - (*v3).z;

	//Calculate the cross product
	newCross = Cross(&newV1, &newV2);

	//Work out the magnitude of the cross product
	float mag = sqrt(newCross.x * newCross.x + newCross.y * newCross.y + newCross.z * newCross.z);

	//Divide the vector by the magnitude
	newCross.x /= mag;
	newCross.y /= mag;
	newCross.z /= mag;

	return newCross;
	
}

Plane maths::CalcPlane(xyz* v1, xyz* v2, xyz* v3)
{
	Plane plane;
	plane.normal = Normal(v1, v2, v3);
	plane.d = Dot(v2, v3);

	return plane;

}

float maths::CalcPlanePoint(Plane* p1, xyz* v1)
{
	float result = Dot(&(p1)->normal, v1);
	result += (*p1).d;
	return result;
}

int maths::Sign(float number)
{
	return (number < 0.0f ? -1 : (number > 0.0f ? 1 : 0));
}

xyz maths::PlaneIntersection(Plane* p, xyz* v1, xyz* v2)
{
	xyz r, inter;
	float t;

	r = SetXYZ(v2->x - v1->x, v2->y - v1->y, v2->z - v1->z);
	
	t = (-(p->d) - Dot(&(p->normal), v1)) / Dot(&(p->normal), &r);

	if (t < 0.0 || t > 1.0)
	{
		inter.x = 9999999999999;
		inter.y = 9999999999999;
		inter.z = 9999999999999;
	}
	else 
	{
		inter.x = v1->x + (r.x * t);
		inter.y = v1->y + (r.y * t);
		inter.z = v1->z + (r.z * t);
	}
	return inter;
}

bool maths::InTriangle(xyz* t0, xyz* t1, xyz* t2, xyz* point)
{
	Plane plane1, plane2, plane3;
	float d1, d2, d3;
	bool hasNeg, hasPos;

	plane1 = CalcPlane(point, t0, t1);
	plane2 = CalcPlane(point, t1, t2);
	plane3 = CalcPlane(point, t2, t0);

	d1 = plane1.d;
	d2 = plane2.d;
	d3 = plane3.d;

	hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(hasNeg && hasPos);
}

xyz maths::SetXYZ(float x, float y, float z)
{
	xyz newXYZ;
	newXYZ.x = x;
	newXYZ.y = y;
	newXYZ.z = z;
	return newXYZ;
}