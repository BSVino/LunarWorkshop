#include "momento.h"

REGISTER_ENTITY(CMomento);

NETVAR_TABLE_BEGIN(CMomento);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMomento);
	SAVEDATA_DEFINE_HANDLE(CSaveData::DATA_STRING, tstring, m_sMomentoName, "MomentoName");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CMomento);
INPUTS_TABLE_END();

void CMomento::Precache()
{
}

void CMomento::Spawn()
{
}
