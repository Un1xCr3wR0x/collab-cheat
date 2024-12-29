#pragma once
#include "CDOTABaseNPC.h"

class CDOTABaseNPC_Hero : public CDOTABaseNPC {
public:
	struct HeroAttributes {
		float strength, agility, intellect;
	};

	GETTER(HeroAttributes, GetAttributes, 0x1880);

	bool IsIllusion() const {
		return Member<CHandle<CDOTABaseNPC_Hero>>(0x18f0).IsValid();
	}
};
