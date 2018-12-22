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

	newV1.x = (*v1).x - (*v2).x;
	newV1.y = (*v1).y - (*v2).y;
	newV1.z = (*v1).z - (*v2).z;

	newV2.x = (*v1).x - (*v3).x;
	newV2.y = (*v1).y - (*v3).y;
	newV2.z = (*v1).z - (*v3).z;

	newCross = Cross(&newV1, &newV2);

	float mag = sqrt(newCross.x * newCross.x + newCross.y * newCross.y + newCross.z * newCross.z);

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

	r.x = v2->x - v1->x;
	r.y = v2->y - v1->y;
	r.z = v2->z - v1->z;
	t = (-(p->d) - Dot(&(p->normal), v1)) / Dot(&(p->normal), &r);

	inter.x = v1->x + (r.x * t);
	inter.y = v1->y + (r.y * t);
	inter.z = v1->z + (r.z * t);

	return inter;
}

bool maths::InTriangle(xyz* t0, xyz* t1, xyz* t2, xyz* point)
{


}