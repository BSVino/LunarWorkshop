#ifndef DT_COMMON
#define DT_COMMON

typedef enum
{
	MODE_NONE = 0,
	MODE_MOVE,
	MODE_TURN,
	MODE_AIM,
	MODE_BUILD,
} controlmode_t;

typedef enum
{
	MENUMODE_MAIN,
	MENUMODE_PROMOTE,
	MENUMODE_LOADERS,
	MENUMODE_INSTALL,
	MENUMODE_WEAPON,
} menumode_t;

typedef enum
{
	UPDATECLASS_EMPTY = 0,
	UPDATECLASS_STRUCTURE,
	UPDATECLASS_STRUCTUREUPDATE,
} updateclass_t;

typedef enum
{
	UPDATETYPE_NONE = 0,
	UPDATETYPE_PRODUCTION,
	UPDATETYPE_BANDWIDTH,
	UPDATETYPE_FLEETSUPPLY,
	UPDATETYPE_SUPPORTENERGY,
	UPDATETYPE_SUPPORTRECHARGE,
	UPDATETYPE_TANKATTACK,
	UPDATETYPE_TANKDEFENSE,
	UPDATETYPE_TANKMOVEMENT,
	UPDATETYPE_TANKHEALTH,
	UPDATETYPE_TANKRANGE,
	UPDATETYPE_SIZE,
} updatetype_t;

typedef enum
{
	PROJECTILE_SMALL,
	PROJECTILE_MEDIUM,
	PROJECTILE_LARGE,
	PROJECTILE_AOE,

	// Strategy mode projectiles
	PROJECTILE_FLAK,		// For infantry
	PROJECTILE_TORPEDO,		// For scouts
	PROJECTILE_ARTILLERY,	// For guess who

	PROJECTILE_FIREWORKS,

	// If you add them here, add their energy prices too in projectile.cpp
	PROJECTILE_MAX,
} projectile_t;

#endif
