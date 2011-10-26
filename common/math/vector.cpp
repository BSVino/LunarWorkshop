#include "vector.h"

#include "matrix.h"

bool EAngle::EqualsExhaustive(const EAngle& v, float flEp) const
{
	Matrix4x4 m, n;
	m.SetAngles(*this);
	n.SetAngles(v);

	return m.Equals(n, flEp);
}

void AngleVectors(const EAngle& a, Vector* pvecF, Vector* pvecU, Vector* pvecR)
{
	Matrix4x4 m;
	m.SetAngles(a);

	if (pvecF)
		*pvecF = m.GetForwardVector();

	if (pvecU)
		*pvecU = m.GetUpVector();

	if (pvecR)
		*pvecR = m.GetRightVector();
}
