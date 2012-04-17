#pragma once

#include "vector.h"
#include "matrix.h"

class TRS
{
public:
				TRS();
				TRS(const Vector& T, const EAngle& R, const Vector& S);

public:
	Matrix4x4	GetMatrix4x4(bool bRotation = true, bool bScaling = true) const;

public:
	Vector		m_vecTranslation;
	EAngle		m_angRotation;
	Vector		m_vecScaling;
};

inline TRS::TRS()
{
	m_vecScaling = Vector(1, 1, 1);
}

inline TRS::TRS(const Vector& T, const EAngle& R, const Vector& S)
{
	m_vecTranslation = T;
	m_angRotation = R;
	m_vecScaling = S;
}

inline Matrix4x4 TRS::GetMatrix4x4(bool bRotation, bool bScaling) const
{
	Matrix4x4 m;
	m.SetTranslation(m_vecTranslation);

	if (bRotation)
		m.SetAngles(m_angRotation);

	if (bScaling)
		m.AddScale(m_vecScaling);

	return m;
}
