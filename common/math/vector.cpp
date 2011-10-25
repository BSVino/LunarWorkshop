#include "vector.h"

#include "matrix.h"

bool EAngle::EqualsExhaustive(const EAngle& v, float flEp) const
{
	Matrix4x4 m, n;
	m.SetAngles(*this);
	n.SetAngles(v);

	return m.Equals(n, flEp);
}
