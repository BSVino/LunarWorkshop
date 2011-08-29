#include "sp_common.h"

#include <matrix.h>
#include <tstring.h>

#ifdef _DEBUG
#define SANITY_CHECK 1
#endif

#if SANITY_CHECK == 1
#define CHECKSANITY(f) (f).CheckSanity();
#else
#define CHECKSANITY(f)
#endif

CScalableFloat::CScalableFloat()
{
	memset(&m_aiScaleStack[0], 0, SCALESTACK_SIZE*sizeof(short));
	m_bZero = true;
	m_flRemainder = 0;
	m_flOverflow = 0;
}

CScalableFloat::CScalableFloat(float flUnits, scale_t eScale)
{
	memset(&m_aiScaleStack[0], 0, SCALESTACK_SIZE*sizeof(short));

	m_flRemainder = 0;
	m_flOverflow = 0;

	if (flUnits == 0)
	{
		m_bZero = true;
		return;
	}

	m_bZero = false;

	int i = SCALESTACK_INDEX(eScale);
	m_bPositive = (flUnits > 0);

	if (m_bPositive)
	{
		while (flUnits < 1)
		{
			flUnits *= 1000;
			i--;
		}

		while (flUnits > 1000)
		{
			flUnits /= 1000;
			i++;
		}

		while (flUnits > 0 && i >= 0)
		{
			m_aiScaleStack[i] = (short)flUnits;
			flUnits -= m_aiScaleStack[i];
			flUnits *= 1000;
			i--;
		}

		m_flRemainder = flUnits/1000;
	}
	else
	{
		while (flUnits > -1)
		{
			flUnits *= 1000;
			i--;
		}

		while (flUnits < -1000)
		{
			flUnits /= 1000;
			i++;
		}

		while (flUnits < 0 && i >= 0)
		{
			m_aiScaleStack[i] = (short)flUnits;
			flUnits -= m_aiScaleStack[i];
			flUnits *= 1000;
			i--;
		}

		m_flRemainder = flUnits/1000;
	}

	CHECKSANITY(*this);
}

CScalableFloat::CScalableFloat(double flUnits, scale_t eScale)
{
	memset(&m_aiScaleStack[0], 0, SCALESTACK_SIZE*sizeof(short));

	m_flRemainder = 0;
	m_flOverflow = 0;

	if (flUnits == 0)
	{
		m_bZero = true;
		return;
	}

	m_bZero = false;

	int i = SCALESTACK_INDEX(eScale);
	m_bPositive = (flUnits > 0);

	if (m_bPositive)
	{
		while (flUnits < 1)
		{
			flUnits *= 1000;
			i--;
		}

		while (flUnits > 1000)
		{
			flUnits /= 1000;
			i++;
		}

		while (flUnits > 0 && i >= 0)
		{
			m_aiScaleStack[i] = (short)flUnits;
			flUnits -= m_aiScaleStack[i];
			flUnits *= 1000;
			i--;
		}

		m_flRemainder = flUnits/1000;
	}
	else
	{
		while (flUnits > -1)
		{
			flUnits *= 1000;
			i--;
		}

		while (flUnits < -1000)
		{
			flUnits /= 1000;
			i++;
		}

		while (flUnits < 0 && i >= 0)
		{
			m_aiScaleStack[i] = (short)flUnits;
			flUnits -= m_aiScaleStack[i];
			flUnits *= 1000;
			i--;
		}

		m_flRemainder = flUnits/1000;
	}

	CHECKSANITY(*this);
}

double CScalableFloat::GetUnits(scale_t eScale) const
{
	if (m_bZero)
		return 0;

	double flResult = 0;
	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
		flResult += ConvertUnits(m_aiScaleStack[i], (scale_t)(i+1), eScale);

	flResult += ConvertUnits(m_flRemainder, (scale_t)(SCALE_NONE+1), eScale);
	flResult += ConvertUnits(m_flOverflow*1000, SCALE_HIGHEST, eScale);

	return flResult;
}

bool CScalableFloat::IsZero() const
{
	if (m_bZero)
		return true;

	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
	{
		if (m_aiScaleStack[i] != 0)
			return false;
	}

	return true;
}

bool CScalableFloat::IsZero()
{
	if (m_bZero)
		return true;

	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
	{
		if (m_aiScaleStack[i] != 0)
			return false;
	}

	m_bZero = true;
	return true;
}

CScalableFloat CScalableFloat::operator-() const
{
	CScalableFloat f;

	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
		f.m_aiScaleStack[i] = -m_aiScaleStack[i];

	f.m_flRemainder = -m_flRemainder;
	f.m_flOverflow = -m_flOverflow;
	f.m_bPositive = !m_bPositive;
	f.m_bZero = m_bZero;

	CHECKSANITY(f);

	return f;
}

CScalableFloat CScalableFloat::AddSimilar(const CScalableFloat& u) const
{
	TAssert(m_bPositive == u.m_bPositive);

	CScalableFloat flReturn;
	flReturn.m_bPositive = m_bPositive;
	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
	{
		flReturn.m_aiScaleStack[i] += m_aiScaleStack[i] + u.m_aiScaleStack[i];
		flReturn.NormalizeStackPosition(i);
	}

	flReturn.m_bZero = m_bZero && u.m_bZero;

	flReturn.m_flRemainder = m_flRemainder + u.m_flRemainder;
	flReturn.NormalizeRemainder();

	flReturn.m_flOverflow = m_flOverflow + u.m_flOverflow;

	CHECKSANITY(flReturn);

	return flReturn;
}

