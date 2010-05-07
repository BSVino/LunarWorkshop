#include "powerup.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <color.h>

#include "digitank.h"

void CPowerup::Render()
{
	Color clrPowerup(255, 255, 255);
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (pEntity == NULL)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (!pTank)
			continue;

		Vector vecOrigin = pEntity->GetOrigin();
		if (pTank->HasDesiredMove())
			vecOrigin = pTank->GetDesiredMove();

		if ((vecOrigin - GetOrigin()).LengthSqr() < 3*3)
			clrPowerup = Color(0, 255, 0);
	}

	glPushMatrix();

	glTranslatef(GetOrigin().x, GetOrigin().y, GetOrigin().z);

	int iRotate = glutGet(GLUT_ELAPSED_TIME)%3600;
	glRotatef(iRotate/10.0f, 0, 1, 0);

	glColor4ubv(clrPowerup);
	glutWireSphere(2, 4, 2);

	glPopMatrix();
}

