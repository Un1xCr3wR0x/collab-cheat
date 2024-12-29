#pragma once
#include "../Base/CUtlVector.h"
#include "CHandle.h"
#include "CBaseEntity.h"

class CDOTABaseNPC;

struct CDOTA_BuffParticle {
	uint32_t particleHandle, unk = 10000, entIdx;
};

// schema class server.dll/CDOTA_Buff
class CDOTAModifier : public VClass {
public:
	GETTER(const char*, GetName, 0x28);
	GETTER(float, GetDuration, 0x60);
	GETTER(float, GetDieTime, 0x64);
	GETTER(int, GetStackCount, 0x78);
	GETTER(DOTA_GC_TEAM, GetTeam, 0x88);
	GETTER(CHandle<CDOTABaseNPC>, GetCaster, 0x68);
	GETTER(CHandle<CDOTABaseAbility>, GetAbility, 0x6c);
	GETTER(CHandle<CDOTABaseNPC>, GetOwner, 0x70);
	FIELD(CUtlVector<CDOTA_BuffParticle>, Particles, 0xa0);

	bool IsSameTeam(CBaseEntity* ent) const {
		return ent->GetTeam() == GetTeam();
	}

	VGETTER(const char*, GetTexture, 3);
};