CScalableFloat CScalableFloat::AddDifferent(const CScalableFloat& u) const
{
	TAssert(m_bPositive != u.m_bPositive);

	CScalableFloat flReturn;
	flReturn.m_bPositive = m_bPositive;

	bool bHighest = true;

	flReturn.m_flOverflow = m_flOverflow + u.m_flOverflow;
	if (flReturn.m_flOverflow > 0)
	{
		bHighest = false;
		flReturn.m_bPositive = true;
	}
	else if (flReturn.m_flOverflow < 0)
	{
		bHighest = false;
		flReturn.m_bPositive = false;
	}

	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		short iResult = m_aiScaleStack[i] + u.m_aiScaleStack[i];
		if (iResult)
		{
			if (bHighest)
			{
				flReturn.m_bPositive = iResult > 0;
				flReturn.m_aiScaleStack[i] = iResult;
				bHighest = false;
			}
			else
			{
				flReturn.m_aiScaleStack[i] = iResult;

				if (flReturn.m_bPositive)
				{
					int j = i;
					while (flReturn.m_aiScaleStack[j] < 0)
					{
						while (j < SCALESTACK_SIZE-1 && flReturn.m_aiScaleStack[j+1] == 0)
						{
							flReturn.m_aiScaleStack[j+1] += 999;
							j++;
						}

						if (j == SCALESTACK_SIZE-1)
							flReturn.m_flOverflow -= 1;
						else
							flReturn.m_aiScaleStack[j+1]--;

						flReturn.m_aiScaleStack[i] += 1000;
					}
				}
				else if (!flReturn.m_bPositive)
				{
					int j = i;
					while (flReturn.m_aiScaleStack[i] > 0)
					{
						while (j < SCALESTACK_SIZE-1 && flReturn.m_aiScaleStack[j+1] == 0)
						{
							flReturn.m_aiScaleStack[j+1] -= 999;
							j++;
						}

						if (j == SCALESTACK_SIZE-1)
							flReturn.m_flOverflow += 1;
						else
							flReturn.m_aiScaleStack[j+1]++;

						flReturn.m_aiScaleStack[i] -= 1000;
					}
				}
			}
		}
		else
			flReturn.m_aiScaleStack[i] = 0;
	}

	flReturn.m_bZero = m_bZero && u.m_bZero;
	
	flReturn.m_flRemainder = m_flRemainder + u.m_flRemainder;

	// It's all zero
	if (bHighest)
	{
		if (flReturn.m_flRemainder == 0)
			flReturn.m_bZero = true;
		else
		{
			flReturn.m_bZero = false;
			flReturn.m_bPositive = flReturn.m_flRemainder > 0;
		}
	}

	flReturn.NormalizeRemainder();

	CHECKSANITY(flReturn);

	return flReturn;
}

CScalableFloat CScalableFloat::operator+(const CScalableFloat& u) const
{
	if (m_bPositive == u.m_bPositive)
		return AddSimilar(u);
	else
		return AddDifferent(u);
}

CScalableFloat CScalableFloat::operator-(const CScalableFloat& u) const
{
	if (m_bZero)
		return -u;

	if (u.m_bZero)
		return *this;

	if (m_bPositive == u.m_bPositive)
		return AddDifferent(-u);
	else
		return AddSimilar(-u);
}

void CScalableFloat::operator+=(const CScalableFloat& u)
{
	if (m_bPositive == u.m_bPositive)
		*this = AddSimilar(u);
	else
		*this = AddDifferent(u);
}

void CScalableFloat::operator-=(const CScalableFloat& u)
{
	if (m_bPositive == u.m_bPositive)
		*this = AddDifferent(-u);
	else
		*this = AddSimilar(-u);
}

