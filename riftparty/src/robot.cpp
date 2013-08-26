#include "robot.h"

#include <physics/physics.h>
#include <renderer/game_renderer.h>
#include <renderer/renderingcontext.h>
#include <ui/gamewindow.h>
#include <game/entities/beam.h>

#include "riftparty_game.h"
#include "riftparty_character.h"

REGISTER_ENTITY(CRobot);

NETVAR_TABLE_BEGIN(CRobot);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CRobot);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CRobot);
INPUTS_TABLE_END();

CRobot::~CRobot()
{
}

void CRobot::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/characters/robot.toy");
}

void CRobot::Spawn()
{
	Precache();

	SetMass(600);

	SetModel("models/characters/robot.toy");

	BaseClass::Spawn();
}

void CRobot::Think()
{
	BaseClass::Think();
}
