#pragma once
#include "CDOTABaseAbility.h"

// Current stat of Power Treads/Vambrace
enum class ItemStat {
	STRENGTH = 0,
	INTELLIGENCE,
	AGILITY
};

class CDOTAItem : public CDOTABaseAbility {
public:
	GETTER(int, GetCurrentCharges, 0x638);
	// For PT and Vambrace
	GETTER(ItemStat, GetItemStat, 0x6a0);
};