CScalableFloat CScalableFloat::operator*( const CScalableFloat& f ) const
{
	// Since I hard-coded the code below, this would have to be updated if I add a scale.
	TAssert(SCALESTACK_SIZE == SCALE_TERAMETER);

	double aflResults[13];

	{
		double fll0 = (double)m_aiScaleStack[0] + m_flRemainder;
		double fll1 = m_aiScaleStack[1];
		double fll2 = m_aiScaleStack[2];
		double fll3 = m_aiScaleStack[3];
		double fll4 = m_aiScaleStack[4];
		double fll5 = m_aiScaleStack[5];
		double fll6 = m_flOverflow;

		double flr0 = (double)f.m_aiScaleStack[0] + f.m_flRemainder;
		double flr1 = f.m_aiScaleStack[1];
		double flr2 = f.m_aiScaleStack[2];
		double flr3 = f.m_aiScaleStack[3];
		double flr4 = f.m_aiScaleStack[4];
		double flr5 = f.m_aiScaleStack[5];
		double flr6 = f.m_flOverflow;

		aflResults[0] = fll0 * flr0;
		aflResults[1] = fll1 * flr0 + fll0 * flr1;
		aflResults[2] = fll2 * flr0 + fll1 * flr1 + fll0 * flr2;
		aflResults[3] = fll3 * flr0 + fll2 * flr1 + fll1 * flr2 + fll0 * flr3;
		aflResults[4] = fll4 * flr0 + fll3 * flr1 + fll2 * flr2 + fll1 * flr3 + fll0 * flr4;
		aflResults[5] = fll5 * flr0 + fll4 * flr1 + fll3 * flr2 + fll2 * flr3 + fll1 * flr4 + fll0 * flr5;
		aflResults[6] = fll6 * flr0 + fll5 * flr1 + fll4 * flr2 + fll3 * flr3 + fll2 * flr4 + fll1 * flr5 + fll0 * flr6;
		aflResults[7] = fll6 * flr1 + fll5 * flr2 + fll4 * flr3 + fll3 * flr4 + fll2 * flr5 + fll1 * flr6;
		aflResults[8] = fll6 * flr2 + fll5 * flr3 + fll4 * flr4 + fll3 * flr5 + fll2 * flr6;
		aflResults[9] = fll6 * flr3 + fll5 * flr4 + fll4 * flr5 + fll3 * flr6;
		aflResults[10] = fll6 * flr4 + fll5 * flr5 + fll4 * flr6;
		aflResults[11] = fll6 * flr5 + fll5 * flr6;
		aflResults[12] = fll6 * flr6;
	}

	CScalableFloat flReturn;
	flReturn.m_bPositive = f.m_bPositive?m_bPositive:!m_bPositive;
	flReturn.m_bZero = true;

	for (size_t i = 0; i < 11; i++)
	{
		int j = i;
		double flValue = aflResults[j];

		if (flValue == 0)
			continue;

		// Subtract 1 for every scale below meters to keep the decimal in the right place.
		j -= (SCALE_METER-1);

		flReturn.m_bZero = false;

		if (flReturn.m_bPositive)
		{
			while (flValue < 1)
			{
				flValue *= 1000;
				j--;
			}

			while (flValue > 1000)
			{
				flValue /= 1000;
				j++;
			}

			if (j >= SCALESTACK_SIZE)
			{
				int k = j;
				while (k-- > SCALESTACK_SIZE)
					flValue *= 1000;
				if (flValue < 2000000000 && flValue > -2000000000)
				{
					flReturn.m_flOverflow += (long)flValue;
					flValue -= (long)flValue;
					flValue *= 1000;
				}
				else
				{
					flReturn.m_flOverflow += flValue;
					flValue = 0;
				}
				j = SCALESTACK_SIZE-1;
			}

			while (flValue > 0 && j >= 0)
			{
				flReturn.m_aiScaleStack[j] += (short)flValue;
				flValue -= (short)flValue;
				flValue *= 1000;
				flReturn.NormalizeStackPosition(j);
				j--;
			}

			while (j < -1)
			{
				flValue /= 1000;
				j++;
			}

			flReturn.m_flRemainder += flValue/1000;
		}
		else
		{
			while (flValue > -1)
			{
				flValue *= 1000;
				j--;
			}

			while (flValue < -1000)
			{
				flValue /= 1000;
				j++;
			}

			if (j >= SCALESTACK_SIZE)
			{
				int k = j;
				while (k-- > SCALESTACK_SIZE)
					flValue *= 1000;
				if (flValue < 2000000000 && flValue > -2000000000)
				{
					flReturn.m_flOverflow += (long)flValue;
					flValue -= (long)flValue;
					flValue *= 1000;
				}
				else
				{
					flReturn.m_flOverflow += flValue;
					flValue = 0;
				}
				j = SCALESTACK_SIZE-1;
			}

			while (flValue < 0 && j >= 0)
			{
				flReturn.m_aiScaleStack[j] += (short)flValue;
				flValue -= (short)flValue;
				flValue *= 1000;
				flReturn.NormalizeStackPosition(j);
				j--;
			}

			while (j < -1)
			{
				flValue /= 1000;
				j++;
			}

			flReturn.m_flRemainder += flValue/1000;
		}
	}
	
	flReturn.NormalizeRemainder();

	CHECKSANITY(flReturn);

	return flReturn;
}

CScalableFloat CScalableFloat::operator/( const CScalableFloat& f ) const
{
	if (f.m_bZero)
		return CScalableFloat();

	// I don't like doing this but there's really no way to do a proper division.
	double flDivide = (double)f.GetUnits(SCALE_METER);

	if (flDivide == 0)
		return CScalableFloat();

	CScalableFloat flReturn;

	flReturn.m_bPositive = f.m_bPositive?m_bPositive:!m_bPositive;

	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
	{
		if (m_aiScaleStack[i] == 0)
			continue;

		double flResult = m_aiScaleStack[i]/flDivide;
		int j = i;

		if (flReturn.m_bPositive)
		{
			while (flResult > 0 && flResult < 0.001f && j >= 0)
			{
				flResult *= 1000;
				j--;
			}

			while (flResult > 1000)
			{
				flResult /= 1000;
				j++;
			}

			if (j >= SCALESTACK_SIZE)
			{
				int k = j;
				while (k-- > SCALESTACK_SIZE)
					flResult *= 1000;
				if (flResult < 2000000000 && flResult > -2000000000)
				{
					flReturn.m_flOverflow += (long)flResult;
					flResult -= (long)flResult;
					flResult *= 1000;
				}
				else
				{
					flReturn.m_flOverflow += flResult;
					flResult = 0;
				}
				j = SCALESTACK_SIZE-1;
			}

			while (flResult > 0 && j >= 0)
			{
				flReturn.m_aiScaleStack[j] += (short)flResult;
				flReturn.NormalizeStackPosition(j);
				flResult -= (short)flResult;
				flResult *= 1000;
				j--;
			}
		}
		else
		{
			while (flResult < 0 && flResult > -0.001f && j >= 0)
			{
				flResult *= 1000;
				j--;
			}

			while (flResult < -1000)
			{
				flResult /= 1000;
				j++;
			}

			if (j >= SCALESTACK_SIZE)
			{
				int k = j;
				while (k-- > SCALESTACK_SIZE)
					flResult *= 1000;
				if (flResult > -2000000000)
				{
					flReturn.m_flOverflow += (long)flResult;
					flResult -= (long)flResult;
					flResult *= 1000;
				}
				else
				{
					flReturn.m_flOverflow += flResult;
					flResult = 0;
				}
				j = SCALESTACK_SIZE-1;
			}

			while (flResult < 0 && j >= 0)
			{
				flReturn.m_aiScaleStack[j] = (short)flResult;
				flReturn.NormalizeStackPosition(j);
				flResult -= (short)flResult;
				flResult *= 1000;
				j--;
			}
		}
	}

	flReturn.m_flRemainder = m_flRemainder/flDivide;
	flReturn.NormalizeRemainder();

	flReturn.m_bZero = m_bZero;
	
	CHECKSANITY(flReturn);

	return flReturn;
}

CScalableFloat CScalableFloat::operator*(float f) const
{
	if (f == 0)
		return CScalableFloat();

	if (m_bZero)
		return *this;

	if (f == 1)
		return *this;

	double flOverflow = m_flOverflow*f;
	double flOverflowFractional;
	if (flOverflow < 2000000000 && flOverflow > -2000000000)
	{
		flOverflowFractional = flOverflow - (long)flOverflow;
		flOverflow = (long)flOverflow;
	}
	else
	{
		flOverflowFractional = 0;
	}

	CScalableFloat flReturn(flOverflowFractional*1000, SCALE_HIGHEST);
	flReturn.m_flOverflow = flOverflow;

	flReturn.m_bPositive = (f > 0)?m_bPositive:!m_bPositive;

	flReturn.m_bZero = false;
	
	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
	{
		float flValue = m_aiScaleStack[i] * f;

		if (flValue == 0)
			continue;

		int j = i;

		while (flValue < -1000 || flValue > 1000)
		{
			flValue /= 1000;
			j++;
		}

		if (j >= SCALESTACK_SIZE)
		{
			int k = j;
			while (k-- > SCALESTACK_SIZE)
				flValue *= 1000;
			flReturn.m_flOverflow += (long)flValue;
			flValue -= (long)flValue;
			flValue *= 1000;
			j--;
		}

		while (((flReturn.m_bPositive && flValue > 0) || (!flReturn.m_bPositive && flValue < 0)) && j >= 0)
		{
			flReturn.m_aiScaleStack[j] += (short)flValue;
			while (flReturn.m_aiScaleStack[j] <= -1000 || flReturn.m_aiScaleStack[j] >= 1000)
				flReturn.NormalizeStackPosition(j);
			flValue -= (short)flValue;
			flValue *= 1000;
			j--;
		}

		flReturn.m_flRemainder += flValue/1000;
	}

	flReturn.m_flRemainder += m_flRemainder * f;
	flReturn.NormalizeRemainder();

	CHECKSANITY(flReturn);

	return flReturn;
}

CScalableFloat CScalableFloat::operator/(float f) const
{
	TAssert(f != 0);
	if (f == 0)
		return CScalableFloat();

	if (m_bZero)
		return CScalableFloat();

	CScalableFloat flReturn;
	flReturn.m_bPositive = (f > 0)?m_bPositive:!m_bPositive;

	for (size_t i = 0; i < SCALESTACK_SIZE; i++)
	{
		float flValue = m_aiScaleStack[i] / f;

		int j = i;
		while (fabs(flValue) > 0 && j >= 0)
		{
			flReturn.m_aiScaleStack[j] += (short)flValue;
			flReturn.NormalizeStackPosition(j);
			flValue -= (short)flValue;
			flValue *= 1000;
			j--;
		}
	}

	flReturn.m_bZero = false;

	CHECKSANITY(flReturn);

	return flReturn;
}

CScalableFloat CScalableFloat::AddMultiple(const CScalableFloat& f, const CScalableFloat& g) const
{
	if (m_bZero)
	{
		if (f.m_bZero)
			return g;

		if (g.m_bZero)
			return f;

		return f + g;
	}

	if (f.m_bZero)
	{
		if (m_bZero)
			return g;

		if (g.m_bZero)
			return *this;

		return *this + g;
	}

	if (g.m_bZero)
	{
		if (m_bZero)
			return f;

		if (f.m_bZero)
			return *this;

		return *this + f;
	}

	CScalableFloat flReturn;

	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		flReturn.m_aiScaleStack[i] = m_aiScaleStack[i] + f.m_aiScaleStack[i] + g.m_aiScaleStack[i];

		int j = i;
		while ((flReturn.m_aiScaleStack[j] <= -1000 || flReturn.m_aiScaleStack[j] >= 1000) && j < SCALESTACK_SIZE-2)
		{
			while (flReturn.m_aiScaleStack[j] >= 1000)
			{
				flReturn.m_aiScaleStack[j] -= 1000;
				flReturn.m_aiScaleStack[j+1] += 1;
			}

			while (flReturn.m_aiScaleStack[j] <= -1000)
			{
				flReturn.m_aiScaleStack[j] += 1000;
				flReturn.m_aiScaleStack[j+1] -= 1;
			}

			j++;
		}
	}

	bool bLargest = true;
	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		if (bLargest)
		{
			if (flReturn.m_aiScaleStack[i])
			{
				flReturn.m_bPositive = flReturn.m_aiScaleStack[i] > 0;
				bLargest = false;
			}
		}
		else
		{
			if (flReturn.m_bPositive && flReturn.m_aiScaleStack[i] < 0)
			{
				flReturn.m_aiScaleStack[i] += 1000;
				flReturn.m_aiScaleStack[i+1] -= 1;
			}
			else if (!flReturn.m_bPositive && flReturn.m_aiScaleStack[i] > 0)
			{
				flReturn.m_aiScaleStack[i] -= 1000;
				flReturn.m_aiScaleStack[i+1] += 1;
			}
		}

		while (flReturn.m_aiScaleStack[i] > 1000 || flReturn.m_aiScaleStack[i] < -1000)
			flReturn.NormalizeStackPosition(i);
	}

	flReturn.m_bZero = m_bZero && f.m_bZero && g.m_bZero;

	flReturn.m_flRemainder = m_flRemainder + f.m_flRemainder + g.m_flRemainder;
	flReturn.NormalizeRemainder();

	CHECKSANITY(flReturn);

	return flReturn;
}

CScalableFloat CScalableFloat::AddMultiple(const CScalableFloat& f, const CScalableFloat& g, const CScalableFloat& h) const
{
	CScalableFloat flReturn;

	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		flReturn.m_aiScaleStack[i] = m_aiScaleStack[i] + f.m_aiScaleStack[i] + g.m_aiScaleStack[i] + h.m_aiScaleStack[i];

		int j = i;
		while ((flReturn.m_aiScaleStack[j] <= -1000 || flReturn.m_aiScaleStack[j] >= 1000) && j < SCALESTACK_SIZE-2)
		{
			while (flReturn.m_aiScaleStack[j] >= 1000)
			{
				flReturn.m_aiScaleStack[j] -= 1000;
				flReturn.m_aiScaleStack[j+1] += 1;
			}

			while (flReturn.m_aiScaleStack[j] <= -1000)
			{
				flReturn.m_aiScaleStack[j] += 1000;
				flReturn.m_aiScaleStack[j+1] -= 1;
			}

			j++;
		}
	}

	bool bLargest = true;
	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		if (bLargest)
		{
			if (flReturn.m_aiScaleStack[i])
			{
				flReturn.m_bPositive = flReturn.m_aiScaleStack[i] > 0;
				bLargest = false;
			}
		}
		else
		{
			if (flReturn.m_bPositive && flReturn.m_aiScaleStack[i] < 0)
			{
				while (flReturn.m_aiScaleStack[i] < 0)
				{
					int j = i;
					while (j < SCALESTACK_SIZE-1 && flReturn.m_aiScaleStack[j+1] == 0)
					{
						TAssert(j+1 < SCALESTACK_SIZE-1);
						flReturn.m_aiScaleStack[j+1] += 999;
						j++;
					}

					flReturn.m_aiScaleStack[i] += 1000;
					flReturn.m_aiScaleStack[j+1] -= 1;
				}
			}
			else if (!flReturn.m_bPositive && flReturn.m_aiScaleStack[i] > 0)
			{
				while (flReturn.m_aiScaleStack[i] > 0)
				{
					int j = i;
					while (j < SCALESTACK_SIZE-1 && flReturn.m_aiScaleStack[j+1] == 0)
					{
						TAssert(j+1 < SCALESTACK_SIZE-1);
						flReturn.m_aiScaleStack[j+1] -= 999;
						j++;
					}

					flReturn.m_aiScaleStack[i] -= 1000;
					flReturn.m_aiScaleStack[j+1] += 1;
				}
			}
		}

		while (flReturn.m_aiScaleStack[i] >= 1000 || flReturn.m_aiScaleStack[i] <= -1000)
			flReturn.NormalizeStackPosition(i);
	}

	flReturn.m_bZero = m_bZero && f.m_bZero && g.m_bZero && h.m_bZero;

	CHECKSANITY(flReturn);

	return flReturn;
}

void CScalableFloat::NormalizeStackPosition( int i )
{
	if (m_bPositive)
	{
		for (int j = i; j < SCALESTACK_SIZE-1; j++)
		{
			if (m_aiScaleStack[j] >= 1000)
			{
				m_aiScaleStack[j] -= 1000;
				m_aiScaleStack[j+1] += 1;
			}
			else
				break;
		}

		if (m_aiScaleStack[SCALESTACK_SIZE-1] >= 1000)
		{
			m_aiScaleStack[SCALESTACK_SIZE-1] -= 1000;
			m_flOverflow += 1;
		}
	}
	else
	{
		for (int j = i; j < SCALESTACK_SIZE-1; j++)
		{
			if (m_aiScaleStack[j] <= -1000)
			{
				m_aiScaleStack[j] += 1000;
				m_aiScaleStack[j+1] -= 1;
			}
			else
				break;
		}

		if (m_aiScaleStack[SCALESTACK_SIZE-1] <= -1000)
		{
			m_aiScaleStack[SCALESTACK_SIZE-1] += 1000;
			m_flOverflow -= 1;
		}
	}
}

void CScalableFloat::NormalizeRemainder()
{
	if (m_flRemainder >= 1 || m_flRemainder <= -1)
	{
		m_aiScaleStack[0] += (short)m_flRemainder;
		while (m_aiScaleStack[0] >= 1000 || m_aiScaleStack[0] <= -1000)
			NormalizeStackPosition(0);
		m_flRemainder -= (short)m_flRemainder;
	}

	if (m_flRemainder < FLT_EPSILON && m_flRemainder > -FLT_EPSILON)
	{
		m_flRemainder = 0;
		return;
	}

	if (!m_bZero && m_bPositive && m_flRemainder < 0)
	{
		m_flRemainder += 1;
		m_aiScaleStack[0] -= 1;

		int j = 0;
		while (m_aiScaleStack[j] < 0)
		{
			while (j < SCALESTACK_SIZE-1 && m_aiScaleStack[j+1] == 0)
			{
				TAssert(j+1 < SCALESTACK_SIZE-1);
				m_aiScaleStack[j+1] += 999;
				j++;
			}

			m_aiScaleStack[j+1]--;
			m_aiScaleStack[0] += 1000;
		}
	}
	else if (!m_bZero && !m_bPositive && m_flRemainder > 0)
	{
		m_flRemainder -= 1;
		m_aiScaleStack[0] += 1;

		int j = 0;
		while (m_aiScaleStack[0] > 0)
		{
			while (j < SCALESTACK_SIZE-1 && m_aiScaleStack[j+1] == 0)
			{
				TAssert(j+1 < SCALESTACK_SIZE-1);
				m_aiScaleStack[j+1] -= 999;
				j++;
			}

			m_aiScaleStack[j+1]++;
			m_aiScaleStack[0] -= 1000;
		}
	}
}

void CScalableFloat::CheckSanity()
{
	for (int i = 0; i < SCALESTACK_SIZE; i++)
	{
		if (m_bZero)
		{
			TAssert(m_aiScaleStack[i] == 0);
		}
		else
		{
			if (m_bPositive)
			{
				TAssert(m_aiScaleStack[i] >= 0);
				TAssert(m_aiScaleStack[i] < 1000);
			}
			else
			{
				TAssert(m_aiScaleStack[i] <= 0);
				TAssert(m_aiScaleStack[i] > -1000);
			}
		}
	}

	if (m_bZero)
	{
		TAssert(m_flRemainder == 0);
		TAssert(m_flOverflow == 0);
	}
	else if (m_bPositive)
	{
		TAssert(m_flRemainder >= 0);
		TAssert(m_flRemainder < 1);
		TAssert(m_flOverflow >= 0);
	}
	else
	{
		TAssert(m_flRemainder <= 0);
		TAssert(m_flRemainder > -1);
		TAssert(m_flOverflow <= 0);
	}
}

bool CScalableFloat::operator==(const CScalableFloat& u) const
{
	if (m_flOverflow != u.m_flOverflow)
		return false;

	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		if (m_aiScaleStack[i] != u.m_aiScaleStack[i])
			return false;
	}

	return (m_flRemainder == u.m_flRemainder);
}

bool CScalableFloat::operator<(const CScalableFloat& u) const
{
	if (m_flOverflow != 0 || u.m_flOverflow != 0)
		return m_flOverflow < u.m_flOverflow;

	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		if (m_aiScaleStack[i] - u.m_aiScaleStack[i] == 0)
			continue;

		return m_aiScaleStack[i] < u.m_aiScaleStack[i];
	}

	return (m_flRemainder < u.m_flRemainder);
}

bool CScalableFloat::operator>(const CScalableFloat& u) const
{
	if (m_flOverflow != 0 || u.m_flOverflow != 0)
		return m_flOverflow > u.m_flOverflow;

	for (int i = SCALESTACK_SIZE-1; i >= 0; i--)
	{
		if (m_aiScaleStack[i] - u.m_aiScaleStack[i] == 0)
			continue;

		return m_aiScaleStack[i] > u.m_aiScaleStack[i];
	}

	return (m_flRemainder > u.m_flRemainder);
}

static float g_aflConversions[11] = 
{
	0.000000000000001f,
	0.000000000001f,
	0.000000001f,
	0.000001f,
	0.001f,
	1.0f,
	1000.0f,
	1000000.0f,
	1000000000.0f,
	1000000000000.0f,
	1000000000000000.0f,
};

double CScalableFloat::ConvertUnits(double flUnits, scale_t eFrom, scale_t eTo)
{
	if (eFrom == eTo)
		return flUnits;

	if (flUnits == 0)
		return 0;

	TAssert( eTo >= SCALE_MILLIMETER && eTo <= SCALE_HIGHEST );
	if (eTo == SCALE_NONE)
		return 0;

	if (eTo < SCALE_NONE)
		return 0;

	if (eTo > SCALE_HIGHEST)
		return 0;

	TAssert( SCALE_HIGHEST+5 <= 11 );
	return flUnits * g_aflConversions[eFrom-eTo+5];
}

CScalableVector::CScalableVector(const CScalableFloat& X, const CScalableFloat& Y, const CScalableFloat& Z)
{
	x = X;
	y = Y;
	z = Z;
}

CScalableVector::CScalableVector(Vector vecUnits, scale_t eScale)
{
	x = CScalableFloat(vecUnits.x, eScale);
	y = CScalableFloat(vecUnits.y, eScale);
	z = CScalableFloat(vecUnits.z, eScale);
}

CScalableVector::CScalableVector(DoubleVector vecUnits, scale_t eScale)
{
	x = CScalableFloat(vecUnits.x, eScale);
	y = CScalableFloat(vecUnits.y, eScale);
	z = CScalableFloat(vecUnits.z, eScale);
}

DoubleVector CScalableVector::GetUnits(scale_t eScale) const
{
	DoubleVector vecResult = Vector(0,0,0);

	vecResult.x = x.GetUnits(eScale);
	vecResult.y = y.GetUnits(eScale);
	vecResult.z = z.GetUnits(eScale);

	return vecResult;
}

bool CScalableVector::IsZero() const
{
	return x.IsZero() && y.IsZero() && z.IsZero();
}

bool CScalableVector::IsZero()
{
	return x.IsZero() && y.IsZero() && z.IsZero();
}

CScalableFloat CScalableVector::Length() const
{
	double flX = x.GetUnits(SCALE_METER);
	double flY = y.GetUnits(SCALE_METER);
	double flZ = z.GetUnits(SCALE_METER);
	double f = flX*flX + flY*flY + flZ*flZ;
	CScalableFloat r(sqrt(f), SCALE_METER);

	return r;
}

CScalableFloat CScalableVector::LengthSqr() const
{
	return x*x + y*y + z*z;
}

CScalableFloat CScalableVector::Dot(const CScalableVector& v) const
{
	return x*v.x + y*v.y + z*v.z;
}

CScalableVector CScalableVector::operator-() const
{
	CScalableVector v;
	v.x = -x;
	v.y = -y;
	v.z = -z;
	return v;
}

CScalableVector CScalableVector::operator+( const CScalableVector& v ) const
{
	CScalableVector vecReturn;

	vecReturn.x = x + v.x;
	vecReturn.y = y + v.y;
	vecReturn.z = z + v.z;

	return vecReturn;
}

CScalableVector CScalableVector::operator-( const CScalableVector& v ) const
{
	CScalableVector vecReturn;

	vecReturn.x = x - v.x;
	vecReturn.y = y - v.y;
	vecReturn.z = z - v.z;

	return vecReturn;
}

CScalableVector CScalableVector::operator*( const CScalableFloat& f ) const
{
	CScalableVector vecReturn;
	for (size_t i = 0; i < 3; i++)
		vecReturn[i] = Index(i) * f;

	return vecReturn;
}

CScalableVector CScalableVector::operator/( const CScalableFloat& f ) const
{
	CScalableVector vecReturn;
	for (size_t i = 0; i < 3; i++)
		vecReturn[i] = Index(i) / f;

	return vecReturn;
}

CScalableVector CScalableVector::operator*( float f ) const
{
	CScalableVector vecReturn;
	for (size_t i = 0; i < 3; i++)
		vecReturn[i] = Index(i) * f;

	return vecReturn;
}

CScalableVector CScalableVector::operator/( float f ) const
{
	CScalableVector vecReturn;
	for (size_t i = 0; i < 3; i++)
		vecReturn[i] = Index(i) / f;

	return vecReturn;
}

CScalableFloat CScalableVector::Index(int i) const
{
	if (i == 0)
		return x;
	if (i == 1)
		return y;
	if (i == 2)
		return z;

	TAssert(false);
	return x;
}

CScalableFloat& CScalableVector::Index(int i)
{
	if (i == 0)
		return x;
	if (i == 1)
		return y;
	if (i == 2)
		return z;

	TAssert(false);
	return x;
}

CScalableFloat CScalableVector::operator[](int i) const
{
	return Index(i);
}

CScalableFloat& CScalableVector::operator[](int i)
{
	return Index(i);
}

bool CScalableVector::operator==(const CScalableVector& u) const
{
	return (x == u.x && y == u.y && z == u.z);
}

CScalableMatrix::CScalableMatrix(const Vector& vecForward, const Vector& vecUp, const Vector& vecRight, const CScalableVector& vecPosition)
{
	SetColumn(0, vecForward);
	SetColumn(1, vecUp);
	SetColumn(2, vecRight);
	SetTranslation(vecPosition);
}

void CScalableMatrix::Identity()
{
	memset(this, 0, sizeof(m));

	m[0][0] = m[1][1] = m[2][2] = 1;

	SetTranslation(CScalableVector());
}

void CScalableMatrix::SetAngles(const EAngle& angDir)
{
	float sp = sin(angDir.p * M_PI/180);
	float sy = sin(angDir.y * M_PI/180);
	float sr = sin(angDir.r * M_PI/180);
	float cp = cos(angDir.p * M_PI/180);
	float cy = cos(angDir.y * M_PI/180);
	float cr = cos(angDir.r * M_PI/180);

	m[0][0] = cy*cp;
	m[0][1] = sr*sy-sp*cr*cy;
	m[0][2] = sp*sr*cy+cr*sy;
	m[1][0] = sp;
	m[1][1] = cp*cr;
	m[1][2] = -cp*sr;
	m[2][0] = -sy*cp;
	m[2][1] = sp*cr*sy+sr*cy;
	m[2][2] = cr*cy-sy*sp*sr;
}

EAngle CScalableMatrix::GetAngles() const
{
	if (m[1][0] > 0.99f)
		return EAngle(90, atan2(m[0][2], m[2][2]) * 180/M_PI, 0);
	else if (m[1][0] < -0.99f)
		return EAngle(-90, atan2(m[0][2], m[2][2]) * 180/M_PI, 0);

	// Clamp to [-1, 1] looping
	float flPitch = fmod(m[1][0], 2);
	if (flPitch > 1)
		flPitch -= 2;
	else if (flPitch < -1)
		flPitch += 2;

	return EAngle(asin(flPitch) * 180/M_PI, atan2(-m[2][0], m[0][0]) * 180/M_PI, atan2(-m[1][2], m[1][1]) * 180/M_PI);
}

CScalableMatrix CScalableMatrix::operator*(const CScalableMatrix& t) const
{
	CScalableMatrix r;

	r.m[0][0] = m[0][0]*t.m[0][0] + m[0][1]*t.m[1][0] + m[0][2]*t.m[2][0];
	r.m[0][1] = m[0][0]*t.m[0][1] + m[0][1]*t.m[1][1] + m[0][2]*t.m[2][1];
	r.m[0][2] = m[0][0]*t.m[0][2] + m[0][1]*t.m[1][2] + m[0][2]*t.m[2][2];

	r.m[1][0] = m[1][0]*t.m[0][0] + m[1][1]*t.m[1][0] + m[1][2]*t.m[2][0];
	r.m[1][1] = m[1][0]*t.m[0][1] + m[1][1]*t.m[1][1] + m[1][2]*t.m[2][1];
	r.m[1][2] = m[1][0]*t.m[0][2] + m[1][1]*t.m[1][2] + m[1][2]*t.m[2][2];

	r.m[2][0] = m[2][0]*t.m[0][0] + m[2][1]*t.m[1][0] + m[2][2]*t.m[2][0];
	r.m[2][1] = m[2][0]*t.m[0][1] + m[2][1]*t.m[1][1] + m[2][2]*t.m[2][1];
	r.m[2][2] = m[2][0]*t.m[0][2] + m[2][1]*t.m[1][2] + m[2][2]*t.m[2][2];

	r.mt[0] = t.mt[0]*m[0][0] + t.mt[1]*m[0][1] + t.mt[2]*m[0][2] + mt[0];
	r.mt[1] = t.mt[0]*m[1][0] + t.mt[1]*m[1][1] + t.mt[2]*m[1][2] + mt[1];
	r.mt[2] = t.mt[0]*m[2][0] + t.mt[1]*m[2][1] + t.mt[2]*m[2][2] + mt[2];

	return r;
}

CScalableVector CScalableMatrix::operator*(const CScalableVector& v) const
{
	CScalableVector vecResult;
	vecResult.x = (v.x * m[0][0]).AddMultiple(v.y * m[0][1], v.z * m[0][2], mt[0]);
	vecResult.y = (v.x * m[1][0]).AddMultiple(v.y * m[1][1], v.z * m[1][2], mt[1]);
	vecResult.z = (v.x * m[2][0]).AddMultiple(v.y * m[2][1], v.z * m[2][2], mt[2]);

	return vecResult;
}

CScalableVector CScalableMatrix::TransformNoTranslate(const CScalableVector& v) const
{
	CScalableVector vecResult;
	vecResult.x = (v.x * m[0][0]).AddMultiple(v.y * m[0][1], v.z * m[0][2]);
	vecResult.y = (v.x * m[1][0]).AddMultiple(v.y * m[1][1], v.z * m[1][2]);
	vecResult.z = (v.x * m[2][0]).AddMultiple(v.y * m[2][1], v.z * m[2][2]);

	return vecResult;
}

// Not a true inversion, only works if the matrix is a translation/rotation matrix.
void CScalableMatrix::InvertTR()
{
	CScalableMatrix t;

	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			t.m[h][v] = m[v][h];

	CScalableFloat fl03 = mt[0];
	CScalableFloat fl13 = mt[1];
	CScalableFloat fl23 = mt[2];

	(*this) = t;

	SetTranslation(t*CScalableVector(-fl03, -fl13, -fl23));
}

void CScalableMatrix::SetColumn(int i, const Vector& vecColumn)
{
	m[0][i] = vecColumn.x;
	m[1][i] = vecColumn.y;
	m[2][i] = vecColumn.z;
}

Vector CScalableMatrix::GetColumn(int i) const
{
	return Vector(m[0][i], m[1][i], m[2][i]);
}

Matrix4x4 CScalableMatrix::GetUnits(scale_t eScale) const
{
	Matrix4x4 r;
	r.Init(m[0][0], m[0][1], m[0][2], 0, m[1][0], m[1][1], m[1][2], 0, m[2][0], m[2][1], m[2][2], 0, 0, 0, 0, 1);
	r.SetTranslation(mt.GetUnits(eScale));
	return r;
}

bool LineSegmentIntersectsSphere(const CScalableVector& v1, const CScalableVector& v2, const CScalableVector& s, const CScalableFloat& flRadius, CScalableVector& vecPoint)
{
	CScalableVector vecLine = v2 - v1;
	CScalableVector vecSphere = v1 - s;

	CScalableFloat flA = vecLine.LengthSqr();
	CScalableFloat flB = vecSphere.Dot(vecLine) * 2;
	CScalableFloat flC1 = s.LengthSqr() + v1.LengthSqr();
	CScalableFloat flC2 = (s.Dot(v1)*2);
	CScalableFloat flC = flC1 - flC2 - flRadius*flRadius;

	CScalableFloat flBB4AC = flB*flB - flA*flC*4;
	if (flBB4AC < CScalableFloat())
		return false;

	CScalableFloat flSqrt(sqrt(flBB4AC.GetUnits(SCALE_METER)), SCALE_METER);
	CScalableFloat flPlus = (-flB + flSqrt)/(flA*2);
	CScalableFloat flMinus = (-flB - flSqrt)/(flA*2);

	CScalableFloat fl1(1.0f, SCALE_METER);
	bool bPlusBelow0 = flPlus < CScalableFloat();
	bool bMinusBelow0 = flMinus < CScalableFloat();
	bool bPlusAbove1 = flPlus > fl1;
	bool bMinusAbove1 = flMinus > fl1;

	// If both are above 1 or below 0, then we're not touching the sphere.
	if (bMinusBelow0 && bPlusBelow0 || bPlusAbove1 && bMinusAbove1)
		return false;

	if (bMinusBelow0 && bPlusAbove1)
	{
		// We are inside the sphere.
		vecPoint = v1;
		return true;
	}

	if (bMinusAbove1 && bPlusBelow0)
	{
		// We are inside the sphere. Is this even possible? I dunno. I'm putting an assert here to see.
		// If it's still here later that means no.
		TAssert(false);
		vecPoint = v1;
		return true;
	}

	// If flPlus is below 1 and flMinus is below 0 that means we started our trace inside the sphere and we're heading out.
	// Don't intersect with the sphere in this case so that things on the inside can get out without getting stuck.
	if (bMinusBelow0 && !bPlusAbove1)
		return false;

	// In any other case, we intersect with the sphere and we use the flMinus value as the intersection point.
	CScalableFloat flDistance = vecLine.Length();
	CScalableVector vecDirection = vecLine / flDistance;

	vecPoint = v1 + vecDirection * (flMinus * flDistance);
	return true;
}
